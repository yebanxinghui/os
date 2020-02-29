#include<stdio.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include<errno.h>
#include<fcntl.h>
#define BUFFER_SIZE 1000
#define BUFFER_NUM 5
int semid;
int sharebuf[BUFFER_NUM];//共享缓冲区标识符数组
int key=75;

typedef struct shareBuffer
{
	char buffer[BUFFER_SIZE];
	int size;
}shareBuffer;
shareBuffer *addr[BUFFER_NUM];
void P(int semid,int index)
{	struct sembuf sem;	
    sem.sem_num = index;
    sem.sem_op = -1;	
	sem.sem_flg = 0; //操作标记：0或IPC_NOWAIT等
	semop(semid,&sem,1);//1:表示执行命令的个数	
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
	int i=0;
	int count=0;
	for(i=0;i<BUFFER_NUM;i++)
	{
		sharebuf[i] = shmget(key,sizeof(struct shareBuffer),0666|IPC_CREAT); /*创建共享存储区*/
		key++;
		addr[i]=(struct shareBuffer *)shmat(sharebuf[i],NULL,0);        /*获取首地址*/
	}
	semid = semget(1000,2,0666|IPC_CREAT);//semid是信号灯集合的id
	char *filepath="flower2.png";
	FILE *fp = fopen(filepath,"wb");
	//printf("read ok!");
	i=0;
	while(addr[i]->size == BUFFER_SIZE)
	{
		P(semid,1);
		fwrite(addr[i]->buffer,1,BUFFER_SIZE,fp);
			
		i = (i + 1)%BUFFER_NUM;
		V(semid,0);		
		//printf("read2 ok!");
	}
	if(addr[i]->size > 0)
        fwrite(addr[i]->buffer,1,addr[i]->size,fp);
	fclose(fp);
    exit(0);
}
/*

	while(1)
	{
		P(semid,1);
		fwrite(addr[i]->buffer,1,addr[i]->size,fp);
		V(semid,0);
		count+=addr[i]->size;
		printf("%d\n",count);
        if(addr[i]->size != BUFFER_SIZE)
            break;
		i = (i + 1)%BUFFER_NUM;
	}
*/