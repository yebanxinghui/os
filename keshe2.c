#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>	
#include<unistd.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<gtk/gtk.h>
#define BUFFER_SIZE 5
int full,empty;//缓冲区信号灯集合
pid_t pro1,pro2,pro3;//3个子进程pid号
int sharebuf[2];//共享缓冲区标识符数组
int key=80;
typedef struct shareBuffer
{
	char buf[BUFFER_SIZE];
	int size;
}ShareBuffer;
ShareBuffer *addr[2];//2个共享缓冲区数组
void P(int semid,int index)
{	struct sembuf sem;	
    sem.sem_num = index;
    sem.sem_op = -1;	
	sem.sem_flg = 0; //操作标记：0或IPC_NOWAIT等
	semop(semid,&sem,1);	//1:表示执行命令的个数	
}
void V(int semid,int index)
{	
	struct sembuf sem;	
	sem.sem_num = index;
	sem.sem_op =  1;
	sem.sem_flg = 0;	
	semop(semid,&sem,1);	
}

void copy(char *buf1,char *buf2,int temp)
{
	int i=0;
	for(i=0;i<temp;i++)
	{
		*buf2++ = *buf1++;
	}
}
void copy2(gchar *buf1,gchar *buf2,int temp)
{
	int i=0;
	for(i=0;i<temp;i++)
	{
		*buf2++ = *buf1++;
	}
}
void destroy(GtkWidget *widget)
{
	gtk_main_quit();
} 

int main(int argc,char *argv[])
{
	full = semget(1000,2,IPC_CREAT|0666);//full是信号灯集合的id
	empty = semget(2000,2,IPC_CREAT|0666);//empty是信号灯集合的id
	semctl(full,0,SETVAL,0);
	semctl(full,1,SETVAL,0);
	semctl(empty,0,SETVAL,1);
	semctl(empty,1,SETVAL,1);
	int i=0;
	char id_char[50]; 
	FILE *p = fopen("input.txt","r");
	fseek(p,0L,SEEK_END); /* 定位到文件末尾 */  
	int len=ftell(p); /* 得到文件大小 */  
	fclose(p);
	
	for(i=0;i<2;i++)
	{
		sharebuf[i] = shmget(key,sizeof(struct shareBuffer),0666|IPC_CREAT); /*创建共享存储区*/
		key++;
		addr[i]=(struct shareBuffer *)shmat(sharebuf[i],NULL,0);        /*获取首地址*/
	}
	if((pro1=fork()))//pro1>0表示父进程
	{
		if((pro2=fork()))//pro2>0表示父进程
		{
			if((pro3=fork()))//pro3>0表示父进程
			{
				wait(0);
				wait(0);
				wait(0);
				semctl(full,0,IPC_RMID);
				semctl(full,1,IPC_RMID);
				semctl(empty,0,IPC_RMID);
				semctl(empty,1,IPC_RMID);
				printf("process end\n");
			}
			else{//pro3=0表示子进程3
				for(i=0;i<2;i++)
				{
					sharebuf[i] = shmget(key,sizeof(struct shareBuffer),0666|IPC_CREAT); /*创建共享存储区*/
					key++;
					addr[i]=(struct shareBuffer *)shmat(sharebuf[i],NULL,0);        /*获取首地址*/
				}
				char *filepath="output.txt";
				FILE *fp = fopen(filepath,"w+");
				GtkWidget *window;
				GtkWidget *vbox;
				GtkWidget *label; 
				GtkWidget *text_view;
				GtkTextBuffer *buffer;
				
				gtk_init(&argc, &argv);//启动 GTK 
				window = gtk_window_new(GTK_WINDOW_TOPLEVEL);//创建窗口
				gtk_widget_set_usize(GTK_WIDGET(window), 400, 400);//设置窗口大小
				gtk_window_set_title(GTK_WINDOW(window),"Process 3"); //设置窗口标题名
				g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
				vbox = gtk_vbox_new(FALSE, 0);
				gtk_container_add(GTK_CONTAINER(window),vbox);
				gtk_widget_show(vbox);
				
				//显示父子进程的pid号
				sprintf(id_char, "本进程ID : %d", getpid()); 
				label = gtk_label_new(id_char);
				gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
				gtk_widget_show(label); 
				sprintf(id_char, "父进程ID : %d", getppid()); 
				label = gtk_label_new(id_char);
				gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
				gtk_widget_show(label); 
				
				//显示文本
				text_view=gtk_text_view_new();
				gtk_widget_set_size_request(text_view,400,350);
				gtk_box_pack_start(GTK_BOX(vbox),text_view,FALSE,FALSE,0);
				buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
				gtk_widget_show(text_view);
				gtk_widget_show(window);
		
				GtkTextIter start,end;
				gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer),&start,&end);//获得缓冲区开始和结束位置
				char text[50];	
				gchar text2[50];	
				int current = 0;
				double percent=0.0;
				
				while(1)
				{
					P(full,1);
					fwrite(addr[1]->buf,1,addr[1]->size,fp);
					V(empty,1);
					if(addr[1]->size != BUFFER_SIZE) break;		
					current = current + addr[1]->size;
					percent = 100.0 * current / len;
					sprintf(text,"进度:%.2f\n",percent);
					copy2(text,text2,50);
					gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&start,text2,-1);					
				}	
				gtk_main();			
				fclose(fp);
				exit(0);
			}
		}
		else{//pro2=0表示子进程2
			for(i=0;i<2;i++)
			{
				sharebuf[i] = shmget(key,sizeof(struct shareBuffer),0666|IPC_CREAT); /*创建共享存储区*/
				key++;
				addr[i]=(struct shareBuffer *)shmat(sharebuf[i],NULL,0);        /*获取首地址*/
			}
			GtkWidget *window;
			GtkWidget *vbox;
			GtkWidget *label; 
			GtkWidget *text_view;
			GtkTextBuffer *buffer;
			
			gtk_init(&argc, &argv);//启动 GTK
			window = gtk_window_new(GTK_WINDOW_TOPLEVEL);//创建窗口
			gtk_widget_set_usize(GTK_WIDGET(window), 400,400);//设置窗口大小
			gtk_window_set_title(GTK_WINDOW(window),"Process 2"); //设置窗口标题名
			g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
			vbox = gtk_vbox_new(FALSE, 0);
			gtk_container_add(GTK_CONTAINER(window),vbox);
			gtk_widget_show(vbox);
			
			//显示父子进程的pid号
			sprintf(id_char, "本进程ID : %d", getpid()); 
			label = gtk_label_new(id_char);
			gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
			gtk_widget_show(label); 
			sprintf(id_char, "父进程ID : %d", getppid()); 
			label = gtk_label_new(id_char);
			gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
			gtk_widget_show(label); 
			
			//显示文本
			text_view=gtk_text_view_new();
			gtk_widget_set_size_request(text_view,400,350);
			gtk_box_pack_start(GTK_BOX(vbox),text_view,FALSE,FALSE,0);
			buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
			
		
			GtkTextIter start,end;
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer),&start,&end);//获得缓冲区开始和结束位置
			char text[50];	
			gchar text2[50];	
			int current = 0;
			double percent=0.0;
			gtk_widget_show(text_view);
			gtk_widget_show(window);
			while(1)
			{
				P(full,0);
				P(empty,1);
				addr[1]->size = addr[0]->size;
				copy(addr[0]->buf,addr[1]->buf,addr[1]->size);
				V(empty,0);
				V(full,1);
				if(addr[1]->size != BUFFER_SIZE) break;
				current = current + addr[1]->size;
				percent = 100.0 * current / len;
				sprintf(text,"进度:%.2f\n",percent);
				copy2(text,text2,50);
				gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&start,text2,-1);
			}
			gtk_main();
			exit(0);
		}
	}
	else{//pro3=0表示子进程1
		for(i=0;i<2;i++)
		{
			sharebuf[i] = shmget(key,sizeof(struct shareBuffer),0666|IPC_CREAT); /*创建共享存储区*/
			key++;
			addr[i]=(struct shareBuffer *)shmat(sharebuf[i],NULL,0);        /*获取首地址*/
		}
		char *filepath="input.txt";
		FILE *fp = fopen(filepath,"r");
		
		GtkWidget *window;
		GtkWidget *vbox;
		GtkWidget *label; 
		GtkWidget *text_view;
		GtkTextBuffer *buffer;
		
		gtk_init(&argc, &argv);//启动 GTK 
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);//创建窗口
		gtk_widget_set_usize(GTK_WIDGET(window), 400, 400);//设置窗口大小
		gtk_window_set_title(GTK_WINDOW(window),"Process 1"); //设置窗口标题名
		g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
		vbox = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(window),vbox);
		gtk_widget_show(vbox);
				
		//显示父子进程的pid号
		sprintf(id_char, "本进程ID : %d", getpid()); 
		label = gtk_label_new(id_char);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		gtk_widget_show(label); 
		sprintf(id_char, "父进程ID : %d", getppid()); 
		label = gtk_label_new(id_char);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		gtk_widget_show(label); 
				
		//显示文本
		text_view=gtk_text_view_new();
		gtk_widget_set_size_request(text_view,400,350);
		gtk_box_pack_start(GTK_BOX(vbox),text_view,FALSE,FALSE,0);
		buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
		
		
		GtkTextIter start,end;
		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer),&start,&end);//获得缓冲区开始和结束位置
		char text[50];	
		gchar text2[50];
		int current = 0;
		double percent=0.0;
		gtk_widget_show(text_view);
		gtk_widget_show(window);
		while(1)
		{
			P(empty,0);
			addr[0]->size = fread(addr[0]->buf,1,BUFFER_SIZE,fp);
			V(full,0);
			if(addr[0]->size != BUFFER_SIZE) break;
			current = current + addr[0]->size;
			percent = 100.0 * current / len;
			sprintf(text,"进度:%.2f\n",percent);
			copy2(text,text2,50);
			gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer),&start,text2,-1);
		}
		gtk_main();
		fclose(fp);
		exit(0);
	}
	return 0;
}

/*
sudo wget http://security.ubuntu.com/ubuntu/pool/main/a/apt/libapt-pkg5.0_1.7.0_amd64.deb
sudo wget http://security.ubuntu.com/ubuntu/pool/main/a/apt/apt_1.7.0_amd64.deb

dpkg -i libapt-pkg5.0_1.7.0_amd64.deb
dpkg -i apt_1.7.0_amd64.deb
apt --fix-broken install
*/
/*

*/