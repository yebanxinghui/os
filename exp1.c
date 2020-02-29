#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>	
#include<unistd.h>
int pipefd[2];//无名管道,0负责读，1负责写
pid_t p[2];//2个子进程pid号
void my_func(int sig_no) {
    if(sig_no == SIGUSR1){
		close(pipefd[1]);//子进程1负责写
		printf("Child Process 1 is Killed by Parent!\n");
	}
    if(sig_no == SIGUSR2){
		close(pipefd[0]);//子进程2负责读
		printf("Child Process 2 is Killed by Parent!\n");
	}
	exit(0);
}
void kill12()
{
	kill(p[0],SIGUSR1);//发信号，不是结束进程
	kill(p[1],SIGUSR2);
}
int main(void)
{
	int num=1;
	if(pipe(pipefd)!=0)//pipe函数返回0位成功，-1失败
	{
		printf("pipe error!\n");
		exit(-1);
	}
	
	if((p[0]=fork()))//p[0]>0表示父进程
	{
		if((p[1]=fork())){//p[1]>0表示父进程
			signal(SIGALRM,kill12);
			alarm(10);
			signal(SIGINT,kill12);
			wait();
			wait();
		}
		else{//p[1]=0表示子进程2
			signal(SIGINT,SIG_IGN);//SIG_IGN表示忽略该信号/SIG_DFL表示默认
			signal(SIGUSR2,my_func);
			
			while(1)
			{
				close(pipefd[1]);//禁止写
				char rec[100];
				memset(rec,'\0',sizeof(rec));
				read(pipefd[0],rec,sizeof(rec));
				printf("%s",rec);
			}
		}
	}
	else{//p[0]=0表示子进程1
		signal(SIGINT,SIG_IGN);
		signal(SIGUSR1,my_func);
		while(1)
		{
			close(pipefd[0]);//禁止读
			char send[100];
			memset(send,'\0',sizeof(send));
			sprintf(send,"I send you %d times \n",num);
			write(pipefd[1],send,sizeof(send));
			num++;
			sleep(1);
		}
	}
	close(pipefd[0]);
	close(pipefd[1]);
	printf("Parent Process is Killed!\n");
	return 0;
}
