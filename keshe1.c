#include<stdio.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#define BUFFER_SIZE 1024

int main(int argc,char **argv)
{
	if(argc!=3)
	{
		fprintf(stderr,"file numbers error!\n");
		exit(1);
	}
	int rd,wt;//分别记录打开读写文件的返回值
	int buf_rd,buf_wt;////分别记录读写缓冲区的大小(以字节为单位)
	char buf[BUFFER_SIZE],*p;//缓冲区
	if((rd = open(argv[1],O_RDONLY)) == -1)//以只读方式打开失败
	{
		fprintf(stderr,"open %s failed!\n",argv[1]);
		exit(-1);
	}
	if((wt = open(argv[2], O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) == -1)//以只写和创建方式(权限为只有拥有者user可以读写)打开失败
	{
		fprintf(stderr,"open %s failed!\n",argv[2]);
		exit(-1);
	}
	while(buf_rd = read(rd,buf,BUFFER_SIZE))//读到数据为0，那么就表示文件结束了
	{
		if(buf_rd == -1 && errno != EINTR)//buf_rd == -1 && errno == EINTR表示读的过程中遇到了中断，否则出错
		{
			fprintf(stderr,"read error!\n");
			break;
		}
		else if(buf_rd > 0)
		{
			/*
			bytes_write=write(fd2,p,bytes_read) 调用这个函数时，虽然要求写入bytes_read个字节
            但实际的情况可能一次性不能写入这么多字节，因此通过返回值bytes_write就可以知道究竟写入多少字节
            当bytes_write<0直接退出，写入失败，或bytes_write=bytes_read 写入成功，也退出。
            (bytes_write>0)&&(bytes_write!=bytes_read)一次性没写完，继续把剩余的写完。
			*/
			p = buf;
			while(buf_wt = write(wt,p,buf_rd))
			{
				if(buf_wt == -1 && errno != EINTR)
					break;
				else if(buf_wt == buf_rd)//写完了
					break;
				else if(buf_wt > 0)//没写完
				{
					p += buf_wt;
					buf_rd -= buf_wt;
				}
			}
			if(buf_wt == -1)//while循环里由于出错break出来的
			{
				fprintf(stderr,"write error!\n");
				break;
			}
		}
	}
	close(wt);
	close(rd);
	return 0;
}
