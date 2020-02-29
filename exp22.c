#include<stdio.h>
#include<pthread.h>
#include<linux/sem.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<time.h>
int semid;
pthread_t p1,p2,p3,p4;
int count=100;
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
void *sell1()
{	
	int s1=0;
	while(count>0){
		P(semid,0);
		if(count){
			s1++;
			printf("线程1卖了%d张票!\n",s1);
		}else{
			V(semid,0);
			break;
		}
		count--;
		srand((unsigned)time(NULL));
		int sleep_time = rand()%10000+1;
		usleep(sleep_time*10);
		V(semid,0);
	}
	pthread_exit(NULL);
}
void *sell2()
{	
	int s2=0;
	while(count>0){
		P(semid,0);
		if(count){
			s2++;
			printf("线程2卖了%d张票!\n",s2);
		}else{
			V(semid,0);
			break;
		}
		count--;
		srand((unsigned)time(NULL));
		int sleep_time = rand()%10000+1;
		usleep(sleep_time*10);
		V(semid,0);
	}
	pthread_exit(NULL);
}
void *sell3()
{	
	int s3=0;
	while(count>0){
		P(semid,0);
		if(count){
			s3++;
			printf("线程3卖了%d张票!\n",s3);
		}else{
			V(semid,0);
			break;
		}
		count--;
		srand((unsigned)time(NULL));
		int sleep_time = rand()%10000+1;
		usleep(sleep_time*10);
		V(semid,0);
	}
	pthread_exit(NULL);
}
void *sell4()
{	
	int s4=0;
	while(count>0){
		P(semid,0);
		if(count){
			s4++;
			printf("线程4卖了%d张票!\n",s4);
		}else{
			V(semid,0);
			break;
		}
		count--;
		srand((unsigned)time(NULL));
		int sleep_time = rand()%100000+1;
		usleep(sleep_time*10);
		V(semid,0);
	}
	pthread_exit(NULL);
}
void sell(void *arg)
{
	int i = *(int *)arg;
	int s=0;
	while(count>0){
		P(semid,0);
		if(count){
			s++;
			printf("线程%d卖了%d张票!\n",i,s);
		}
		else{
			V(semid,0);
			break;
		}
		count--;
		srand((unsigned)time(NULL));
		int sleep_time = rand()%100000+1;
		usleep(sleep_time*5);
		V(semid,0);
	}
	pthread_exit(NULL);
}
int main()
{
	semid = semget(1000,1,IPC_CREAT|0666);//semid是信号灯集合的id
	semctl(semid,0,SETVAL,1);
	int a[4]={1,2,3,4};
	pthread_create(&p1,NULL,(void *)sell,&a[0]);
	pthread_create(&p2,NULL,(void *)sell,&a[1]);
	pthread_create(&p3,NULL,(void *)sell,&a[2]);
	pthread_create(&p4,NULL,(void *)sell,&a[3]);
	pthread_join(p1,NULL);
	pthread_join(p2,NULL);
	pthread_join(p3,NULL);
	pthread_join(p4,NULL);
	printf("100张票卖完了!\n");
	semctl(semid,0,IPC_RMID);
	return 0;
}
