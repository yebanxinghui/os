#include<stdio.h>
#include<pthread.h>
#include<linux/sem.h>
#include<sys/types.h>
int semid;
pthread_t p1,p2;
int a=0;
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
void *subp1()
{	
	int i=1; 
	for(;i<=100;i++){
		P(semid,1);
		printf("%d\n",a);
		V(semid,0);
	}
	pthread_exit(NULL);
}
void *subp2()
{
	int i=1;
	for(;i<=100;i++){
		P(semid,0);
		a+=i;
		V(semid,1);
	}
	pthread_exit(NULL);
}
int main()
{
	semid = semget(1000,2,IPC_CREAT|0666);//semid是信号灯集合的id
	semctl(semid,1,SETVAL,0);
	semctl(semid,0,SETVAL,1);
	pthread_create(&p1,NULL,(void *)subp1,NULL);
	pthread_create(&p2,NULL,(void *)subp2,NULL);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	semctl(semid,0,IPC_RMID);
	return 0;
}
