#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#define BUFFER_SIZE 1024
char buf[BUFFER_SIZE];
int main()
{
	int fd;
	int count=0;
	fd = open("/dev/keshe3",O_RDWR);//打开设备
	if(fd!=-1)
	{
		printf("请输入字符串:\n");
		scanf("%s",buf);
		while(buf[count++]) ;
		if(write(fd,buf,count))
		{
			if(read(fd,buf,count))
				printf("输出字符串:\n%s\n",buf);
			else
				printf("读设备失败\n");
		}
		else 
			printf("写设备失败\n");
	}
	else
		printf("打开设备失败\n");
		return -1;
	close(fd);
	return 0;
}
