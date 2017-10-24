Reference:
http://blog.csdn.net/fz835304205/article/details/16963755
hexedit /dev/mem
ctrl+w 保存  ctrl+c 退出
----
既然内存的地址以及内容信息全部被保存在mem这个设备文件里，那么我们可以想到通过另外一种方式来实现对物理地址的读写了。
那就是将mem设备文件和 mmap系统调用结合起来使用，将文件里的物理内存地址映射到进程的地址空间，从而实现对内存物理地址的读写。下面谈一下mmap系统调用。

mmap的函数原型为：void *mmap(void *start,size_t length,int prot,int flags,int fd,off_t offset)，该函数定义在/usr/include/sys/mman.h中，
使用时要包含：#include<sys /mman.h>，mmap()用来将某个文件中的内容映射到进程的地址空间，对该空间的存取即是对该文件内容的读写。参数说明如下：

start:指向欲映射到的地址空间的起始地址，通常设为null或者0.表示让系统融自动选定地址，映射成功后该地址会返回。

length:表示映射的文件内容的大小，以字节为单位。

prot:表示映射区域的保护方式，有如下四种组合：
--PROT_EXEC 映射区域可执行 ,
--PROT_READ 映射区域可读 ,
--PROT_WRITE 映射区域可写,
--PROT_NONE 映射区域不能被访问

flags:映射区域的一些特性，主要有：
--MAP_FIXED 如果映射不成功则出错返回,
--MAP_SHARED 对映射区域的写入数据会写回到原来的文件
--MAP_PRIVATE 对映射区域的写入数据不会写回原来的文件
--MAP_ANONYMOUS
--MAP_DENYWRITE 只允许对映射区域的写入操作，其他对文件直接写入的操作将被拒绝
--MAP_LOCKED 锁定映射区域
在调用mmap()时，必须要指定MAP_SHARED或MAP_PRIVATE。

fd:open()返回的文件描述符。

offset:为被映射文件的偏移量，表示从文件的哪个地方开始映射，一般设置为0，表示从文件的最开始位置开始映射。offset必须是分页大小（4096字节）的整数倍。
----
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>  //close
#include <string.h>  //memcpy,
#include <sys/mman.h>//mmap head file
int main (void)
{
   int i;
   int fd;
   char *map_base;
   char *buf = "helloworld!";

   //open /dev/mem with read and write mode
   fd = open ("/dev/mem", O_RDWR);
   if (fd < 0)
   {
          printf("cannot open /dev/mem.");
          return -1;
   }

   //map physical memory 0-10 bytes
   map_base = (char *)mmap(0, 10, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if(map_base < 0)
   {
      printf("mmap failed.");
      return -1;
   }
   //Read old value
   for (i = 0; i < 10; i++)
   {
          printf("old mem[%d]:%c\n", i, *(map_base + i));
   }
   //write memory
   memcpy(map_base, buf, 10);
   //Read new value
   for (i = 0;i < 10;i++)
   {
      printf("new mem[%d]:%c\n", i,*(map_base + i));
   }
   munmap(map_base, 10); //destroy map memory
   close(fd);   //close file
   return 0;
}
