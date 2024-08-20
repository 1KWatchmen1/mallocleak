
/*1、2、3三个是一个流程，3是最终版本；这种方式仅限于一个单文件，对于多个文件，此种方式不可用；*/
dlsym方式：4、5,5是最终版本，但是addr2line还没搞明白
//在init_hook()函数中截获malloc，
//在malloc和free函数中写明自己的实现
//这里是通过 fprintf(fp, "[+]%p addr:%p, size: %ld\n", caller, p, size);这里的caller输出一个地址，然后用addr2line命令来获取文件和行号
//要借助工具addr2line -f -e 5 -a 0x55a5377a9b83//测试不行

//第二种方式比第一种方式（宏定义）的优越：适合整个工程（应该是因为在mian函数中初始化）



//bpf 我没测试，我都没安装，这里简介以下使用：

1、bpf可以判断有无内存泄漏，但是不知道怎么判断在哪一行
2、使用工具bpftrace
编写一个mem.bt文件，通过uprobe使用这个地方的库（malloc函数在libc库中）来挂载到malloc和free上
//这个 uprobe 将在调用 libc 库中的 malloc 函数时触发。
uprobe:/lib/x86_64-linux-gnu/libc.so.6:malloc
//要监控名为 "6" 的可执行程序，你需要在 mem.bt 文件中使用 comm == "6" 作为过滤条件。
/comm == "6"/
{
        printf("malloc");
}
uprobe:/lib/x86_64-linux-gnu/libc.so.6:free
/comm == "6"/
{
         printf("free");
}

通过执行可执行程序6，然后将mem.bt也启动起来，这样的话，可执行程序在调用malloc和free的时候，执行men.bt程序的窗口就会显示出调用malloc和free情况；
这种情况是在不影响代码的情况下，进行的；这是bpf的优势