#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>    
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>   
#include <termios.h> 
#include <errno.h> 
#include <string.h>
#include <sys/ioctl.h>
#include "bar.h"

#define LOW_LEVEL	0
#define HIGH_LEVEL	1
#define SCANNER_PWRON  5
#define SCANNER_PWROFF	 6


/**
*@def TRIG_DEV
*@brief 驱动设备结点
**/
#define TRIG_DEV "/dev/bar"

/**
*@def scanCom
*@brief 条码枪输出串口
**/
static const char *scanCom = "/dev/ttymxc0,9600,N,8,1";



int comfd,trigfd;

typedef enum{NAME=0,SPEED,PARITY,DATA,STOP,FLOWCTL,MAX}comdesc_t;
#define SCAN_DEBUG
#ifdef SCAN_DEBUG
#define debug(format, arg...) do{printf("%s: %d " format "\n" , __FUNCTION__ ,__LINE__, ## arg); } while (0)
#else
#define debug(format,arg...) 
#endif

static int speed_arr[] = { B460800,B230400, B115200,B38400, B19200, B9600, B4800, B2400, B1200, B300,
					B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static int name_arr[] = {460800,230400, 115200,38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
					19200,  9600, 4800, 2400, 1200,  300, };

void set_Speed(int fd, int speed){
	int   i; 
	int   status; 
	struct termios	 Opt;
	tcgetattr(fd, &Opt); 
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) { 
		if	(speed == name_arr[i]) {	 
			tcflush(fd, TCIOFLUSH); 	
			cfsetispeed(&Opt, speed_arr[i]);  
			cfsetospeed(&Opt, speed_arr[i]);   
			status = tcsetattr(fd, TCSANOW, &Opt);  
			if	(status != 0)  	   
				debug("tcsetattr fd1\n");  	
			else	 
				tcflush(fd,TCIOFLUSH);	
			return;
		}  
	}
}

int set_Parity(int fd,int databits,int stopbits,char parity,int flowctl)
{ 
	struct termios options;

	if(tcgetattr(fd, &options) != 0){
		debug("Com:   tcgetattr fail\n");
		return BAR_COMERR;
	}
	
	options.c_cflag &= ~CSIZE;
	switch(databits){
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			debug("Com:   Unsupported data size\n"); 
			return BAR_COMERR;
	}

	switch(parity){
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;                       
			options.c_iflag &= ~INPCK;                      
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 's':
		case 'S':
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			debug("Com:   Unsupported parity\n");
			return BAR_COMERR;
	}

	switch(stopbits){
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			debug("Com:   Unsupported stop bits\n");
			return BAR_COMERR;
	}


	options.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG|XCASE|ECHONL|NOFLSH);      
	options.c_iflag &= ~(IGNBRK|IGNCR|INLCR|ICRNL|IUCLC|ISTRIP|BRKINT);
	options.c_iflag |= IGNPAR;
	options.c_oflag &= ~OPOST;		
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cc[VTIME] = 20;                             
	options.c_cc[VMIN]  = 0;    

	switch(flowctl){
		case 1://hwflow control
		options.c_cflag |= CRTSCTS;
		break;
		case 2://swf control
		options.c_iflag |= IXON | IXOFF;
		break;
		case 0://none flow control
		default:
		options.c_cflag &= ~CRTSCTS;
		options.c_iflag &= ~(IXON|IXOFF|IXANY);
		break;
	}
    
	tcflush(fd, TCIFLUSH);											
	if(tcsetattr(fd, TCSANOW, &options) != 0){
		debug("Com:   Setup Serial Port\n");
		return BAR_COMERR;	
	}
	return BAR_OK;
}

int serial_open(const char * szComDesc)
{
	comdesc_t i;
	int serial_fd;
	char * comdesc[MAX];	
	char desc_buf[64];
	char * ptmp = desc_buf;
	
	if(!szComDesc || strlen(szComDesc)>63){
		debug("ComDesc is too long\n");
		return BAR_COMERR;
	}
	else{
		while(*szComDesc){
			if(*szComDesc == ' ')
				szComDesc++;
			else
				*ptmp++ = *szComDesc++;
		}
		*ptmp = 0;	
	}
	for(i = NAME;i < MAX;i++)
		comdesc[i] = NULL;
	
	comdesc[NAME] = strtok(desc_buf,",");
	for(i = SPEED;i < MAX;i++){
		comdesc[i] = strtok(NULL,",");
		if(!comdesc[i])
			break;
	}
	if( i <= STOP){
		debug("Format of serial's name is error\n");
		return BAR_COMERR;
	}
	serial_fd = open(comdesc[NAME],O_RDWR);
	if(serial_fd < 0){
		debug("Open Serial %s error\n",comdesc[NAME]);
		return BAR_INUSE;
	}
	else{
		if(!comdesc[FLOWCTL]){
			char zero = '0';
			comdesc[FLOWCTL] = &zero;
		}
		set_Speed(serial_fd,atoi(comdesc[SPEED]));
		set_Parity(serial_fd,atoi(comdesc[DATA]),\
			atoi(comdesc[STOP]),*comdesc[PARITY],\
			atoi(comdesc[FLOWCTL]));
	}	
	return serial_fd;
}

int Os__bar_open()
{
	int fd = -1;
	int ctrlbits;

	if ((fd = serial_open(scanCom)) < 0)
		return fd;

	ioctl(fd, TIOCMGET, &ctrlbits);
  	ctrlbits &= ~TIOCM_RTS; 
  	ioctl(fd, TIOCMSET, &ctrlbits); 
	
	comfd = fd;

	if((fd = open(TRIG_DEV,O_RDONLY)) < 0)
		return BAR_DRIVER;

	trigfd = fd;
	ioctl(trigfd,SCANNER_PWRON);

	return 0;
}

void Os__bar_close()
{
	//close serial
	close(comfd);

	//close bar device
	ioctl(trigfd, HIGH_LEVEL);
	ioctl(trigfd,SCANNER_PWROFF);
	close(trigfd);
}


void _bar_scan()
{
	ioctl(trigfd, HIGH_LEVEL);
	usleep(2000);
	ioctl(trigfd, LOW_LEVEL);
}

int Os__bar_read(unsigned char *buf, int len)
{
	int n=0;
	
	fflush(stdout);
	usleep(10000);
	_bar_scan();

	while(1)
	{
		//PS420使用SE955型扫描枪，条码后面跟0x0A 0x0D两个结束符
		if(read(comfd, &buf[n], 1) > 0)
		{
			if(buf[n] == 0x0D)
			{
				buf[n] = '\0';
				break;
			}
			else if(buf[n] == 0x0A)
				continue;
			if(++n >= len)
				break;
		}
		else
			break;
	}

	return n;			
}
