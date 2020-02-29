#include<stdio.h>
#include<stdlib.h>
#include<string.h>	
#include<unistd.h>
#include<dirent.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/vfs.h>
//#include<sys/time.h>
#include<gtk/gtk.h>
#include<glib.h>
#include<time.h>
#undef TRUE
#define TRUE 1
#define BUFFER_SIZE 1024
char buf[BUFFER_SIZE];

char *txt_pid=NULL;
static long idle,total;		     //计算cpu时的数据
static int flag=0;				 //计算cpu使用率时启动程序的标志
static int flag1=0;				 //计算单个进程cpu使用率时使用的标志
static char temp_cpu[50];		 //打印专用：cpu使用率 
static char temp_mem[50];		 //打印专用：内存使用率
static char temp_time[50];		 //打印专用：当前时间
static float cpu_used_percent=0; //cpu使用率 
static float cpu_data[151];		 //cpu历史数据 
static float mem_data[151];		 //内存历史数据
static int flag2=0;				 //初始化cpu_data数组中数据的标志 
static int flag3=0;				 //初始化mem_data数组中数据的标志
static int cpu_first_data=0;	 //cpu第一个数据 
static int mem_first_data=0;	 //内存第一个数据
static int cpu_start_position=15;//绘制cpu移动的线条 
static int mem_start_position=15;//绘制内存移动的线条
static long mem_total;		 	 //内存总大小
static long mem_free;			 //空闲内存
static GtkWidget *cpu_record_drawing_area;
static GtkWidget *mem_record_drawing_area;
static GtkWidget *notebook;
static GtkWidget *entry;		 //新进程
static GtkWidget *entry2;		 //搜索
void destroy(GtkWidget *widget,GdkEvent *event,gpointer data)
{
	gtk_main_quit();
} 

char *get_sys_hostname(char *hostname_buf)//功能1
{
	FILE *fp;
	int i=0;
	char *buf = hostname_buf;
	fp = fopen("/proc/sys/kernel/hostname","r");
	fgets(buf,256,fp);
	fclose(fp);
	return buf;
}

double get_start_time()//功能2
{
	FILE *fp;
	int i=0;
	double num;
	fp = fopen("/proc/uptime","r");
	fscanf(fp,"%lf",&num);
	fclose(fp);
	return num;
}

double get_last_time()//功能3
{
	FILE *fp;
	int i=0;
	double num1,num2;
	fp = fopen("/proc/uptime","r");
	fscanf(fp,"%lf %lf",&num1,&num2);
	fclose(fp);
	return num2;
}

char *get_sys_version(char *ver_buf,char *tmp_buf)//功能4
{
	FILE *fp;
	FILE *fp2;
	int i=0;
	char *p1 = ver_buf;
	char *p2 = tmp_buf;
	fp = fopen("/proc/sys/kernel/ostype","r");
	fgets(p1,256,fp);
	fp2 = fopen("/proc/sys/kernel/osrelease","r");
	fgets(p2,256,fp2);
	p1 = ver_buf;
	while(*p1 != '\0') p1++;
	p1--;
	*p1++ = ' ';
	while(*p2 != '\0')
	{
		*p1=*p2;
		p1++;
		p2++;
	}
	*p1='\0';
	p1 = ver_buf;
	fclose(fp);
	fclose(fp2);
	return p1;
}

char *get_cpu_name(char *name_buf)//功能5
{
	FILE * fp;
	int i=0;
	char *buf = name_buf;
    fp=fopen("/proc/cpuinfo","r");
	for(i=0;i<5;i++)
		fgets(buf,256,fp);
	for(i=0;i<256;i++){
		if(buf[i]==':')
			break;
	}
	i += 2;
	buf += i;
	buf[41]='\0';
	fclose(fp);
   	return buf;
}

char *get_cpu_type(char *type_buf)//功能5
{
	FILE * fp;
	int i=0;
	char *buf = type_buf;
    fp=fopen("/proc/cpuinfo","r");
	for(i=0;i<2;i++)
		fgets(buf,256,fp);
	for(i=0;i<256;i++){
		if(buf[i]==':')
			break;
	}
	i += 2;
	buf += i;
	buf[12]='\0';
	fclose(fp);
   	return buf;
}

char *get_cpu_f(char *f_buf)//功能5
{
	FILE * fp;
	int i=0;
	char *buf = f_buf;
    fp=fopen("/proc/cpuinfo","r");
	for(i=0;i<8;i++)
		fgets(buf,256,fp);
	for(i=0;i<256;i++){
		if(buf[i]==':')
			break;
	}
	i += 2;
	buf += i;
	buf[8]='\0';
	fclose(fp);
   	return buf;
}

void Search(gpointer data)//功能6
{
	const gchar *text;
	text = gtk_entry_get_text(GTK_ENTRY(entry2));
	char pid[100];
	stpcpy(pid,text);
	int i;
	FILE *fp = NULL;
	char file[512] = "";
	char buf[9][256] = {0};
	char msg[256];
	int flag = 0;
	
	char num[100];//存pid号
	char name[100];//名称
	if(pid[0]<'0'||pid[0]>'9')//输入的是数字，也就是pid号
	{
		char buf2[2][100];
		FILE *fp;
		DIR *dir = opendir("/proc");
		struct dirent *p;
		unsigned int length=0;
		while(p = readdir(dir))
		{
			if((p->d_name)[0]>='0'&&(p->d_name)[0]<='9')//文件名第一个字符时数字
			{
				sprintf(file,"/proc/%s/stat",p->d_name);
			
				fp = fopen(file,"r");
				fscanf(fp,"%s (%s",buf2[0],buf2[1]);
				strcpy(num,buf2[0]);
				strcpy(name,buf2[1]);
				length = strlen(name)-1;
				name[length]='\0';
				for(i=0;i<length;i++)
				{
					if(name[i]==pid[i])
						flag=1;
					else{
						flag=0;
						break;
					}
				}
				fclose(fp);
				if(flag==1){
					strcpy(pid,num);
					break;
				}
			}
		}
		closedir(dir);
		if(flag == 0)
		{
			strcpy(msg, "process name is wrong\nplease input the right process name of process!\n");
			GtkWidget *dialog ;
			dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, msg, "");
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			return;
		}
	}
	sprintf(file, "/proc/%s/status", pid);
	if(!(fp = fopen(file, "r")))
	{
		strcpy(msg, "pid is wrong\nplease input the right id of process!\n");
		GtkWidget *dialog ;
		dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, msg, "");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return;
	}
	
	for(i = 0; i < 9; i++)
		fgets(buf[i], sizeof(buf[i]), fp);
	strcpy(msg, buf[0]);
	for(i = 1; i < 9; i++)
		strcat(msg, buf[i]);


	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, msg, "");
	gtk_window_set_title(GTK_WINDOW(dialog), "杀死进程?");
	GtkResponseType result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	char bufkill[100]="";
	if (result == GTK_RESPONSE_YES || result == GTK_RESPONSE_APPLY)
	{
		sprintf(bufkill,"kill -s 9 %s",pid);
		system(bufkill);
	}
}

void get_proc_info(GtkWidget *list,int *A,int *I,int *R,int *S, int *Z)//功能7
{
	gtk_clist_set_column_title(GTK_CLIST(list),0,"PID");
	gtk_clist_set_column_title(GTK_CLIST(list),1,"进程名");
	gtk_clist_set_column_title(GTK_CLIST(list),2,"状态"); 
	gtk_clist_set_column_title(GTK_CLIST(list),3,"PPID");
	gtk_clist_set_column_title(GTK_CLIST(list),4,"优先级"); 
	gtk_clist_set_column_title(GTK_CLIST(list),5,"内存占用");
	gtk_clist_set_column_width(GTK_CLIST(list),0,50);
    gtk_clist_set_column_width(GTK_CLIST(list),1,100);
    gtk_clist_set_column_width(GTK_CLIST(list),2,50);
    gtk_clist_set_column_width(GTK_CLIST(list),3,50);
	gtk_clist_set_column_width(GTK_CLIST(list),4,50);
    gtk_clist_set_column_width(GTK_CLIST(list),5,55);
	gtk_clist_column_titles_show(GTK_CLIST(list)); 
	
	char buf[300];//存文件名(也就是pid号)
	char buf2[24][100];//存stat信息,I是空闲,R是正在运行,S是睡眠,Z是僵尸进程
	char pid[100];//pid号
	char name[100];//名称
	char state[100];//状态
	char ppid[100];//ppid号
	char pri[100];//优先级
	char room[100];//内存占用
	gchar *txt[6];
	FILE *fp;
	DIR *dir = opendir("/proc");
	struct dirent *p;
	
	while(p = readdir(dir))
	{
		if((p->d_name)[0]>='0'&&(p->d_name)[0]<='9')//文件名第一个字符时数字
		{
			(*A)++;//统计进程总数
			sprintf(buf,"/proc/%s/stat",p->d_name);
			
			fp = fopen(buf,"r");
			fscanf(fp,"%s (%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
buf2[0],buf2[1],buf2[2],buf2[3],buf2[4],buf2[5],buf2[6],buf2[7],buf2[8],buf2[9],buf2[10],buf2[11],buf2[12],buf2[13],buf2[14],buf2[15],buf2[16],buf2[17],buf2[18],buf2[19],buf2[20],buf2[21],buf2[22],buf2[23]);
			strcpy(pid,buf2[0]);
			strcpy(name,buf2[1]);
			strcpy(state,buf2[2]);
			strcpy(ppid,buf2[3]);
			strcpy(pri,buf2[18]);
			strcpy(room,buf2[23]);
			name[strlen(name)-1]='\0';
			if(state[0] == 'I') (*I)++;
			else if(state[0] == 'R') (*R)++;
			else if(state[0] == 'S') (*S)++;
			else if(state[0] == 'Z') (*Z)++;
			
			txt[0] = pid;
			txt[1] = name;
			txt[2] = state;
			txt[3] = ppid;
			txt[4] = pri;
			txt[5] = room;
			gtk_clist_append(GTK_CLIST(list),txt);
			fclose(fp);
		}
	}
	closedir(dir);
}

void refresh(GtkWidget *list)
{
	gtk_clist_clear(GTK_CLIST(list));
    gtk_clist_set_column_title(GTK_CLIST(list),0,"PID");
	gtk_clist_set_column_title(GTK_CLIST(list),1,"进程名");
	gtk_clist_set_column_title(GTK_CLIST(list),2,"状态"); 
	gtk_clist_set_column_title(GTK_CLIST(list),3,"PPID");
	gtk_clist_set_column_title(GTK_CLIST(list),4,"优先级"); 
	gtk_clist_set_column_title(GTK_CLIST(list),5,"内存占用");
	gtk_clist_set_column_width(GTK_CLIST(list),0,50);
    gtk_clist_set_column_width(GTK_CLIST(list),1,100);
    gtk_clist_set_column_width(GTK_CLIST(list),2,50);
    gtk_clist_set_column_width(GTK_CLIST(list),3,50);
	gtk_clist_set_column_width(GTK_CLIST(list),4,50);
    gtk_clist_set_column_width(GTK_CLIST(list),5,55);
	gtk_clist_column_titles_show(GTK_CLIST(list)); 
	
	char buf[300];//存文件名(也就是pid号)
	char buf2[24][100];//存stat信息,I是空闲,R是正在运行,S是睡眠,Z是僵尸进程
	char pid[100];//pid号
	char name[100];//名称
	char state[100];//状态
	char ppid[100];//ppid号
	char pri[100];//优先级
	char room[100];//内存占用
	gchar *txt[6];
	FILE *fp;
	DIR *dir = opendir("/proc");
	struct dirent *p;
	
	while(p = readdir(dir))
	{
		if((p->d_name)[0]>='0'&&(p->d_name)[0]<='9')//文件名第一个字符时数字
		{
			sprintf(buf,"/proc/%s/stat",p->d_name);
			
			fp = fopen(buf,"r");
			fscanf(fp,"%s (%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
buf2[0],buf2[1],buf2[2],buf2[3],buf2[4],buf2[5],buf2[6],buf2[7],buf2[8],buf2[9],buf2[10],buf2[11],buf2[12],buf2[13],buf2[14],buf2[15],buf2[16],buf2[17],buf2[18],buf2[19],buf2[20],buf2[21],buf2[22],buf2[23]);
			strcpy(pid,buf2[0]);
			strcpy(name,buf2[1]);
			strcpy(state,buf2[2]);
			strcpy(ppid,buf2[3]);
			strcpy(pri,buf2[18]);
			strcpy(room,buf2[23]);
			name[strlen(name)-1]='\0';
			
			txt[0] = pid;
			txt[1] = name;
			txt[2] = state;
			txt[3] = ppid;
			txt[4] = pri;
			txt[5] = room;
			gtk_clist_append(GTK_CLIST(list),txt);
			fclose(fp);
		}
	}
	closedir(dir);
}

void select_row_callback(GtkWidget *list,gint row,gint column,GdkEventButton *event,gpointer data)
{
	//设置select_row信号的回调函数
	txt_pid = NULL;
	gtk_clist_get_text(GTK_CLIST(list),row,column,&txt_pid);
	printf("%s\n",txt_pid);
}

void kill_proc(void)//功能6
{
    char buf[30]="";
	if(txt_pid == NULL) return;
	else if(txt_pid[0] >= '0' && txt_pid[0] <= '9'){
		//printf("%s\n",txt_pid);
		sprintf(buf,"kill -s 9 %s",txt_pid);//9表示强制结束进程
	}
	else ///printf("%s\n",txt_pid);
		sprintf(buf,"pkill %s",txt_pid);//9表示强制结束进程
    system(buf);
}

gboolean cpu_draw(GtkWidget *widget)//功能8
{
	int my_first_data;
	GdkColor color;
	GdkDrawable *canvas;
	GdkGC *gc;
	GdkFont *font;
	canvas = widget->window; 
	gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
 
	gdk_draw_rectangle(canvas, gc, TRUE, 10, 5, 360, 150);
	color.red = 0;
	color.green = 20000;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	int i;	
	for(i=20;i<155;i+=15)//绘制横线
		gdk_draw_line(canvas, gc, 10, i, 370, i);
	for(i=10;i<360;i+=15)//绘制竖线
		gdk_draw_line(canvas, gc, i+cpu_start_position,5, i+cpu_start_position,155);			
	cpu_start_position-=3;
	if(cpu_start_position==0)  cpu_start_position=15;
	if(flag2==0)//第一次清空数据
	{
		for(i=0;i<121;i++)
			cpu_data[i]=0;
		flag2=1;
	}

	cpu_data[cpu_first_data]=cpu_used_percent/100;
	cpu_first_data++;	
	//if(cpu_first_data==120) cpu_first_data=0;

	color.red = 0;
	color.green = 65535;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	
	my_first_data=cpu_first_data;
	for(i=0;i<120;i++)
	{
		gdk_draw_line(canvas,gc,10+i*3,154-149*cpu_data[my_first_data%121],10+(i+1)*3,154-149*cpu_data[(my_first_data+1)%121]);
		my_first_data++;
		if(my_first_data==121)	my_first_data=0;
	}
 	color.red = 0;
	color.green = 0;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	return TRUE;
}
gboolean cpu_record_callback(GtkWidget *widget,GdkEventExpose *event,gpointer data)//cpu使用记录回调函数 
{
	gtk_timeout_add(1000,(GtkFunction)cpu_draw,(gpointer)widget);//每秒调用一次cpu_record_draw
	return TRUE;
}

gboolean mem_draw(GtkWidget *widget)//功能9
{
	int my_first_data;
	GdkColor color;
	GdkDrawable *canvas;
	GdkGC *gc;
	GdkFont *font;
	canvas = widget->window; 
	gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
 
	gdk_draw_rectangle(canvas, gc, TRUE, 10, 5, 360, 150);
	color.red = 0;
	color.green = 20000;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	int i;	
	for(i=20;i<155;i+=15)//绘制横线
		gdk_draw_line(canvas, gc, 10, i, 370, i);
	for(i=10;i<360;i+=15)//绘制竖线
		gdk_draw_line(canvas, gc, i+mem_start_position,5, i+mem_start_position,155);			
	mem_start_position-=3;
	if(mem_start_position==0)  mem_start_position=15;
	if(flag3==0)//第一次清空数据
	{
		for(i=0;i<121;i++)
			mem_data[i]=0;
		flag3=1;
	}

	mem_data[mem_first_data]=(float)(mem_total-mem_free)/mem_total;
	mem_first_data++;	
	//if(mem_first_data==120) mem_first_data=0;

	color.red = 0;
	color.green = 65535;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	
	my_first_data=mem_first_data;
	for(i=0;i<120;i++)
	{
		gdk_draw_line(canvas,gc,10+i*3,154-149*mem_data[my_first_data%121],10+(i+1)*3,154-149*mem_data[(my_first_data+1)%121]);
		my_first_data++;
		if(my_first_data==121)	my_first_data=0;
	}
 	color.red = 0;
	color.green = 0;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	return TRUE;
}
gboolean mem_record_callback(GtkWidget *widget,GdkEventExpose *event,gpointer data)//内存记录回调函数 
{
	gtk_timeout_add(1000,(GtkFunction)mem_draw,(gpointer)widget);
	//g_signal_connect(G_OBJECT(mem_record_drawing_area), "configure_event", G_CALLBACK(mem_record_callback), NULL);
	return TRUE;
}

char* cpu_read()//功能11
{
	long user_t, nice_t, system_t, idle_t,total_t;//此次读取的数据
	long total_c,idle_c;//此次数据与上次数据的差
	char cpu_t[10];		
	FILE *fp = fopen("/proc/stat","r");
	fscanf(fp, "%s %ld %ld %ld %ld", cpu_t, &user_t, &nice_t, &system_t, &idle_t);
	if(flag==0)	
	{
		flag=1;	
		idle=idle_t;
		total=user_t+nice_t+system_t+idle_t;
		cpu_used_percent=0;	
	}
	else
	{
		total_t=user_t+nice_t+system_t+idle_t;
		total_c=total_t-total;
		idle_c=idle_t-idle;	
		cpu_used_percent=100*(total_c-idle_c)/total_c;
		total=total_t;	//此次数据保存
		idle=idle_t;
	}
	fclose(fp);	
	sprintf(temp_cpu,"cpu使用率：%0.1f%%",cpu_used_percent);
	return temp_cpu;
}
gboolean cpu_refresh(gpointer cpu_label)
{
	gtk_label_set_text(GTK_LABEL(cpu_label),cpu_read());
	gtk_widget_show(cpu_label);
	return TRUE;
}

char* mem_read()//功能12
{
	char buffer[101],tmp[5];
	char data[20];	
	long total=0,free=0,count=0;	//总内存和用户内存
	
	FILE *fp = fopen("/proc/meminfo","r");
	fscanf(fp,"%s %ld %s\n",buffer,&total,tmp);//MemTotal总内存
	total/=1024;
	fscanf(fp,"%s %ld %s\n",buffer,&free,tmp);//MemFree空闲内存,完全未用到的物理内存=LowFree+HighFree
	fscanf(fp,"%s %ld %s\n",buffer,&count,tmp);//MemAvailable可用内存，≈ MemFree+Buffers+Cached
	//fscanf(fp,"%s %ld %s\n",buffer,&free,tmp);//Buffers设备缓冲
	//fscanf(fp,"%s %ld %s\n",buffer,&free,tmp);//Cached高速缓冲
	count/=1024;
	mem_total = total;
	mem_free = count;
	sprintf(temp_mem,"内存:%ldMB/%ldMB",total-count,total);
	fclose(fp);	
	return temp_mem;
}
gboolean mem_refresh(gpointer mem_label)
{
	gtk_label_set_text(GTK_LABEL(mem_label),mem_read());	
	gtk_widget_show(mem_label);
	return TRUE;
}

char* time_read()//功能10
{
    time_t times;
    struct tm *p_time;
    time(&times);
    p_time = localtime(&times);
	sprintf(temp_time,"时间: %02d:%02d:%02d",p_time->tm_hour, p_time->tm_min, p_time->tm_sec);
    return temp_time;
}
gboolean time_refresh(gpointer time_label)
{
	gtk_label_set_text(GTK_LABEL(time_label),time_read());	
	gtk_widget_show(time_label);
	return TRUE;
}

void on_button_clicked(GtkButton* button,gpointer data)//功能13
{
	const gchar* command;
    command = gtk_entry_get_text(GTK_ENTRY(entry));
    system(command);
}

void shut(void)//功能14
{
    char buf[20];
	sprintf(buf,"shutdown");//shutdown -h now
    system(buf);
}


int main(int argc,char *argv[])
{
	GtkWidget *window;
	GtkWidget *table;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *frame;
    GtkWidget *frame2;
	GtkWidget *frame3;
	GtkWidget *scrolled_window;
	GtkWidget *list;
	GtkWidget *list2;
	GtkWidget *button1;
    GtkWidget *button2;
	GtkWidget *button3;
    GtkWidget *button4;
	GtkWidget *fixed;
	GtkWidget *capability;	//内存资源
	GtkWidget *cpu_record;	//cpu曲线图
    GtkWidget *mem_record;	//内存曲线图
	GtkWidget *cpu_hbox;	//容纳cpu两个图
    GtkWidget *mem_hbox;	//容纳mem两个图
	GtkWidget *time_label;	//当前时间
    GtkWidget *cpu_label;	//cpu使用率
    GtkWidget *mem_label;	//内存使用情况
	GtkWidget *searchbutton;
	char buffer1[256],buffer2[256],buffer3[256];
	char buf1[50];//第一个页面名字
	char n1_buf1[500];//第一个页面第操作系统信息框
    char n1_buf2[500];//第一个页面第CPU信息框
	int A,I,R,S,Z;//A代表总进程数,I是空闲,R是正在运行,S是睡眠,Z是僵尸进程
	A=I=R=S=Z=0;
	
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "监控系统"); 
	gtk_widget_set_size_request (window, 550, 500); 
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);
	
	gtk_container_set_border_width(GTK_CONTAINER(window),10);
	table = gtk_table_new(2, 3, FALSE);
	gtk_container_add(GTK_CONTAINER (window), table); 
	
	//笔记本控件，用于分标签
	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_TOP);
	gtk_table_attach_defaults(GTK_TABLE(table), notebook, 0, 6, 0, 1);
	gtk_widget_show(notebook);
	
	//准备建立系统信息页
	
	sprintf(buf1, "系统信息");
	sprintf(n1_buf1, "操作系统信息");
    sprintf(n1_buf2, "CPU信息");
	vbox = gtk_vbox_new(FALSE,0);
	
	frame = gtk_frame_new(n1_buf1);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 10);
	gtk_widget_set_size_request(frame, 500, 200);
	sprintf(n1_buf1, "该系统主机名：%s系统启动时间：%lf s\n持续运行时间：%lf s\n操作系统版本：%s\n",get_sys_hostname(buffer1),get_start_time(),get_last_time(),get_sys_version(buffer2,buffer3));
	label = gtk_label_new(n1_buf1);
	gtk_container_add(GTK_CONTAINER(frame), label);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,5);
    gtk_widget_show(frame);
	
	frame2 = gtk_frame_new(n1_buf2);
    gtk_container_set_border_width(GTK_CONTAINER(frame2), 10);
    gtk_widget_set_size_request(frame2, 500, 200);
    sprintf(n1_buf2, "  CPU名称：%s\n  CPU类型：%s\n  CPU频率：%s MHz\n",get_cpu_name(buffer1),get_cpu_type(buffer2),get_cpu_f(buffer3));
    label = gtk_label_new (n1_buf2);
    gtk_container_add (GTK_CONTAINER(frame2), label);
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(vbox),frame2,FALSE,FALSE,5);
    gtk_widget_show(frame2);
	
	gtk_widget_show(vbox);
	//正式建立第一页标签
	label = gtk_label_new(buf1);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);
	
	//准备建立进程信息页
	sprintf(buf1,"进程信息");
	hbox=gtk_hbox_new(FALSE, 5);
	
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_size_request(scrolled_window, 350, 300);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	
	//六列，分别是PID，名称，状态，PPID，优先级，内存占用(以字节Byte来算的，除以1024^2就是MB)
	list = gtk_clist_new(6);
	get_proc_info(list,&A,&I,&R,&S,&Z);
	gtk_signal_connect(GTK_OBJECT(list),"select_row",GTK_SIGNAL_FUNC(select_row_callback),NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window),list); 
	gtk_box_pack_start(GTK_BOX(hbox), scrolled_window, TRUE, TRUE, 5);
	
	//右边详细显示状态栏和按钮栏
	vbox = gtk_vbox_new(FALSE, 5); 
	frame = gtk_frame_new(buf1);
	gtk_widget_set_size_request(frame, 100, 215);
	sprintf(n1_buf1,"进程总数: %d\n\n空闲进程: %d\n\n正在运行: %d\n\n睡眠进程: %d\n\n僵尸进程: %d\n",A,I,R,S,Z);
	label = gtk_label_new(n1_buf1);
	gtk_container_add(GTK_CONTAINER(frame), label);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);
	
    button1 = gtk_button_new_with_label("结束进程");
	g_signal_connect(G_OBJECT(button1), "clicked",G_CALLBACK(kill_proc), "结束进程");
	gtk_box_pack_start(GTK_BOX(vbox), button1, FALSE, FALSE, 10);
	
	button2 = gtk_button_new_with_label("刷新");
    g_signal_connect_swapped(G_OBJECT(button2), "clicked",G_CALLBACK (refresh), list);
	gtk_box_pack_start(GTK_BOX(vbox), button2, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 5);
	
	entry2 = gtk_entry_new();
	searchbutton = gtk_button_new_with_label("查询");
	
	gtk_box_pack_start(GTK_BOX(vbox), entry2,TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), searchbutton, TRUE, TRUE, 5);
	gtk_widget_show(entry2);
	gtk_widget_show(searchbutton);
	g_signal_connect(G_OBJECT(searchbutton), "clicked", G_CALLBACK(Search), NULL);
	gtk_widget_show_all(hbox);
	
	//正式建立第二页标签
	label = gtk_label_new(buf1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, label);
	
	
	//建立内存资源信息页（第三页）
	capability = gtk_vbox_new(FALSE,0);	
	gtk_container_set_border_width(GTK_CONTAINER(capability),5);
	gtk_widget_set_size_request(capability,200,320);
	gtk_widget_show(capability);
	label = gtk_label_new(" 内存资源 ");	
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),capability,label);
	
	cpu_hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(capability),cpu_hbox,TRUE,TRUE,2);
	gtk_widget_show(cpu_hbox);
	mem_hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(capability),mem_hbox,TRUE,TRUE,2);
	gtk_widget_show(mem_hbox);
	
	//CPU记录窗口
	cpu_record = gtk_frame_new("cpu使用曲线");
	gtk_container_set_border_width(GTK_CONTAINER(cpu_record),5);
    gtk_widget_set_size_request(cpu_record,500,130);
	gtk_widget_show(cpu_record);
    gtk_box_pack_start(GTK_BOX(cpu_hbox),cpu_record,TRUE,TRUE,2);
	cpu_record_drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(cpu_record_drawing_area, 50,50);
	gtk_widget_set_app_paintable(cpu_record_drawing_area, TRUE);
	g_signal_connect(G_OBJECT(cpu_record_drawing_area), "expose_event",G_CALLBACK(cpu_record_callback),NULL);
	gtk_container_add(GTK_CONTAINER(cpu_record), cpu_record_drawing_area);
	gtk_widget_show(cpu_record_drawing_area);
	
	//内存使用记录窗口
	mem_record = gtk_frame_new("内存使用曲线");
	gtk_container_set_border_width(GTK_CONTAINER(mem_record),5);
    gtk_widget_set_size_request(mem_record,500,130);
    gtk_widget_show(mem_record);
    gtk_box_pack_start(GTK_BOX(mem_hbox),mem_record,TRUE,TRUE,2);

    mem_record_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(mem_record_drawing_area, 50,50);
	gtk_widget_set_app_paintable(mem_record_drawing_area, TRUE);
    g_signal_connect(G_OBJECT(mem_record_drawing_area), "expose_event",G_CALLBACK(mem_record_callback),NULL);
	gtk_container_add (GTK_CONTAINER(mem_record), mem_record_drawing_area);
    gtk_widget_show (mem_record_drawing_area);
	
	//状态栏
	hbox = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(capability),hbox,FALSE,FALSE,2);
    gtk_widget_show(hbox);
	time_label = gtk_label_new("");
    cpu_label = gtk_label_new("");
    mem_label = gtk_label_new("");	

	gtk_timeout_add(1000,(GtkFunction)time_refresh,(gpointer)time_label);//当前时间刷新
    gtk_timeout_add(1000,(GtkFunction)cpu_refresh,(gpointer)cpu_label);	//cpu使用率刷新
    gtk_timeout_add(1000,(GtkFunction)mem_refresh,(gpointer)mem_label);	//内存使用刷新

	gtk_label_set_justify(GTK_LABEL(time_label),GTK_JUSTIFY_RIGHT);
    gtk_label_set_justify(GTK_LABEL(cpu_label),GTK_JUSTIFY_RIGHT);
    gtk_label_set_justify(GTK_LABEL(mem_label),GTK_JUSTIFY_RIGHT);
	
    gtk_box_pack_start(GTK_BOX(hbox),time_label,FALSE,FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox),cpu_label,FALSE,FALSE,10);
    gtk_box_pack_start(GTK_BOX(hbox),mem_label,FALSE,FALSE,10);
	
    gtk_widget_show(time_label);
    gtk_widget_show(cpu_label);
    gtk_widget_show(mem_label);    
    
	//建立第四页	
	vbox = gtk_vbox_new(FALSE, 5); 
	label = gtk_label_new("辅助页面1");	
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);
		
	label = gtk_label_new("输入命令：");
	gtk_container_add(GTK_CONTAINER(vbox), label);
	entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(vbox),entry,TRUE,TRUE,10);
	button3 = gtk_button_new_with_label("执行");
    g_signal_connect(G_OBJECT(button3),"clicked",G_CALLBACK(on_button_clicked),NULL);
	gtk_box_pack_start(GTK_BOX(vbox),button3,TRUE,FALSE,10);
	gtk_widget_show_all(vbox);
	
	//建立第五页
	vbox = gtk_vbox_new(FALSE, 5); 
	label = gtk_label_new("辅助页面2");	
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, label);
	
	button4 = gtk_button_new_with_label("关机");
	g_signal_connect(G_OBJECT(button4),"clicked",G_CALLBACK(shut), "关机");
	gtk_box_pack_start(GTK_BOX(vbox),button4,TRUE,FALSE,10);
	gtk_widget_show_all(vbox);
	
	gtk_widget_show(table);
    gtk_widget_show(window);
    gtk_main();
    return 0;
}