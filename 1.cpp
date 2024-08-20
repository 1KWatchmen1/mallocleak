#include <stdio.h>
#include <stdlib.h>
#if 1
//file
void *_malloc(size_t size, const char *filename, int line)
{
    printf("_malloc: %s, %d\n", filename, line);
}
void _free(void *ptr, const char *filename, int line)
{
    printf("free: %s, %d\n",filename,line);
}

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