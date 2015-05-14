/****************************************************************** 
*  simple uart interface 
*****************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>  

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/poll.h>

const int speed_arr[] = { B38400, B19200, B115200,B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
const int name_arr[] = {38400,  19200,  115200, 9600,  4800,  2400,  1200,  300,
	    38400,  19200,  9600, 4800, 2400, 1200,  300, };

int uart_open(char *dev_name)
{
	int fd;

	fd = open( dev_name, O_RDWR|O_NOCTTY|O_NDELAY); 			 //|O_NOCTTY|O_NDELAY);         
	if (-1 == fd) {
        perror("Can't Open Serial Port");
        return -1;
	}
	else {
		fcntl(fd, F_SETFL, 0);// reset 
	}
	return fd;
}

void uart_close(int fd)
{
    close(fd);
}

#if 0
int uart_set_parity(int fd,int databits,int stopbits,int parity,int flow_control)
{
	  struct termios options;

      if ( tcgetattr( fd,&options)  !=  0) {
  			perror("tcgetattr:");
  			return (-1);
   	  }
		 
	  options.c_cflag |= (CLOCAL | CREAD);
	  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	  options.c_oflag &= ~OPOST;
	  options.c_cflag &= ~CRTSCTS;
	  options.c_cc[VMIN]  = 0;
	  options.c_cc[VTIME] = 10;
	  
	  options.c_iflag &= ~(ICRNL | PARMRK | IGNCR | IUCLC);        /* don't map CR to NL */
														   /* slove 0x0D 0x0A problem */

	  //options.c_cflag &= ~PARENB; 
	  //options.c_cflag &= ~CSTOPB; 
	  //options.c_cflag &= ~CSIZE; 
	  //options.c_cflag |= CS8; 

      tcflush(fd,TCIFLUSH); 		
											   
      if (tcsetattr(fd,TCSANOW,&options) != 0) {
          perror("tcsetattr:");
          return (-1);
	  }

  	  return (0);
 }
#endif

int uart_set_parity(int fd,int databits,int stopbits,int parity,int flow_control,int speed)
{
        struct termios oldtio, newtio;
        if ((tcgetattr(fd, &oldtio)) != 0) {
                //printf("Save old error!\n");
                return -1;
        }

        cfmakeraw(&newtio);

        // memset(&newtio, 0x00, sizeof(newtio));
        // newtio.c_cflag |= (CLOCAL | CREAD);
        // newtio.c_cflag &= ~CSIZE;
        // newtio.c_cflag &= ~CSTOPB;// 1 stop bit
        // newtio.c_cflag &= ~PARENB;// No parity
        // newtio.c_cflag |= CS8; // 8 bits data

        /*设置波特率*/
        //cfsetispeed(&newtio, B115200);
        //cfsetospeed(&newtio, B115200);
        switch (speed) {
        case 9600:
                cfsetispeed(&newtio, B9600);
                cfsetospeed(&newtio, B9600);
                break;
        case 19200:
                cfsetispeed(&newtio, B19200);
                cfsetospeed(&newtio, B19200);
                break;
        case 115200:
                cfsetispeed(&newtio, B115200);
                cfsetospeed(&newtio, B115200);
                break;
        default:
                //printf("Gprs_UartSpeed_Setting  parameter fault!\n");
                break;
        }

//        newtio.c_cc[VTIME] = 0;
//        newtio.c_cc[VMIN]  = 0;
        tcflush(fd, TCIFLUSH);

        if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
                //printf("Set new error!\n");
                return -1;
        }

        return 0;
}


void uart_set_speed(int fd, int speed)
{
  	int   i;
  	int   status;
  	struct termios Opt;
	
  	tcgetattr(fd, &Opt);
	
 	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
        if (speed == name_arr[i]) {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if (status != 0)
                perror("tcsetattr fd1");
            return;
        }
        tcflush(fd,TCIOFLUSH);
   	}
	return;
}

void inline uart_clear_send_buffer(int fd)
{
    tcflush(fd,TCOFLUSH); 	
}

void inline uart_clear_rec_buffer(int fd)
{
	tcflush(fd,TCIFLUSH); 
}

void inline uart_clear_send_and_rec_buffer(int fd)
{
	tcflush(fd,TCIOFLUSH); 
}



int uart_recv_timeout(int fd,unsigned char *buff,int len, int seconds)
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    //uart_clear_send_and_rec_buffer(fd);

    FD_ZERO(&rfds);
    FD_SET(fd,&rfds);

    tv.tv_sec  = seconds;
    tv.tv_usec = 0;

    retval = select(fd+1,&rfds,NULL,NULL,&tv);

    if (retval < 0 ){
       //gprs_info("%s:%d  select error \n",__func__,__LINE__);
       return -1;
    } 
    if (retval == 0){
       //gprs_info("%s:%d  select time out \n",__func__,__LINE__);
        return 0;
    }

    return read(fd,buff,len);
}

int uart_send(int fd,unsigned char *buff,int len)
{
    return write(fd, buff, len);
}

