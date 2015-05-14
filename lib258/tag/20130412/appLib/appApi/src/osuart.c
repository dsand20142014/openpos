/*
osuart.c
*/




#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <osuart.h>



int  OSUART_tty_init(unsigned char ucport)
{
    int g_tty_fd;
    if (ucport == 0)
    {
        g_tty_fd = open(g_tty_name="/dev/s3c2410_serial0", 2);
    }

    if (ucport == 1)
    {
        g_tty_fd = open(g_tty_name="/dev/s3c2410_serial1", 2);
    }

    if (ucport == 2)
    {
        g_tty_fd = open(g_tty_name="/dev/s3c2410_serial2", 2);
    }

    /*
    if ((g_tty_name = ttyname(1)) && *g_tty_name)
    {
    	g_tty_fd = open(g_tty_name, 2);
    }
    else
    {
    	g_tty_fd = open(g_tty_name="/dev/ttySAC2", 2);
    }
    */
    if ( g_tty_fd <= 0 )
    {
        perror(g_tty_name);
        exit(2);
    }

    return g_tty_fd;

}

void OSUART_tty_mode(int fd, int mode)
{
    int g_tty_fd;
    struct termios g_tty_attr;
    struct termios g_tty_old_attr;

    g_tty_fd = fd;

    switch (mode)
    {
    case OSUART_TTY_OPEN:
        (void)tcgetattr(g_tty_fd, &g_tty_old_attr);
        g_tty_attr = g_tty_old_attr;
        cfmakeraw(&g_tty_attr);

        cfsetspeed(&g_tty_attr, B115200);

        tcsetattr(g_tty_fd, TCSADRAIN, &g_tty_attr);
        break;
    case OSUART_TTY_CLOSE:
        tcsetattr(g_tty_fd, TCSADRAIN, &g_tty_old_attr);
        break;
    }
}

int OSUART_tty_open(unsigned char ucport)
{
    int fd;

    fd = OSUART_tty_init(ucport);
    OSUART_tty_mode(fd, OSUART_TTY_OPEN);

    return fd;
}

void OSUART_tty_close(int fd)
{
    OSUART_tty_mode(fd, OSUART_TTY_CLOSE);
    close(fd);
}

int OSUART_tty_SendByte(int fd, unsigned char	*pucData, unsigned int uiLen)
{
    unsigned int uiLoop;
    int g_tty_fd=fd;


    /*Uart_Printf("pp1000a send data:\n");
    for (uiLoop = 0; uiLoop < uiLen; uiLoop++)
    {
    	Uart_Printf("%02x ", *(pucData + uiLoop));
    }
    Uart_Printf("\n");
    */
    for (uiLoop = 0; uiLoop < uiLen; uiLoop++)
    {
        if (write(g_tty_fd, pucData + uiLoop, 1) < 0)
        {
            return -1;
        }
    }
	Uart_Printf("%s OK\n", __func__);
    return uiLoop;
}

int OSUART_tty_RecvByte(int fd, unsigned char *pucData, unsigned int uiLen)
{
    unsigned int uiLoop;
    int g_tty_fd=fd, retval=0;

#if 1
       fd_set rfds;
       struct timeval tv ;


	for (uiLoop = 0; uiLoop < uiLen; uiLoop++)
    	{    
		FD_ZERO(&rfds);
		FD_SET(g_tty_fd, &rfds);

		tv.tv_sec=10;
		tv.tv_usec = 0;


		retval = select(g_tty_fd+1, &rfds, NULL, NULL, &tv);
		if(retval){
			if (FD_ISSET(g_tty_fd, &rfds))
				retval = read(g_tty_fd, pucData + uiLoop, 1) ;		
		}else
		{
			perror("select()");
			return -1;
		}
    	}

#else
	
    for (uiLoop = 0; uiLoop < uiLen; uiLoop++)
    {
        if (read(g_tty_fd, pucData + uiLoop, 1) < 0)
        {
            return -1;
        }
	//Uart_Printf("R:%02x\n", pucData[uiLoop]);		
    }
#endif
    return uiLoop;
}

