/********************************************************
* @author 刘一峰
* 
* @date 20121114 
*********************************************************/
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
#include <sys/types.h>
#include <sys/param.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <errno.h>
#include <termios.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>

 
#define MODULE_POWER_RESET      0
#define MODULE_AT_PROBLEM		1
#define MODULE_SIM_READY		2
#define MODULE_CHECK_SIGNAL		3
#define MODULE_START_PPP        4
#define MODULE_PPP_RUNNING      5


#define GSM_IOC_MAGIC       'G'
#define GSM_SOFTONOFF_ON    _IOW(GSM_IOC_MAGIC,3, int)
#define GSM_SOFTONOFF_OFF   _IOW(GSM_IOC_MAGIC,4, int)

#define GSM_RST_ON          _IOW(GSM_IOC_MAGIC,5, int)
#define GSM_RST_OFF         _IOW(GSM_IOC_MAGIC,6, int)

#define GSM_CTS_ON          _IOW(GSM_IOC_MAGIC,11, int)
#define GSM_CTS_OFF         _IOW(GSM_IOC_MAGIC,12, int)

#define GSM_RTS_ON          _IOW(GSM_IOC_MAGIC,13, int)
#define GSM_RTS_OFF         _IOW(GSM_IOC_MAGIC,14, int)

#define GSM_GET_CTS         _IOR(GSM_IOC_MAGIC,15, int)
#define GSM_GET_RTS         _IOR(GSM_IOC_MAGIC,16, int)

#define GSM_IOC_MAXNR       16




//共享内存部分，拨号进程通过共享内存给状态栏发送拨号状态
#define SHMNAME "shm_dial"
#define OPEN_FLAG O_RDWR|O_CREAT
#define OPEN_MODE 00777
#define FILE_SIZE 10

static const char * scanCom = "/dev/ttymxc1,115200,N,8,0"; //关闭流控
static const char * gsmCmd = "/usr/sbin/pppd call gprs &";
static const char * gsmCmd_ndbg = "/usr/sbin/pppd call gprs > /dev/null 2>&1 &";
static const char * gsmOff = "/etc/ppp/ppp-off &";
static const char * gsmOff_ndbg="/etc/ppp/ppp-off > /dev/null 2>&1 &";
static const char * gsmExit = "pkill pppd &";
static const char * gsmExit_ndbg="pkill pppd > /dev/null 2>&1 &";

//拨号状态
static int gprs_module_state;
//模块复位状态
static int module_power_reset;
//指向映射共享内存的区域
void *add_w=NULL;

int DIAL_DEBUG=1;

static char *g_module_device;

typedef enum{NAME=0,SPEED,PARITY,DATA,STOP,FLOWCTL,MAX}comdesc_t;

static int speed_arr[] = { B460800,B230400, B115200,B38400, B19200, B9600, B4800, B2400, B1200, B300,
					B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static int name_arr[] = {460800,230400, 115200,38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
					19200,  9600, 4800, 2400, 1200,  300, };

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

void gsmIO(int flg)
{
	int fd;
	fd =open("/dev/gprs",O_RDWR);
	
	if(fd==-1)
		printf("open device err\n");
		
	ioctl(fd,flg);
	
	close(fd);
}


static void _sleep(__time_t sec,__suseconds_t us)
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = us;
    select(0, NULL, NULL, NULL, &tv);
}


void _delay (unsigned int _delay)
{
    unsigned int t=_delay*10000;
    _sleep(t/1000000,t%1000000);
}


void _gsmopt_handup()
{
	if (DIAL_DEBUG)
		system(gsmOff);
	else
		system(gsmOff_ndbg);	
}

/*
检查pppd是否已经开启，开启返回1
*/
int _gsmopt_checkpppd()
{
	FILE *file;
	char _pid[20];
  	int ret=0;
  	
  	memset(_pid,0,sizeof(_pid));
	file=popen("pgrep pppd","r");
	
	if (file != NULL)
	{
		if (fgets(_pid, 20, file) != NULL)
			ret=1;
		else
			ret=0;	
	}

	fclose(file);
	
	return ret;
}

void _gsmopt_connect()
{
	
	if (DIAL_DEBUG)
	{
		if(_gsmopt_checkpppd()==0)
		{
			system("rm /var/run/pppd2.tdb > /dev/null 2>&1 &");
    		system(gsmCmd);
    	}
    }
    else
    {
		if(_gsmopt_checkpppd()==0)
		{
			system("rm /var/run/pppd2.tdb > /dev/null 2>&1 &");
    		system(gsmCmd_ndbg);
    	}
    }	
}



void _gsmopt_killpppd()
{
	FILE *file;
  	char _pid[20];
  	char _cmd[50];
  	

	memset(_pid,0,sizeof(_pid));
	memset(_cmd,0,sizeof(_cmd));
	file=popen("pgrep pppd","r");
	
	if (file != NULL) {
		if (fgets(_pid, 20, file) != NULL) {			
			sprintf(_cmd,"kill -9 %s > /dev/null 2>&1 &",_pid);
			_debug("kill pppd %s\n",_pid);
			system(_cmd);
		}
	}
	
	fclose(file);
	
}


void _gsmopt_exitppp()
{
	if (DIAL_DEBUG)
    	system(gsmExit);
    else
    	system(gsmExit_ndbg);	
}

void _gsm_rts(int _opt)
{
	if(_opt==0)
		gsmIO(GSM_RTS_OFF);
	else
		gsmIO(GSM_RTS_ON);
}

void _gsm_cts(int _opt)
{
	if(_opt==0)
		gsmIO(GSM_CTS_OFF);
	else
		gsmIO(GSM_CTS_ON);
}

void _gsm_reset()
{
	gsmIO(GSM_RST_ON);
    _delay(100);      
    gsmIO(GSM_RST_OFF);
	_debug("GSM RESET\n");
}

void _gsm_offon()
{
	gsmIO(GSM_SOFTONOFF_OFF);
    _delay(100);      
    gsmIO(GSM_SOFTONOFF_ON);
	_debug("GSM ONOFF\n");
}



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


int _checkDial(unsigned short uiTimeout)
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
          _delay(1);
          Timeout --;
  
      } while (Timeout != 0);
      return -1;
    }
   
    return 0;
}

int _reg_detect(unsigned char *pucInData)
{
  unsigned char * pucPtr,*pucOffset;
  unsigned char ucMode,ucStat;
  pucPtr = (unsigned char *)strstr( (char *)pucInData,"+CGREG:");
  if( pucPtr )
    {
        pucOffset = (unsigned char *)strchr( (char *)pucPtr,',');
        if( pucOffset )
        {
            ucMode =  *(pucOffset-1);
            ucStat =  *(pucOffset+1);
            if((ucStat == '1')||(ucStat == '5'))
              return 0;
            else
              return -1;
         }
         else
          return -1;
     }
     else
      return -1;

  return -1;
}

//创建共享内存
int _shm_create()
{
	int fd,ret;
	fd = shm_open(SHMNAME, OPEN_FLAG, OPEN_MODE);
	if(fd<0)
		return -1;
	
	//调整确定文件共享内存的空间
	ret = ftruncate(fd, FILE_SIZE);
	if(ret<0)
	{
		//删除共享内存
		shm_unlink(SHMNAME);
		return -2;
	}

	//映射目标文件的存储区
    add_w = mmap(NULL, FILE_SIZE, PROT_WRITE, MAP_SHARED, fd, SEEK_SET);
	if(NULL == add_w)
	{	
		//取消映射并删除共享内存
		munmap(add_w, FILE_SIZE);
		shm_unlink(SHMNAME);
		return -3;
	}

	return 0;	
}

/*
int _acitve
0		写信号
1		拨号状态
char _flg
往共享内存里写状态，主要是信号量
0		没放sim卡
1		没信号
2-5		总共4格信号
*/
void _shm_setSignalState(int _active,char _flg)
{
	if(_active)
		memcpy(add_w+1, &_flg, 1);
	else
		memcpy(add_w, &_flg, 1);
}


//销毁共享内存，在退出或者pkill时候做
void _shm_destory()
{
	munmap(add_w, FILE_SIZE);
//	shm_unlink(SHMNAME);
}

void dial_exit()
{
  	_debug("DIAL EXIST\n");
  	_gsmopt_exitppp();      	
	_shm_setSignalState(1,0);
	_shm_setSignalState(0,9);
	_shm_destory();
	exit(0);
}

void dial_noexit()
{
	_debug("ILLEGAL OPERATION\nDIAL RECEIVE INTERRUPT CALL\n");
}


void _gsm_existSignal()
{

	signal(SIGINT, dial_exit);
	signal(SIGTERM, dial_exit);
}


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

unsigned long asc_long(unsigned char *Pts, unsigned char Ls)
{
    unsigned char I;
    unsigned long Lg1,
    Lg2;

    Lg1 = 0 ;
    Lg2 = 1 ;
    Pts += Ls ;
    for (I = 0; I< Ls ; I++)
    {
        Lg1 += (Lg2 * (*--Pts & 0x0F) ) ;
        Lg2 *= 10 ;
    }

    return (Lg1) ;
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

    fd = uart_open((int *)g_module_device);
    if (fd < 0)return -1;

    _debug("Cmd : %s\n",cmd);

    len = strlen(cmd);
    strcpy(buffer, cmd);
    buffer[len++] = 0xD;
    
    _gsm_rts(0);
    uart_send(fd,buffer,len);
    _delay(10);
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


int _signalDetect(void)
{
        int	detect_result = 0;
        char 	cmd[200]="AT+CSQ\r";
        char 	rsp[200];
        int signal_value=0;
        int rlen, pos,l; 
        char *ptrb;
        char *ptre;
         
        memset(rsp, 0, sizeof(rsp));
        rlen = _AT_cmd(cmd,rsp,sizeof(rsp),1);

        if (rlen <= 0)
            return 0; 
         
        ptrb=strpbrk(rsp,":");
        if(ptrb)
        {
           ptre=strpbrk(rsp,",");
           if(ptre)
           {
             l=ptre-ptrb;
             if(l>0)
             {
              ptrb+=2;
              signal_value=asc_long(ptrb,l-2); 
              if(signal_value>31)
                signal_value=99;
             } 
          }
        }
        else
           return 0; 
         
        if(signal_value<=3)
             detect_result = 1;
        else if((signal_value > 3) && (signal_value <= 15))     
             detect_result = 2;
        else if((signal_value > 15) && (signal_value <= 27))     
             detect_result = 3;
         else if((signal_value >= 28) && (signal_value <= 31))     
             detect_result = 4;
        else
             detect_result = 0;           
           
        return detect_result;
}

int main(int argc, char *argv[])
{
    int _already_online=0;
    int _cnt_offline=0;
    int _cnt_hwr=0;
	int _cnt_at=0;
	int count=0;
    char rbuf[256];
    int  fd;
  	int _s=0,_s_c=0; 
  	long _nosim=0;

	if ((argc != 2) || strncmp(argv[1] , "dbg=",4))
	{
        printf("Usage %s dbg=[0][1] \n",argv[0]);
        return -1;
	}

    if( argv[1][4] >= '0' && argv[1][4] <= '1') {
        DIAL_DEBUG = argv[1][4] - 0x30;
    }

    //初始化模块状态
    gprs_module_state = MODULE_POWER_RESET;
    module_power_reset = 0; 


	//上来就做个复位，防止程序异常关闭时候，串口不通
	_gsm_offon();
    g_module_device = (char *)scanCom;
    _gsmopt_exitppp();
	_debug("CHECKING %s\n",g_module_device);
	//检测两次串口，串口不通，就直接关程序
	while(1)
	{
	    fd = uart_open((int *)g_module_device);
	    if (fd < 0)
	    {count++;_delay(200);}
	    else 
	    {count=0;break;}
	    
	    if(count>1)
	    {
	    	_debug("OPEN SERIAL COM FAILED\n");
	      	exit(0);
	    }  
	}
    uart_close(fd);

	_gsm_existSignal();
   
   	fd=_shm_create();

	_debug("******MODULE STATE*******\n");
	_debug("0 POWER_RESET\n");
	_debug("1 AT_PROBLEM\n");
	_debug("2 SIM_READY\n");
	_debug("3 CHECK_SIGNAL\n");
	_debug("4 START_PPP\n");
	_debug("5 PPP_RUNNING\n");
	_debug("*************************\n");
	
	_AT_cmd("AT+UPSV=1\r",rbuf,sizeof(rbuf),1);  
	_AT_cmd("AT+UBANDSEL?\r",rbuf,sizeof(rbuf),1);	

    while(1) {
        _debug("MODULE STATE [%d]\n",gprs_module_state);

        switch (gprs_module_state) {          
        case MODULE_POWER_RESET:
        	_gsmopt_exitppp();
        	
			if(module_power_reset==1)
			{
				_delay(100);
				_gsm_offon();
				_delay(300);
				_gsm_reset();
				_delay(300);	
			}	
			    
            gprs_module_state = MODULE_AT_PROBLEM;
            break;

        case MODULE_AT_PROBLEM:
			memset(rbuf,0,sizeof(rbuf));
            _AT_cmd("AT",rbuf,sizeof(rbuf),3);			
			if(strstr(rbuf,"OK"))
	        {
	        	_shm_setSignalState(0,1);
				gprs_module_state = MODULE_SIM_READY;
				_cnt_at=0;
			}
	        else
	        {
	        	//AT不通 3次reset模块，5次关闭进程
				if(_cnt_at==2)
				{
					module_power_reset=1;					
				}
				else if(_cnt_at>=4)
				{
					_debug("PLS CHECK HW\n");
					exit(-1);
				}
				

				_debug("AT PROBLEM[%d]\n",_cnt_at+1);
				_cnt_at++;
	            gprs_module_state = MODULE_POWER_RESET;
			}
            break;
		case MODULE_SIM_READY:			
			while(gprs_module_state != MODULE_CHECK_SIGNAL)
			{
				_shm_setSignalState(0,0);
				memset(rbuf,0,sizeof(rbuf));
            	_AT_cmd("AT+CPIN?\r",rbuf,sizeof(rbuf),3);
				if(strstr(rbuf,"READY"))
					gprs_module_state = MODULE_CHECK_SIGNAL;
				else
				{
					_debug("SIM CARD NOT EXIST\n");
					
					//重置模块的时候会检测不到卡一小段时间
					if(_nosim<5)
						_delay(100);
					else if(_nosim>=1000)
					{
						_debug("PLS INPUT SIM CARD\n");
						exit(0);
					}
					else
						_delay(1000);
					_nosim++;	
						
				}
			}
			break;
        case MODULE_CHECK_SIGNAL:
        	if(_gsmopt_checkpppd()==1)
        	{
        		gprs_module_state=MODULE_START_PPP; 
        		break;
        	}
        	
			_s=_signalDetect();
			if(_s==-1)
				_shm_setSignalState(0,1);
			else		
				_shm_setSignalState(0,_s+1);	
			//没信号
			if(_s==0)
			{
				_s_c++;	//检查信号次数
				_debug("PLS CHECK SIGNAL[%d]\n",_s_c);
				
				if(_s_c<30)
				{
					gprs_module_state=MODULE_CHECK_SIGNAL;  
					_delay(100);
				}
				else
				{
					module_power_reset=1;
					gprs_module_state=MODULE_POWER_RESET;
					_s_c=0;				
				}	
			}	  
           	else 
			{
				_s_c=0;
				
				while(1)
				{
					count++;
					memset(rbuf,0,sizeof(rbuf));
		  	        _debug("-%d-",count);
					_AT_cmd("AT+CGREG?\r",rbuf,sizeof(rbuf),2);  
		           
		           
		           	if(count<15)
		           	{
		           		if(!_reg_detect(rbuf))
		            	{
		            		gprs_module_state = MODULE_START_PPP;
							break;
		              	} 
		              	else
		              		_delay(200);	
	          		}
	          		else
	          		{
		          		gprs_module_state=MODULE_POWER_RESET;  
		              	module_power_reset = 1;						
						break;	
	          		}
	          			
	        	}
	        	count=0; 	
	              	
            }
			break;

        case MODULE_START_PPP:
        	_already_online=0;
			_gsmopt_connect();   
            gprs_module_state = MODULE_PPP_RUNNING;
            break;

        case MODULE_PPP_RUNNING:
            if (!_checkDial(1)) {
            	_shm_setSignalState(1,1); 
				_debug(" online..\n");
				_already_online=1;
				_cnt_offline=1;
            }else{
				_shm_setSignalState(1,0);
	            _debug(" offline[%d]..\n",_cnt_offline);
				
				//掉线立马拨号		
				if(_already_online==1)
				{			
					//_gsmopt_handup();
					gprs_module_state = MODULE_CHECK_SIGNAL;
             	}
             	//正好在pppd的时候，休眠了，休眠的级别调得比dial高，还没获取到连接状态的时候已经关闭了
             	if(_cnt_offline>15)            	
             	{         	
             		_cnt_offline=1;           
             		gprs_module_state = MODULE_CHECK_SIGNAL;
             	}
             	
             	
             	_cnt_offline++;
            }
			_delay(300);
            break;
            }//switch
    }//while
    
}
