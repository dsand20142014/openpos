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

#define LOW_LEVEL	0
#define HIGH_LEVEL	1
#define USE_Serial_SCANNER 13
#define SCANNER_PWRON  14
#define SCANNER_PWROFF	 15

static const char * scanCom = "/dev/ttymxc1,115200,N,8,1";

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

static void set_Speed(int fd, int speed){
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

static int set_Parity(int fd,int databits,int stopbits,char parity,int flowctl)
{ 
	struct termios options;

	if(tcgetattr(fd, &options) != 0){
		debug("Com:   tcgetattr fail\n");
		return(-1);
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
			return(-1);
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
			return(-1);
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
			return(-1);
	}

	//Set Input parity option
	//if(parity == 'n' || parity == 'N')
	//	options.c_iflag = 0;

	/* if tht port is not for moden, then set it as raw model */
	options.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG|XCASE|ECHONL|NOFLSH);      
	//options.c_oflag = 0;                     
	options.c_iflag &= ~(IGNBRK|IGNCR|INLCR|ICRNL|IUCLC|ISTRIP|BRKINT);
	options.c_iflag |= IGNPAR;
	options.c_oflag &= ~OPOST;		
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cc[VTIME] = 20;                             
	options.c_cc[VMIN]  = 0;    

	switch(flowctl){
		case 1://hwflow control
		printf("hardware flow control.\n");
		options.c_cflag |= CRTSCTS;
		break;
		case 2://swf control
		options.c_iflag |= IXON | IXOFF;
		break;
		case 0://none flow control
		default:
		printf("none flow control.\n");
		options.c_cflag &= ~CRTSCTS;
		options.c_iflag &= ~(IXON|IXOFF|IXANY);
		break;
	}
    
	/* Update the option and do it NOW */
	tcflush(fd, TCIFLUSH);											
	if(tcsetattr(fd, TCSANOW, &options) != 0){
		debug("Com:   Setup Serial Port\n");
		return (-1);	
	}
	return(0);
}

/*serial_open():open serial
*@szComDesc :format --COMn,speed,N,8,1,<x> 
*return value :com's fd
*/

static int serial_open(const char * szComDesc)
{
	comdesc_t i;
	int serial_fd;
	char * comdesc[MAX];	
	char desc_buf[64];
	char * ptmp = desc_buf;
	
	if(!szComDesc || strlen(szComDesc)>63){
		debug("szComDesc is too long\n");
		return -1;
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
		return -3;
	}
	serial_fd = open(comdesc[NAME],O_RDWR);
	printf("open %s fd is %d.\n",comdesc[NAME],serial_fd);
	if(serial_fd < 0){
		debug("Open Serial %s error\n",comdesc[NAME]);
		return -2;
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
//		0);
	}	
	return serial_fd;
}

/* for GUI api  */
int OSGUI__bar_open(int *fd_serial)
{
	int fd = -1;
	int ctrlbits;

	if ((fd = serial_open(scanCom)) < 0){
		debug("OPEN SERIAL ERROR");
		return -1;
	}

	ioctl(fd, TIOCMGET, &ctrlbits);
  	ctrlbits &= ~TIOCM_RTS; 
  	ioctl(fd, TIOCMSET, &ctrlbits); 
	
	*fd_serial = fd;

	return 0;
}

void OSGUI__bar_close(int fd_serial)
{
	/*close serial */
	close(fd_serial);
}


/* @@timeout: ms */
int OSGUI__bar_read(int fd, char *buf, int len)
{
	int retval;
	fd_set fset_read;	
	struct timeval tm_out;
	int nread=0;
	int n=0;
	int count=0;

	memset(buf,0,len);

	FD_ZERO(&fset_read);
	FD_SET(fd,&fset_read);

	tm_out.tv_sec = 0;
	tm_out.tv_usec = 100*1000;

	do {
		retval = select(fd+1,&fset_read,NULL,NULL,&tm_out);

		if (retval < 0){
			debug("SELECT ERROR");
			return -1;
		}

		if( retval == 0){
			count++;
		}

		if(retval > 0 && (FD_ISSET(fd,&fset_read))){
			n = read(fd, buf+nread, len-nread);
			nread += n;
			tm_out.tv_sec = 0;
			tm_out.tv_usec = 100*1000;
			count = 0;
		}
	}while(count < 200);
	return nread;			
}

/* end for GUI api */
int main(void)
{
		int comfd;
		char inbuf[256];
		OSGUI__bar_open(&comfd);
		
		while(1)
		{
			write(comfd,"AT\r", sizeof("AT\r"));
			memset(inbuf,0,sizeof(inbuf));
			if(read(comfd,inbuf,sizeof(inbuf)))
			{
				printf("read:[%s] \n",inbuf);
				//		fflush(stdout);
			}
			else
			{
				printf("read err\n");
				//return 0;

			}
		
//			usleep(100000);
			printf("1\n");
			write(comfd,"AT+CGREG?\r", sizeof("AT+CGREG?\r"));  //3
			printf("2\n");
			memset(inbuf,0,sizeof(inbuf));
			printf("3\n");
			read(comfd,inbuf,sizeof(inbuf));
printf("4\n");
			printf("\n[AT+CGREG? %s] \n",inbuf);

			usleep(100000);

			write(comfd,"AT+UBANDSEL=900;&W\r", sizeof("AT+UBANDSEL=900;&W\r")); 
			memset(inbuf,0,sizeof(inbuf));
			read(comfd,inbuf,sizeof(inbuf));
			printf("\n[AT+UBANDSEL=900 %s] \n",inbuf);
			
			usleep(100000);

			write(comfd,"AT+IFC=0,0\r", sizeof("AT+IFC=0,0\r"));  //关流控1/3
			memset(inbuf,0,sizeof(inbuf));
			read(comfd,inbuf,sizeof(inbuf));
			printf("\n[AT+IFC %s] \n",inbuf);  

			usleep(100000);

			write(comfd,"AT+CFUN=16\r", sizeof("AT+CFUN=16\r"));  
			memset(inbuf,0,sizeof(inbuf));
			read(comfd,inbuf,sizeof(inbuf));
			printf("\n[AT+CFUN=16 %s] \n",inbuf);   

			usleep(100000);
			
			write(comfd,"AT+CSQ\r", sizeof("AT+CSQ\r"));  
			memset(inbuf,0,sizeof(inbuf));
			read(comfd,inbuf,sizeof(inbuf));
			printf("\n[AT+CSQ %s] \n",inbuf);
		}
		

		OSGUI__bar_close(comfd);
		return 0;
}
