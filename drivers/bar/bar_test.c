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
#define PWR_ON          5
#define PWR_OFF         6

#define TRIG_DEV "/dev/bar"
static const char * scanCom = "/dev/ttymxc0,9600,N,8,1,1";

typedef enum{NAME=0,SPEED,PARITY,DATA,STOP,FLOWCTL,MAX}comdesc_t;
#define SCAN_DEBUG
#ifdef SCAN_DEBUG
#define debug(format, arg...) do{printf("%s: %d " format "\n" , __FUNCTION__ ,__LINE__, ## arg); } while (0)
#else
#define debug(format,arg...) 
#endif

static char testcmd[] ={0x04,0xEB,0x04,0x00,0xFF,0x0D};

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
    printf("set serial:databits=%d,stopbits=%d,parity=%c,flowctl=%d\n",databits,stopbits,parity,flowctl);
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
	}	
	return serial_fd;
}

/* for GUI api  */
int OSGUI__bar_open(int *fd_serial, int *fd_tg)
{
	int fd = -1;
	int ctrlbits;

	if ((fd = serial_open(scanCom)) < 0){
		debug("OPEN SERIAL ERROR");
		return -1;
	}

	//ioctl(fd, TIOCMGET, &ctrlbits);
  	//ctrlbits &= ~TIOCM_RTS; 
  	//ioctl(fd, TIOCMSET, &ctrlbits); 
	
	*fd_serial = fd;

	if((fd = open(TRIG_DEV,O_RDONLY)) < 0){
		debug("OPEN DEV TRIGER ERROR");
		return -1;
	}

	*fd_tg = fd;
    ioctl(*fd_tg,PWR_ON);
	return 0;
}

void OSGUI__bar_close(int fd_serial, int fd_tg)
{
	/*close serial */
	close(fd_serial);

	/* close bar device */
	ioctl(fd_tg, HIGH_LEVEL);
    ioctl(fd_tg,PWR_OFF);
	close(fd_tg);
}


/* bar scan */
void OSGUI__bar_scan(int fd)
{
	ioctl(fd, HIGH_LEVEL);
	usleep(200000);
	ioctl(fd, LOW_LEVEL);
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


	FD_ZERO(&fset_read);
	FD_SET(fd,&fset_read);

	tm_out.tv_sec = 0;
	tm_out.tv_usec = 1000*1000;

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
			tm_out.tv_usec = 1000*1000;
			count = 0;
			printf("buf=%s\n",buf);
		}
	}while(count < 200);

	if (nread){
   		int i;
		printf("\n");
		for(i=0;i<strlen(buf);i++)
			printf("[%02x]",buf[i]);
			printf("\n");
	}
    else
    {
        printf("not read anything!\n");
    }
	return nread;			
}

/* end for GUI api */

int main(void)
{
		int comfd,trigfd;
        int ctrlbits;
        int ret;
        char buf[256];
		OSGUI__bar_open(&comfd,&trigfd);
		while(1)
		{
            printf("set bar:\n");

            ioctl(comfd, TIOCMGET, &ctrlbits);
            ctrlbits |= TIOCM_RTS; 
            ioctl(comfd, TIOCMSET, &ctrlbits);

            ret=write(comfd,testcmd,sizeof(testcmd));

            usleep(1000);
            ioctl(comfd, TIOCMGET, &ctrlbits);
            ctrlbits &= ~TIOCM_RTS;  
            ioctl(comfd, TIOCMSET, &ctrlbits);

            if(ret<0)
            {
                perror("write bar:");
            }
            else
            {
                printf("write %d bytes to bar;\n",ret);
            }
            OSGUI__bar_read(comfd,buf,sizeof(buf));

            printf("read bar:\n");
			OSGUI__bar_scan(trigfd);
			if(OSGUI__bar_read(comfd,buf,sizeof(buf)))
			{
				printf("bar read : %s\n",buf);
				fflush(stdout);
			}
			else
			{
				perror("read bar");
			}
			usleep(100000);
		}
		OSGUI__bar_close(comfd,trigfd);
		return 0;
}
