#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <errno.h>

#define PRINTER "/dev/printer"

static int print_square(void)
{
	int fd,i,j;
	int ret;
	char a[48*8];
	fd=open(PRINTER,O_RDWR);
	if(fd<0)
	{
		perror("open printer");
		return -1;
	}
	for(i=0;i<400;i++)
	{
		if(i%2==0)
		{
			for(j=0;j<sizeof(a);j++)
			{
				if(j%48%2==0)
					a[j]=0xFF;
				else
					a[j]=0x00;
			}
		}
		else
		{
			for(j=0;j<sizeof(a);j++)
			{
				if(j%48%2==0)
					a[j]=0x00;
				else
					a[j]=0xFF;
			}
		}
		ret=write(fd,a,sizeof(a));
		if(ret!=sizeof(a))
		{
			printf("%d\n",ret);
		}
	}
	close(fd);
	return ret;

}

static int print_num(void)
{
        int fd,i;
        int ret;
        unsigned char a[48*6]={0};
        fd=open(PRINTER,O_RDWR);
        if(fd<0)
        {
                perror("open printer");
                return -1;
        }
	for(i=0;i<256;i++)
	{
		a[i]=i;
	}
	ret=write(fd,a,sizeof(a));
	if(ret!=sizeof(a))
	{
		printf("%d\n",ret);
	}
	close(fd);
	return ret;	
}

int main(int argc,char** argv)
{
	return print_square();
}
