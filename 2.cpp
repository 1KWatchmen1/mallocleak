#include <stdio.h>
#include <stdlib.h>
#if 1
//file
void *_malloc(size_t size, const char *filename, int line)
{
    void *p = malloc(size);
    printf("[+]%p, %s, %d\n", p, filename, line);
    return p;
}
void _free(void *ptr, const char *filename, int line)
{
    free(ptr);
    printf("[-]%p, %s, %d\n", ptr, filename, line);
}
/*这是一个预处理宏，用来将代码中的 malloc(size) 替换为 _malloc(size, __FILE__, __LINE__)。
__FILE__ 是一个预定义宏，表示当前文件的名称。
__LINE__ 是一个预定义宏，表示当前行号。
在代码编译时，__FILE__ 会被替换为一个字符串，表示当前正在编译的文件的文件名。*/
#define malloc(size) _malloc(size, __FILE__,__LINE__)
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