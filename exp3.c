#include<stdio.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#define BUFFER_SIZE 1000
#define BUFFER_NUM 5
int semid;
int readbuf,writebuf;
int sharebuf[BUFFER_NUM];//共享缓冲区标识符数组
int *var[BUFFER_NUM];
int key=75;
typedef struct shareBuffer
{
	char buffer[BUFFER_SIZE];
	int size;
}shareBuffer;

void P(int semid,int index)
{	struct sembuf sem;	
    sem.sem_num = index;
    sem.sem_op = -1;	
	sem.sem_flg = 0; //操作标记：0或IPC_NOWAIT等
	semop(semid,&sem,1);	//1:表示执行命令的个数	
	return;
}

void V(int semid,int index)
{	
	struct sembuf sem;	
	sem.sem_num = index;
	sem.sem_op =  1;
	sem.sem_flg = 0;	
	semop(semid,&sem,1);	
	return;
}

int main()
{
	int i;
	for(i=0;i<BUFFER_NUM;i++)
	{
		sharebuf[i] = shmget(key,sizeof(shareBuffer),0666|IPC_CREAT); /*创建共享存储区*/
		key++;
	}
	semid = semget(1000,2,IPC_CREAT|0666);//semid是信号灯集合的id
	semctl(semid,0,SETVAL,BUFFER_NUM);//有几个可以写
	semctl(semid,1,SETVAL,0);//有几个可以读
	if((readbuf=fork()))//readbuf>0表示父进程
	{
		if((writebuf=fork())){//writebuf>0表示父进程
			wait(0);
			wait(0);
			semctl(semid,0,IPC_RMID);
			semctl(semid,1,IPC_RMID);
			for(i=0;i<BUFFER_NUM;i++)
				shmctl(sharebuf[i],IPC_RMID,0);
		}
		else{//readbuf
			printf("readbuf Create OK ! \n");
			execv("./readbuf",NULL);
			exit(0);
		}
	}
	else{//writebuf
		printf("writebuf Create OK ! \n");
		execv("./writebuf",NULL);
		exit(0);
	}
	return 0;
}
