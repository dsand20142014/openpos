#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int main(int argc,char *argv[])
{
	int fd;
	fd = open("/dev/trimag", O_RDWR) ;
	
	read(fd, NULL, NULL, NULL);
}
