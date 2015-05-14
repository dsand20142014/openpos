/*
	timer_driver demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <asm/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include     <unistd.h>     /*Unix¡À¨º¡Á1/4^(o)?E^y'?¡§O`a**/
#include	 <sys/signal.h>
#include     <termios.h>    /*PPSIXO"O~?E"??O"??¡§O`a**/
#include     <errno.h>      /*?¨ªI^¨®^(o)A*?¡§O`a**/
#include <linux/ioctl.h>
#include "timer_driver.h"

int timer_main()
{
    int fd;
    int result;
    int i;
    char buf[]="helloworld";
    char inbuf[20];
    unsigned long ioctltest;
    unsigned long delaytime=1000, remain=1;

    memset(inbuf, 0, 20);

    fd = open("/dev/ebc_timer", O_RDWR);
    if ( fd == -1 )
    {
        Uart_Printf("Cann't open file\n");
        exit(0);
    }

    ioctl(fd, CMD_PIT_ADD, &delaytime);

#if 1
    do
    {
        ioctl(fd, CMD_PIT_GET, &remain);
        Uart_Printf("remain=%d\n", remain);
    }
    while (remain>0);
#endif


    Uart_Printf("stop!\n");

exit:
    close(fd);
    return 0;
}

