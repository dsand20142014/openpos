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

int manager(void *p) {}

#define GPS_WAIT_TIMEOUT 1  // 1S
#define GPS_SEARCH_MAXTIMES 10     
#define GPS_PHRASE_GPRMC  "$GPRMC,"
#define GPS_PHRASE_GPGSV  "$GPGSV,"
#define GPS_DEVICE_NAME   "/dev/gps"
#define GPS_CMD_WAKE   "\xB5\x62\x0A\x04\x00\x00\x0E\x34"  // B5 62 0A 04 00 00 0E 34

#define GPS_POWERON    0
#define GPS_POWEROFF   1
#define GPS_WAKEUP     2
#define GPS_WAKEDOWN   3

static const char * gpsCom = "/dev/ttymxc1,9600,N,8,1,0";

typedef enum{NAME=0,SPEED,PARITY,DATA,STOP,FLOWCTL,MAX}comdesc_t;
typedef enum{TIME=0,STATUS,LATITUDE,NS,LONGITUDE,EW,SPD,COG,MV,MVE,MODE,CS,GPRMCMAX}gprmc_t;   
typedef enum{NOMSG=0,MSGNO,NOSV,GPGSVMAX}gpgsv_t;



#define GPS_DEBUG
#ifdef GPS_DEBUG
#define debug(format, arg...) do{printf("" format "" , ## arg); } while (0) //do{printf("%s: %d " format "\n" , __FUNCTION__ ,__LINE__, ## arg); } while (0)
#else
#define debug(format,arg...) 
#endif

static int speed_arr[] = { B460800,B230400, B115200,B38400, B19200, B9600, B4800, B2400, B1200, B300,
					B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static int name_arr[] = {460800,230400, 115200,38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
					19200,  9600, 4800, 2400, 1200,  300, };



static void gps_set_Speed(int fd, int speed){
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

static int gps_set_Parity(int fd,int databits,int stopbits,char parity,int flowctl)
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



static int gps_serial_open(const char * szComDesc)
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
		gps_set_Speed(serial_fd,atoi(comdesc[SPEED]));
		gps_set_Parity(serial_fd,atoi(comdesc[DATA]),\
			atoi(comdesc[STOP]),*comdesc[PARITY],\
			atoi(comdesc[FLOWCTL]));
	}	
	return serial_fd;
}


int gps_open(int *fd_serial)
{
	int fd = -1;
	int ctrlbits;

	if ((fd = gps_serial_open(gpsCom)) < 0){
		debug("OPEN SERIAL ERROR");
		return -1;
	}

	ioctl(fd, TIOCMGET, &ctrlbits);
  	ctrlbits &= ~TIOCM_RTS; 
  	ioctl(fd, TIOCMSET, &ctrlbits); 
	
	*fd_serial = fd;

	return 0;
}

void gps_close(int fd_serial)
{
	
	close(fd_serial);
}

unsigned char gps_poweron(void)
{
	int fd, result;
	
	fd = open(GPS_DEVICE_NAME, O_RDWR);

	if (fd == -1)
        {
            	debug("open device %s error\n",GPS_DEVICE_NAME);
        }

	result = ioctl(fd, GPS_POWERON);
	if(result != 0)
	  debug("gps_poweron  ioctl failed!!!\n");

	close(fd);
	return 0;

}


unsigned char gps_poweroff(void)
{
	int fd, result;
	
	fd = open(GPS_DEVICE_NAME, O_RDWR);

	if (fd == -1)
  {
      debug("open device %s error\n",GPS_DEVICE_NAME);
  }

	result = ioctl(fd, GPS_POWEROFF);
	if(result != 0)
	  debug("gps_poweroff  ioctl failed!!!\n");

	close(fd);
	return 0;

}

unsigned char gps_wakeup(void)
{
	int fd, result;
	
	fd = open(GPS_DEVICE_NAME, O_RDWR);

	if (fd == -1)
  {
      debug("open device %s error\n",GPS_DEVICE_NAME);
  }

	result = ioctl(fd, GPS_WAKEUP);
	if(result != 0)
	  debug("gps_wakeup  ioctl failed!!!\n");

	close(fd);
	return 0;

}

unsigned char gps_wakedown(void)
{
	int fd, result;
	
	fd = open(GPS_DEVICE_NAME, O_RDWR);

	if (fd == -1)
  {
      debug("open device %s error\n",GPS_DEVICE_NAME);
  }

	result = ioctl(fd, GPS_WAKEDOWN);
	if(result != 0)
	  debug("gps_wakedown  ioctl failed!!!\n");

	close(fd);
	return 0;

}



unsigned char gps_sendcmd_wake(void)
{
	 int comfd;
	 char inbuf[1024];
	 unsigned char i;
	 
	 gps_poweron();
	 if(gps_open(&comfd) == -1)
   {
  	   debug("gps  open err\n");
  	   return 1;
   }
   usleep(350000);  //微秒
   if(gps_read(comfd,inbuf,sizeof(inbuf)))
   { 
  	   debug("read:[%s] \n",inbuf);  	   
   }else
   {
  	   debug("read err\n");
   } 
	 write(comfd,GPS_CMD_WAKE,8 );   // strlen(GPS_CMD_WAKE)
	 //usleep(100000);  //微秒
   printf("\n******************************\n");
	 
	 if(gps_read(comfd,inbuf,sizeof(inbuf)))
   { 
  	   for(i=0;i< 20;i++)
  	       printf("%02x ",inbuf[i]);
  	     printf("\n******************************\n"); 
   }else
   {
  	   debug("read err\n");
   } 
	 
	 gps_poweroff();
	 return 0;
}





int gps_read(int fd, char *buf, int len)
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

	tm_out.tv_sec = GPS_WAIT_TIMEOUT;           // 秒
	tm_out.tv_usec =  0;//100*1000;   // 微秒
 
	do {
		    retval = select(fd+1,&fset_read,NULL,NULL,&tm_out);
				if (retval < 0)
				{
					debug("SELECT ERROR");
					return -1;
				}
				if( retval == 0)
				{
					count++;
				}
				if(retval > 0 && (FD_ISSET(fd,&fset_read)))
				{
					n = read(fd, buf+nread, len-nread);
					nread += n;
					tm_out.tv_sec = 0;           // 秒
	        tm_out.tv_usec =  100*1000;   // 微秒
					count = 0;
				}
	}while(count < 200);
	return nread;		
	

}




int gps_info_deal(char *inbuf,unsigned char *satellitesnum,unsigned char *Latitude,unsigned char *longitude)
{
	  char * gprmc[GPRMCMAX];
	  char * gpgsv[GPGSVMAX];
	  unsigned char *pt;
	  unsigned int i,j;
	  char buf[1024];
	  
	  if(!inbuf)
	  {
	  	  debug("$gpsinfo null\n");
	  	  return -1; 
	  }	  
	  pt = strstr(inbuf,GPS_PHRASE_GPRMC); //  TIME=0,STATUS,LATITUDE,NS,LONGITUDE,EW,SPD,COG,MV,MVE,MODE,CS,GPRMCMAX
	  if(!pt)
	  {
	  	  debug("$GPRMC err\n");
	  	  return -2;
	  }  //   $GPRMC,,V,,,,,,,,,,N*53  
	  pt+= strlen(GPS_PHRASE_GPRMC);
	  memset(buf,0,sizeof(buf));
	  i = 0;
	  buf[i++] = ' ';
	  
	  while((*pt))
	  {
	  	 if(((*pt) == ',')&&((*(pt+1)) == ','))
	  	 {
	  	 	   buf[i++] = ',';
	  	 	   buf[i++] = ' ';  	 	 
	  	 	   pt++;
	  	 }else
	  	 {
	  	 	   buf[i++] = *pt;
	  	 	   pt++;
	  	 }
	  }
	  debug("\n*********%s",buf);
	  
	  for(i = TIME;i < GPRMCMAX;i++)
		 gprmc[i] = NULL;
		 
		
	  gprmc[TIME] = strtok(buf,",");
	  
	  for(i = STATUS;i < GPRMCMAX;i++)
	  {
				gprmc[i] = strtok(NULL,",");
				if(!gprmc[i])
					break;
	  }
	  
	  for(j=0;j<i;j++)  
	  {
	  	  debug("\n j=%d|    %s",j,gprmc[j]); 
	  }
	  
		if(i < GPRMCMAX)
		{
			debug("GPRMC format error\n");
			return -3;
		}  
		
		debug("\n222*********%s",inbuf);
		
		if(*gprmc[STATUS] == 'V')
		{
			   debug("STATUS error\n");
		    	return -6;  
		}    
		strcpy(Latitude,gprmc[NS]);
		strcat(Latitude,gprmc[LATITUDE]);
		strcpy(longitude,gprmc[EW]);
		strcat(longitude,gprmc[LONGITUDE]);
		
		pt = strstr(inbuf,GPS_PHRASE_GPGSV); //  NOMSG=0,MSGNO,NOSV,GPGSVMAX
	  if(!pt)
	  {
	  	  debug("$GPGSV err\n");
	  	  return -4;
	  }  //   $GPGSV,1,1,00*79  
	  pt+= strlen(GPS_PHRASE_GPGSV);
	  memset(buf,0,sizeof(buf));
	  i = 0;
	  buf[i++] = ' ';
  
	  while((*pt))
	  {
	  	 if(((*pt) == ',')&&((*(pt+1)) == ','))
	  	 {
	  	 	   buf[i++] = ',';
	  	 	   buf[i++] = ' ';  	 	 
	  	 	   pt++;
	  	 }else
	  	 {
	  	 	   buf[i++] = *pt;
	  	 	   pt++;
	  	 }
	  }
	  debug("\n111*********%s",buf);
	  for(i = NOMSG;i < GPGSVMAX;i++)
		 gpgsv[i] = NULL;
	  gpgsv[NOMSG] = strtok(buf,",");
	  for(i = MSGNO;i < GPGSVMAX;i++)
	  {
				gpgsv[i] = strtok(NULL,",");
				if(!gpgsv[i])
					break;
	  }
	  for(j=0;j<i;j++)  
	  {
	  	  debug("\n i=%d,j=%d>    %s",i,j,gpgsv[j]); 
	  }
		if(i < GPGSVMAX)
		{
			debug("GPGSV format error\n");
			return -5;
		}
		pt = strchr(gpgsv[NOSV],'*');
		if(pt) 
		{
			 *pt = 0;
			 strcpy(satellitesnum,gpgsv[NOSV]);
		}
		else 
		   strcpy(satellitesnum,gpgsv[NOSV]);
		
	  return 0;
}


unsigned char gps_get_gpsinfo(unsigned char *satellitesnum,unsigned char *Latitude,unsigned char *longitude,unsigned int timeout) // second 
{
	  char inbuf[1024];
	  int comfd;
	  unsigned char i;
	  unsigned int times;
	  
	/*  
	   memset(inbuf,0,sizeof(inbuf));
	   strcpy(inbuf,"$GPRMC,,V,11.1234,N,123456.890,W,,,,,,N*53 $GPGSV,1,1,2*79");
	   debug("read:[%s] \n",inbuf);
	   if(gps_info_deal(inbuf,satellitesnum,Latitude,longitude) == 0)
	   {
	   	 
	   	  debug("\nsatellitesnum=%s",satellitesnum);
	   	  debug("\nLatitude=%s",Latitude);
	   	  debug("\nlongitude=%s",longitude);
	   	  return 0;
	   }else
	   {
	   	   debug("\nfuck you!");
	   	  return 0;
	   }			  	  
	  */
	  gps_poweron();
	 // gps_wakeup();
	 gps_wakedown();
	 // gps_poweroff();
	  
	  if(gps_open(&comfd) == -1)
	  {
	  	   debug("gps  open err\n");
	  	   return 1;
	  }
	  if(timeout < GPS_SEARCH_MAXTIMES)
	  	  times = GPS_SEARCH_MAXTIMES;
	  else
	  	  times = timeout;
	  for(i=0;i<times;i++)
	  {
	  	  memset(inbuf,0,sizeof(inbuf));
			  if(gps_read(comfd,inbuf,sizeof(inbuf)))
			  {
			  	   debug("read:[%s] \n",inbuf);
			  	   if(gps_info_deal(inbuf,satellitesnum,Latitude,longitude) == 0)
			  	   {
			  	   	  gps_close(comfd);
			  	   	  debug("\nsatellitesnum=%s",satellitesnum);
			  	   	  debug("\nLatitude=%s",Latitude);
			  	   	  debug("\nlongitude=%s",longitude);
			  	   	  gps_poweroff();
			  	   	  return 0;
			  	   }			  	   
			  }else
			  {
			  	   debug("read err\n");
			  } 
	  }
	  debug("\n try times is over i=%d\n",i);
	  gps_close(comfd);
	  
	 // gps_poweroff();
	 
	  return 1;	  
}




int main(void)
{
		unsigned char buf1[10],buf2[50],buf3[50],inbuf[1024];
		int comfd;
		
		
		//gps_sendcmd_wake();
		
		memset(buf1,0,sizeof(buf1));
		memset(buf2,0,sizeof(buf2));
		memset(buf3,0,sizeof(buf3));
		
		gps_get_gpsinfo(buf1,buf2,buf3,1);
		
		
		return 0;
}


