#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/param.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <errno.h>
#include <termios.h>
#include <sys/msg.h>
#include "gsm.h"
#include "sysparam.h"

#define GSM_IOC_MAGIC       'G'
#define GSM_RTS_ON          _IOW(GSM_IOC_MAGIC,13, int)
#define GSM_RTS_OFF         _IOW(GSM_IOC_MAGIC,14, int)

#define TMPFILE			"/tmp/tmpfile"
#define GDIR			"/etc/ppp"

int DIAL_DEBUG=1;

static char *g_module_device;
static const char * scanCom = "/dev/ttymxc1,115200,N,8,0"; //关闭流控

typedef enum{NAME=0,SPEED,PARITY,DATA,STOP,FLOWCTL,MAX}comdesc_t;

static int speed_arr[] = { B460800,B230400, B115200,B38400, B19200, B9600, B4800, B2400, B1200, B300,
					B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static int name_arr[] = {460800,230400, 115200,38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
					19200,  9600, 4800, 2400, 1200,  300, };


unsigned int  _ppp0_t(void)
{
    unsigned int fd, intrface, retn = 0,i = 1;
    char *ppp0_name= "ppp0";
    struct ifreq buf[16];
    struct ifconf ifc;

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) 
    {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        
        if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) 
        {
          intrface = ifc.ifc_len / sizeof (struct ifreq);
          while (intrface-- > 0)
          {
            if (strcmp(ppp0_name,buf[intrface].ifr_name)==0) 
            {        
              i=0;
              break;
            }
          }       
        }  
    } 
    close (fd);
    return i;
}

void gsmIO(int flg)
{
	int fd;
	fd =open("/dev/gprs",O_RDWR);
	
	if(fd==-1)
		printf("open device err\n");
		
	ioctl(fd,flg);
	
	close(fd);
}


void _gsm_rts(int _opt)
{
	if(_opt==0)
		gsmIO(GSM_RTS_OFF);
	else
		gsmIO(GSM_RTS_ON);
}


int GSM__dial_check(unsigned short uiTimeout)
{
    unsigned short Timeout=uiTimeout;
    unsigned int  result;
    
    result = _ppp0_t();
    if (result != 0) 
    {
      do
      {
          result = _ppp0_t();
          if (result == 0)  return 0;
          Os__xdelay(1);
          Timeout --;
  
      } while (Timeout != 0);
      return -1;
    }
   
    return 0;
}


//调试信息
static void _debug(char *fmt,...)
{
    va_list ap;
    char string[512];

    if (DIAL_DEBUG) {
    	if (strlen(fmt) > 400)
            return;

        va_start(ap,fmt);
        vsprintf(string,fmt,ap);
        fprintf(stderr,string);
        va_end(ap);
    }
}


static void set_Speed(int fd, int speed)
{
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
				printf("tcsetattr fd1\n");  	
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
		_debug("Com:   tcgetattr fail\n");
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
			_debug("Com:   Unsupported data size\n"); 
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
			_debug("Com:   Unsupported parity\n");
			return(-1);
	}

	options.c_cflag &= ~CSTOPB;
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
		_debug("Com:   Setup Serial Port\n");
		return (-1);	
	}
	return(0);
}

int uart_recv_timeout(int fd, char *buf, int len,int second)
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

  tm_out.tv_sec  = second;
  tm_out.tv_usec = 0;

  retval = select(fd+1,&fset_read,NULL,NULL,&tm_out);

   if (retval < 0 )   
       return -1;
   
    if (retval == 0)
        return 0;

    return read(fd,buf,len);

}

static int serial_open(const char * szComDesc)
{
	comdesc_t i;
	int serial_fd;
	char * comdesc[MAX];	
	char desc_buf[64];
	char * ptmp = desc_buf;
	
	if(!szComDesc || strlen(szComDesc)>63){
		_debug("szComDesc is too long\n");
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
		_debug("Format of serial's name is error\n");
		return -3;
	}
	serial_fd = open(comdesc[NAME],O_RDWR);
	if(serial_fd < 0){
		_debug("Open Serial %s error\n",comdesc[NAME]);
		return -2;
	}
	else{
		if(!comdesc[FLOWCTL]){
			char zero = '0';
			comdesc[FLOWCTL] = &zero;
		}
		set_Speed(serial_fd,atoi(comdesc[SPEED]));
		set_Parity(serial_fd,
		atoi(comdesc[DATA]),
			atoi(comdesc[STOP]),
			*comdesc[PARITY],
			atoi(comdesc[FLOWCTL]));
	}	
	return serial_fd;
}

int uart_open(int *fd_serial)
{
	int fd = -1;
	int ctrlbits;


	if ((fd = serial_open((char *)fd_serial)) < 0){
		_debug("OPEN SERIAL ERROR");
		return -1;
	}

	ioctl(fd, TIOCMGET, &ctrlbits);
  	ctrlbits &= ~TIOCM_RTS; 
  	ioctl(fd, TIOCMSET, &ctrlbits); 
	
	return fd;
}

void uart_close(int fd_serial)
{
	close(fd_serial);
}

int uart_send(int fd,unsigned char *buff,int len)
{
    return write(fd, buff, len);
}

static int _AT_cmd(char *cmd,unsigned char *rbuf,int rlen,int timeout)
{
    int  fd;
    int  len;
    char buffer[128];
    int  i,retval;
    
	g_module_device = (char *)scanCom;
    fd = uart_open((int *)g_module_device);
    if (fd < 0)return -1;

    _debug("Cmd : %s\n",cmd);

    len = strlen(cmd);
    strcpy(buffer, cmd);
    buffer[len++] = 0xD;
    
    _gsm_rts(0);
    uart_send(fd,buffer,len);
    Os__xdelay(20);
    _gsm_rts(1);
    retval = uart_recv_timeout(fd,rbuf,rlen,timeout);

   
    if (retval <= 0) 
    {
        if ( retval < 0 )
            _debug("%s:%d  select error \n",__func__,__LINE__);
        else        
            _debug("%s:%d  select time out \n",__func__,__LINE__);
        
        goto exit;
    }

	_debug("[%s]\n",rbuf);

    len = retval;  
    if (len >= 4) { // 'O' 'K' 0D 0A
        if ((rbuf[len-4]=='O'||rbuf[len-4]=='o')&&
            (rbuf[len-3]=='K'||rbuf[len-3]=='k')  ){
            /* OK received */ 
            goto exit;
        }
    }
    _debug("<%s:%d> NO OK \n",__FUNCTION__,__LINE__);
    retval = 0;
exit:
	_gsm_rts(0);
    uart_close(fd);
    return retval;
}


void _setBands(int _bands)
{
	char rbuf[256];
	
	memset(rbuf,0,sizeof(rbuf));
	switch(_bands)
    {
    case 1:
    	_AT_cmd("AT+UBANDSEL=850\r",rbuf,sizeof(rbuf),3);
        break;
    case 2:
    	_AT_cmd("AT+UBANDSEL=900\r",rbuf,sizeof(rbuf),3);
        break;
    case 3:
    	_AT_cmd("AT+UBANDSEL=1800\r",rbuf,sizeof(rbuf),3);
        break;
    case 4:
    	_AT_cmd("AT+UBANDSEL=1900\r",rbuf,sizeof(rbuf),3);
        break;
    case 5:
    	_AT_cmd("AT+UBANDSEL=850,1900\r",rbuf,sizeof(rbuf),3);
       break;
    case 6:
    	_AT_cmd("AT+UBANDSEL=900,1800\r",rbuf,sizeof(rbuf),3);
        break;
    case 7:
    	_AT_cmd("AT+UBANDSEL=900,1900\r",rbuf,sizeof(rbuf),3);
        break;
    }
    
    
}

void GSM__setParam_gprs(int _bands)
{
	char rbuf[256];
	int ret=0;
	const int MAX_SIXE = 10;
	PROCESS_INIT_INFO proInitList[MAX_SIXE];
	int cn = 0, i = 0;
	
	system("pkill dial");
	
	while(1)
    {
    	
    	ret=access("/var/lock/LCK..ttymxc1",F_OK);
    	if(ret==0)
    	{
    		if (!GSM__dial_check(1)) 
    			system("/etc/ppp/ppp-off");    		
    	}
    	else
    		break;  	

    	Os__xdelay(50);
    }
    
    
	while(ret<=0)
	{
   		ret=_AT_cmd("AT\r",rbuf,sizeof(rbuf),3);
   		Os__xdelay(50);
 	}	  
    		
	_setBands(_bands);	
	memset(rbuf,0,sizeof(rbuf));
	_AT_cmd("at&w\r",rbuf,sizeof(rbuf),3);
	memset(rbuf,0,sizeof(rbuf));
	_AT_cmd("at+cfun=16\r",rbuf,sizeof(rbuf),3);
	
	//获取dial版本号和调试标志
	memset(proInitList,0,sizeof(PROCESS_INIT_INFO)*MAX_SIXE);
	cn = PARAM_shell_get(proInitList,MAX_SIXE,DAEMON_SHELL);
	
	memset(rbuf,0,sizeof(rbuf));
	for(i=0;i<cn;i++)
	{
		if(strcmp(proInitList[i].name,"dial.dm") == 0)
		{
			sprintf(rbuf,"/daemon/dial.dm.%s dbg=%d &",proInitList[i].version,proInitList[i].dbg);
			break;
		}
	}
	if(i >= cn)
	{
		sprintf(rbuf,"/daemon/dial.dm.4.1.4 dbg=0 &");
	}
	system(rbuf);

}


void _copy_from_tmpfile(char *file)
{
    char command[100];

    memset(command, 0, sizeof(command));
    sprintf(command, "cp -f %s %s",TMPFILE,file);
    system(command);
}

void GSM_config_pap_chap(char *username, char *password)
{
	char command[256];
    char pap[50],chap[50];
    char *_pap = "pap-secrets";
    char *_chap= "chap-secrets";

    memset(pap,0,sizeof(pap));
    sprintf(pap,"%s/%s",GDIR,_pap);
    memset(chap,0,sizeof(chap));
    sprintf(chap,"%s/%s",GDIR,_chap);


    /* pap */
    memset(command, 0, sizeof(command));
    sprintf(command, "sed -e '$c\\ \"%s\"    *     \"%s\"    *  ' %s>%s ",username, password, pap, TMPFILE);
    system(command);
    _copy_from_tmpfile(pap);

    /* chap */
    memset(command, 0, sizeof(command));
    sprintf(command, "sed -e '$c\\ \"%s\"    *     \"%s\"    *  ' %s>%s ",username, password, chap, TMPFILE);

    system(command);
    _copy_from_tmpfile(chap);
}

int GSM_set_APN(unsigned char* ucAPN)
{

	FILE *fp;
	int i = 0 , j;
	int tem = 0;
	char apn_buf[64];
	char rbuff[1024];
	char wbuff[1024];
	char APN[30];
	char *chat_script_file = "/etc/ppp/chat/gprs-chat-script"; 

	memset(APN,0,sizeof(APN));
	
	strncpy(APN, ucAPN,sizeof(APN)-1);

	fp = fopen(chat_script_file, "r+");

	if(NULL == fp)
	{
		printf("[%s:%d] file open failed",__func__,__LINE__); 
		return -1;
	}

	memset(rbuff, 0x00, sizeof(rbuff));
	memset(wbuff, 0x00, sizeof(wbuff));
	memset(apn_buf, 0x00, sizeof(apn_buf));
	fread(rbuff, 1024, 1, fp);
	while ( i < 1023)
	{
		if (rbuff[i]=='I' &&rbuff[i+1]=='P')
		{
			tem = i;
			for (j = 0; j < 64; j++, tem++)
			{
				if (rbuff[tem+5] == 0x22)
					break;
				apn_buf[j] = rbuff[tem+5];
			}
			break;
		}
		i++;
	}

	if (strcmp(apn_buf,  ucAPN) == 0x00) {
		fclose(fp);
		return 0;
	}

	i += 5;
	tem = i;
	memcpy(wbuff, rbuff, i);
	memcpy(wbuff+i, APN, strlen(APN));

	while (i < 1024) {
		if (rbuff[i]=='"') break;
		i ++;
	}

	memcpy(wbuff+tem+strlen(APN), rbuff+i,strlen(rbuff)-i);

	ftruncate(fileno(fp), 0);
	fseek(fp, 0, SEEK_SET);
	fwrite(wbuff,strlen(wbuff), 1, fp);
	fclose(fp);
	system("sync");

	return 0;

}

int Os__change_APN(unsigned char* ucAPN)
{
	SYS_SETTING setting;
	int ret = 0;
	
	if(ucAPN == NULL || *ucAPN == '\0')
		return -1;
	
	//先判断当前APN是否一样,如果不一样需要更改
	memset(&setting,0,sizeof(SYS_SETTING));
	strcpy(setting.section,"GSM_SETTING");
	strcpy(setting.key,"DEFAULT_APN");
	if(PARAM_setting_get(&setting) >= 0)
	{
		//比较APN
		if(strcmp(setting.value,ucAPN) == 0)
			return 0;
	}
	else
		return -1;
		
	if(GSM_set_APN(ucAPN) == 0)
	{
		memset(&setting,0,sizeof(SYS_SETTING));
		strcpy(setting.section,"GSM_SETTING");
		strcpy(setting.key,"DEFAULT_APN");
		strcpy(setting.value,ucAPN);
		PARAM_setting_set(&setting);
	}
	//关闭拨号服务,dial会自动重启pppd服务
	while(1)
    {
    	ret=access("/var/lock/LCK..ttymxc1",F_OK);
    	if(ret==0)
    	{
    		if (!GSM__dial_check(1)) 
    		{
    			system("/etc/ppp/ppp-off");
    			Os__xdelay(100);
			}    		
    	}
    	else
    		break;   	
    	
    	Os__xdelay(50);
    }
    
    return 0;
}

int GSM_AT_cmd(char *cmd,unsigned char *rbuf,int rlen,int timeout)
{
	return _AT_cmd(cmd,rbuf,rlen,timeout);
}
