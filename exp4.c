#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
char  get_type(mode_t mod)
{
	switch(mod & S_IFMT)
    {
		case S_IFSOCK: return 's';//socket 套接字
		case S_IFREG: return '-';//普通文件
		case S_IFCHR: return 'c';//字符设备
		case S_IFBLK: return 'b';//块设备
		case S_IFLNK: return 'l';//符号链接
		case S_IFIFO: return 'p';//管道文件
		case S_IFDIR: return 'd';//目录文件
    }
}
//获取文件权限
void get_perm(mode_t mod,char *buf)
{
    bzero(buf,sizeof(buf));
    int i = 9;
    while(i--)
    {
        if(mod & 1<<i)
        {
            switch((8-i)%3)
            {
            case 0: buf[8-i] = 'r'; break;
            case 1: buf[8-i] = 'w'; break;
            case 2: buf[8-i] = 'x'; break;
            }
        }
        else
            buf[8-i] = '-';
    }
}
//获取文件最后一次修改时间
void get_ltime(time_t *t,char *buf)
{
	struct tm *tmp;
	tmp=localtime(t);
	sprintf(buf,"%d-%d-%d %d:%d:%d", (1900 + tmp->tm_year), ( 1 + tmp->tm_mon), tmp->tm_mday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec); 
	return ;
}
void printdir(char *dir, int depth){
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir))==NULL){  //opendir   
        fprintf(stderr,"该目录打不开\n");
        return;
    }
    chdir(dir); //chdir
	while(entry=readdir(dp)){
        lstat(entry->d_name,&statbuf);
		char buf_perm[10];
		get_perm(statbuf.st_mode,buf_perm);
		char buf_time[100];
		get_ltime(&statbuf.st_ctime,buf_time);
        if(S_ISDIR(statbuf.st_mode)){
            if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)
               	continue;
			//printf("%*s%s/\n",depth," ",entry->d_name);
			if(depth==0)
				printf("%c%9s %ld %s %s %ld %s %s %d/\n",
				get_type(statbuf.st_mode),buf_perm,statbuf.st_nlink,getpwuid(statbuf.st_uid)->pw_name,getgrgid(statbuf.st_gid)->gr_name,statbuf.st_size,buf_time,entry->d_name,depth/4);
			else 
				printf("%*s%c%9s %ld %s %s %ld %s %s %d/\n",
				depth," ",get_type(statbuf.st_mode),buf_perm,statbuf.st_nlink,getpwuid(statbuf.st_uid)->pw_name,getgrgid(statbuf.st_gid)->gr_name,statbuf.st_size,buf_time,entry->d_name,depth/4);
            //打印目录项的深度、目录名等信息//printf(%*depth)
			printdir(entry->d_name,depth+4);
            //递归调用printdir,打印子目录的信息,其中的depth+4;
        }
		else{
			if(depth==0)
				printf("%c%9s %ld %s %s %ld %s %s %d/\n",
				get_type(statbuf.st_mode),buf_perm,statbuf.st_nlink,getpwuid(statbuf.st_uid)->pw_name,getgrgid(statbuf.st_gid)->gr_name,statbuf.st_size,buf_time,entry->d_name,depth/4);
			else 
				printf("%*s%c%9s %ld %s %s %ld %s %s %d\n",
			depth," ",get_type(statbuf.st_mode),buf_perm,statbuf.st_nlink,getpwuid(statbuf.st_uid)->pw_name,getgrgid(statbuf.st_gid)->gr_name,statbuf.st_size,buf_time,entry->d_name,depth/4);
		}
	}
    chdir(".."); //chdir
    closedir(dp); //closedir
}

int main()
{
	char s[100];
	printf("请输入一个目录:如：/mnt\n");
	scanf("%s",s);
	printdir(s,0);
	return 0;
}
