#define _GNU_SOURCE //这两行都是通过man dlysm查到
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//unlink
//本文件编译要连接库：gcc -o 4 4.cpp -ldl
#if 0
//file
void *_malloc(size_t size, const char *filename, int line)
{
    void *p = malloc(size);

    char buff[128] = {0};
    sprintf(buff, "./checkleak/%p.mem", p);//filename

    FILE *fp = fopen(buff, "w");//FILE不是系统调用，是lib C的库
    fprintf(fp, "[+]%s:%d addr:%p, size: %ld\n",filename, line, p, size);
    // fflush(fp);//刷新
    fclose(fp);//关闭的时候自带刷新，先写到缓存，再刷新到磁盘

    return p;
}
void _free(void *ptr, const char *filename, int line)
{
    char buff[128] = {0};
    sprintf(buff, "./checkleak/%p.mem", ptr);//filename
    //目前不严谨，单线程可以
    //如果存在，
    if(unlink(buff) < 0) {//if file no exist,出现double free
        printf("double free: %p\n",ptr);
        return;

    } 
    free(ptr);
    // printf("[-]%p, %s, %d\n", ptr, filename, line);
}
/*这是一个预处理宏，用来将代码中的 malloc(size) 替换为 _malloc(size, __FILE__, __LINE__)。
__FILE__ 是一个预定义宏，表示当前文件的名称。
__LINE__ 是一个预定义宏，表示当前行号。
在代码编译时，__FILE__ 会被替换为一个字符串，表示当前正在编译的文件的文件名。*/
#define malloc(size) _malloc(size, __FILE__,__LINE__)//注意宏定义要放在函数定义之后；
#define free(ptr) _free(ptr, __FILE__, __LINE__)
//malloc这个函数的源码在：kjk1@kkk-virtual-machine:~/WorkSpace/leak$ ls /lib/x86_64-linux-gnu/libc.so.6  -l
// lrwxrwxrwx 1 root root 12 5月   3  2022 /lib/x86_64-linux-gnu/libc.so.6 -> libc-2.27.so
// 可以看到，是使用了一个软连接，连接到我们的版本库，这样的话后期版本库再更新也没事，如果直接打开版本库的话，随着版本升级的话，就要改代码
#elif 0
//disym dlopen
//第一种方式,
//1、声明malloc一样的函数和变量
typedef void *(*malloc_t)(size_t size);
typedef void (*free_t)(void *ptr);
malloc_t malloc_f = NULL;//定义两个函数指针
free_t free_f = NULL;

int enable_malloc_hook = 1;  //解决段错误
int enable_free_hook = 1; 

void *malloc(size_t size)
{
    //解决段错误
    void *p = NULL;
    if(enable_malloc_hook)
    {
        enable_malloc_hook = 0;
        p = malloc_f(size);
        // printf("malloc:\n");//加入此行代码后出现段错误，如何发现哪里出现的段错误：倒版本，倒完版本之后使用gdb调试；
        void * caller = __builtin_return_address(0);
        char buff[128] = {0};
        sprintf(buff, "./checkleak/%p.mem", p);//filename

        FILE *fp = fopen(buff, "w");//FILE不是系统调用，是lib C的库
        fprintf(fp, "[+]%p addr:%p, size: %ld\n", caller, p, size);//这里行号不能用__LINE__;使用__builtin_return_address(0)//到上n+1级调用的位置,此时输出的是机器代码，要借助工具addr2line -f -e 5 -a 0x55a5377a9b83
        fflush(fp);

        enable_malloc_hook = 1;
    }
    else
    {
        p = malloc_f(size);
    }
    return p; 
}
void free(void *ptr)
{
    if(enable_free_hook)
    {
        enable_free_hook = 0;

        char buff[128] = {0};
        sprintf(buff, "./checkleak/%p.mem", ptr);//filename
        //目前不严谨，单线程可以
        //如果存在，
        if(unlink(buff) < 0) {//if file no exist,出现double free
        printf("double free: %p\n",ptr);
        return;
        } 
        free_f(ptr);

        enable_free_hook = 1;
    }
    else{
        free_f(ptr);
    }
    // printf("free:\n");
}

///2、实现hook的初始化函数
void init_hook(void)
{
    if(malloc_f == NULL)
    {
        //通过man dlsym看这个函数的使用方法，返回的是一个void*
        malloc_f = (malloc_t)dlsym(RTLD_NEXT, "malloc");//当我们用到malloc时，就被malloc_f给截获；
    }
    if(free_f == NULL)
    {
        free_f = (free_t)dlsym(RTLD_NEXT, "free");
    }
}
/*调试过程：段错误
gcc -o 4 4.cpp -ldl -g  //编译时加上调试信息
gdb ./4 //gdb调试
b 57    //57加入断点
r   //通过gdb运行程序//可以看到Breakpoint 1, malloc (size=5) at 4.cpp:57 此时size = 5，说明第一条是执行过去的
c //继续：Breakpoint 1, malloc (size=1024) at 4.cpp:57；此时1024肯定不对；也就是在printf函数内部调用了malloc函数，出现了无线递归的现象，导致栈溢出的问题，那就要给一个递归结束的条件*/

//这里是通过 fprintf(fp, "[+]%p addr:%p, size: %ld\n", caller, p, size);这里的caller输出一个地址，然后用addr2line命令来获取文件和行号
//要借助工具addr2line -f -e 5 -a 0x55a5377a9b83//测试不行
//这种方式比宏定义的
#else
///代码量很多、第三方库也很多；公司自研代码也多
// bpf可以知道有没有内存泄露，但是定位到哪一行有点困难
#endif
int main()
{
    // init_hook();
    void *p1 = malloc(5);
    void *p2 = malloc(10);
    void *p3 = malloc(15);
    
    free(p1);
    free(p3);
}

