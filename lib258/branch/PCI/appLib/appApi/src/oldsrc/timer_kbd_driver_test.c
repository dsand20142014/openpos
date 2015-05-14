/*
	timer_kbd_driver demo
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
#include "timer_kbd_driver.h"
#include "../tax/osdriver.h"

int main()
{
    int fd;
    unsigned int buf[20];
    unsigned int status=0;
    unsigned char tmp;


    fd = open("/dev/ebc_timer_kbd", O_RDWR);
    if ( fd == -1 )
    {
        Uart_Printf("Cann't open file ");
        exit(0);
    }
    Uart_Printf("0\n");

    ioctl(fd, CMD_TIMER_KBD_ADD);

    Uart_Printf("1\n");

#if 0
    ioctl(fd, CMD_TIMER_KBD_STATUS, &status);
    Uart_Printf("status=%d\n", status);
    goto exit;
#else

    do
    {
        ioctl(fd, CMD_TIMER_KBD_STATUS, &status);
        Uart_Printf("status=%d\n", status);
    }
    while (status!=DRV_ENDED);

    ioctl(fd, CMD_TIMER_KBD_VALUE, &tmp);
    Uart_Printf("data=%d\n", tmp);

#endif

    Uart_Printf("2\n");

exit:
    close(fd);
    return 0;
}

