#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
int main()
{
         long ret = syscall(335,"cat.png","cat3.png");
         printf("return code is: %ld\n", ret);
         return 0;
}

/*
int main(int argc,char **argv)
{
	int i=syscall(335,argv[1],argv[2]);
	printf("Successfully!\n");
	printf("%d\n",i);
	return 1;
}
*/
