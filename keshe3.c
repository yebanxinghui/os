#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/module.h>
#include<linux/uaccess.h>	
#include<linux/types.h>
#include<linux/init.h>
#include<linux/cdev.h>

#define MY_MAJOR 241
#define BUFFER_SIZE 1024
char buf[BUFFER_SIZE];

int myopen(struct inode *inode, struct file *file);
int myrelease(struct inode *inode, struct file *file);
ssize_t myread(struct file *file, char __user *p, size_t count, loff_t *f);
ssize_t mywrite(struct file *file, const char __user *p, size_t count, loff_t *f);

const char* devName = "mydrive";//设备名

struct file_operations pStruct =
{ 
    .owner = THIS_MODULE,
    .open = myopen, 
    .release = myrelease,
    .read = myread, 
    .write = mywrite, 
};
static int char_init(void)//注册
{
	int ret = register_chrdev(MY_MAJOR, devName, &pStruct);
	if(ret<0)
	{
		printk("Register fail!\n");
		return -1;
	}
	else{
		printk("Register successful!\n");
		return 0;
	}
}

static void char_exit(void)//卸载
{
	unregister_chrdev(MY_MAJOR, devName);
	printk("Unregister successful!\n");
}

int myopen(struct inode *inode, struct file *file)//打开设备
{
    printk("Open successful!\n");
    try_module_get(THIS_MODULE);
    return 0;
}


int myrelease(struct inode *inode, struct file *file)//卸载设备
{
    printk("Release successful!\n");
    module_put(THIS_MODULE);
    return 0;
}

ssize_t myread(struct file *file, char __user *p, size_t count, loff_t *f)//读设备
{
    if(copy_to_user(p,buf,count))//成功返回0，失败返回失败数目
        return 0;
    else
	return count;
}

ssize_t mywrite(struct file *file, const char __user *p, size_t count, loff_t *f)//写设备
{
    if(copy_from_user(buf,p,count))
        return 0;
    else
	return count;
}

MODULE_LICENSE("GPL");
module_init(char_init);
module_exit(char_exit);
