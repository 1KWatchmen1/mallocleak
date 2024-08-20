#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>//unlink
#if 1
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
#endif
int main()
{
    void *p1 = malloc(5);
    void *p2 = malloc(10);
    void *p3 = malloc(15);
    
    free(p1);
    free(p3);
}

