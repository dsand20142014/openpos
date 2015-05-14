//@author 刘一峰

#include <ftw.h>
#include <stdio.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <termios.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <stdarg.h>
#include "param.h"
#include "updateService.h"
//#include "md5.h"


char *D_UART_NAME="/dev/ttymxc4";
char *D_OTG_NAME="/dev/ttyGS0";

#define SERIAL_COM_OPEN  1
#define SERIAL_COM_CLOSE 2

#define DOWNCMD_REQUEST		0x01	//CMD
#define DOWNCMD_RESPFILE	0x02	//文件
#define DOWNCMD_RESPKEY		0x03	//按键，只有网络状态下才有按键
#define DOWNCMD_LOCAL		0x04	//本地文件

// 包协议 14位
// 02+downcmd(1)+data+03+lrc
#define SPD_STX 0x02
#define SPD_ETX 0x03

//文件夹类型
#define REQUEST_FOLDERTYPE_APP			0x31	//应用程序
#define REQUEST_FOLDERTYPE_DRIVER		0x32	//驱动
#define REQUEST_FOLDERTYPE_LIB			0x33	//库
#define REQUEST_FOLDERTYPE_FONT			0x34	//字体
#define REQUEST_FOLDERTYPE_APPMANAGER	0x35	//os
#define REQUEST_FOLDERTYPE_UBOOT		0x36	//uboot
#define REQUEST_FOLDERTYPE_KERNEL		0x37	//kernel

//文件类型
#define REQUEST_FILETYPE_CONFIG			0x31	//系统级别的配置文件
#define	REQUEST_FILETYPE_SETTING		0x32	//应用级别的配置文件
#define REQUEST_FILETYPE_BIN			0x33	//应用程序文件
#define REQUEST_FILETYPE_BINLOGO		0x34	//程序图标
#define REQUEST_FILETYPE_BINIMG			0x35	//程序所用到的图片
#define REQUEST_FILETYPE_DAEMON			0x36	//系统进程文件
#define REQUEST_FILETYPE_SCRIPT			0x37	//应用程序UI脚本文件

#define PROCESS_ADD 	0x30	//新增
#define PROCESS_UPDATE	0x31	//更新
#define PROCESS_COVER	0x32	//覆盖
#define PROCESS_DEL		0x33	//删除
#define PROCESS_DDEAL	0x34	//作为母pos状态，不做操作时用

//文件类型
#define TYPE_DRV		0x31		//驱动	
#define TYPE_LIB		0x32		//库
#define TYPE_FONT		0x33		//字库
#define TYPE_UBOOT		0x34
#define TYPE_KERNEL		0x35		
#define TYPE_DAEMON		0x36

//大类，细分为下面的小类，小类只在处理设备结点的时候有用
#define C_UART	1	//通讯方式为串口
#define C_NW	2	//通讯方式为网络
#define C_OTG	3	//通讯方式为OTG2UART

#define SPD_C_UART		1		//使用串口下装
#define SPD_C_OTG		2		//使用otg口下装
#define SPD_C_WLAN		3		//使用有线网络下装
#define SPD_C_WIFI		4		//使用无线下装


#define SERVER_PORT 	11111		//服务器端口

int needReboot=0;  //1-需要重启 其他-不需要

int  g_tty_fd=-1;			//串口连接句柄
int  g_otg_fd=-1;			//OTG连接句柄
int  client_socket=-1;		//网络连接句柄
char SERIAL_COM[50];		//串口名称 从配置读
char SERVER_IP[20];			//下装器ip，端口固定为11111 从配置读

struct termios g_tty_attr;		//串口属性
struct termios g_tty_attrbk;	//串口属性备份

char CONNECT_S[2];			//连接类型 1-UART 2-OTG2UART 3-WLAN 4-WIFI 5-TF 6-U DISK
char SMT_OPEN[2];			//连接状态 1-打开 0-关闭

static unsigned int etcFile,etcSection,etcSectionSMT;


/*
char UART_EXIST[2];					//是否监控串口的标志 0-不检测 其他-检测
char OTG_EXIST[2];					//是否监控otg转串口的标志 0-不检测 其他-检测
char WLAN_EXIST[2];					//是否监控有线网络的标志 0-不检测 其他-检测
char WIFI_EXIST[2];					//是否监控wifi的标志 0-不检测 其他-检测
char TF_EXIST[2];					//是否监控TF的标志 0-不检测 其他-检测
char USB_EXIST[2];					//是否监控USB的标志 0-不检测 其他-检测
char UPDATE_MODE[2];				//通过TF/USB更新的操作标志,1请求更新,2更新中,3更新失败,4操作成功
*/

int SPD_DATA_FLG=-1;				//符合协议的数据标志 0-符合 其他-不符合
char AUCCMD[130];					//协议

struct timeval tv;
fd_set rfds;
int ret;

//以下是在操作时候给界面送的消息
int msg_id;							//消息队列ID
#define SPD_SMG_KEY		3000	//消息队列key值
#define MSG_TYPE_SERVER 1		
#define MSG_TYPE_CLIENT 2

struct update_msg
{
	unsigned char ucProcess;	//更新进度
	unsigned char ucOption;		//操作类型 2更新中,3更新失败,4更新成功
};

typedef struct
{
	long msgtyp;
	struct update_msg msg_data;
}UP_SER_MSG;

UP_SER_MSG msg;						//消息队列





int OSSPD_nw_send(unsigned char *pucInBuf,unsigned short uiInLen);
void _nwflush(void);
void _Os_spd_OCSwitch(int optFlg);
void OSSPD_package_allocate(void);



int DIAL_DEBUG=1;

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

int _checkdbg(int argc, char *argv[])
{
	
	if ((argc != 2) || strncmp(argv[1] , "dbg=",4))
	{
        printf("Usage %s dbg=[0][1] \n",argv[0]);
        return -1;
	}

    if( argv[1][4] >= '0' && argv[1][4] <= '1') {
        DIAL_DEBUG = argv[1][4] - 0x30;
    }
	
	return 0;
}

//创建消息队列
void _msg_create()
{
	msg_id = msgget(SPD_SMG_KEY, 0777|IPC_CREAT);
	
}


unsigned char  OSSPD_tool_lrc(unsigned char *pointchar,int length)
{
    unsigned char Lrc1;
	int i=0;
    Lrc1 = 0xff;
    while (i++<length) 
		Lrc1 ^= *pointchar++;

    return (Lrc1);
}


unsigned char OSSPD_tool_conv(unsigned char Oct)
{
    if (Oct >= 0x41)
        return ( (Oct - 7) & 0x0F );
    else
        return ( Oct & 0x0F );
} 

/*
判断文件夹存在不存在
*/
int OSSPD_tool_folderExist(unsigned char *pPath)
{
	int iret=0;
	iret=access(pPath,F_OK);
	if(iret== 0)
		return 1;
	return 0;
}

unsigned char *OSSPD_tool_hex2asc(unsigned char *Ptd,unsigned char *Pts,unsigned char Lg )
{
    unsigned char I,
    *Pt0;

    Pt0 = Ptd ;
    if ( Lg % 2 ) *Ptd++ = (*Pts++ & 0x0F) + 0x30 ;
    for ( I = 0 ; I < (Lg / 2) ; I++)
    {
        *Ptd++ = ( (*Pts & 0xF0) >> 4 ) + 0x30 ;
        *Ptd++ = (*Pts++ & 0x0F) + 0x30 ;
    }
    while (Pt0 != Ptd)
    {
        if (*Pt0 >= 0x3A) *Pt0 += 7 ;
        Pt0++;
    }
    return((unsigned char*)Ptd);
}

unsigned char *OSSPD_tool_asc2hex(unsigned char *Ptd ,unsigned char Ld,unsigned char *Pts,unsigned char Ls)
{
    unsigned char I;

    (void) memset( Ptd, 0x00, Ld ) ;

    Ptd = Ptd + Ld - ((Ls + 1) / 2) ;
    if ( Ls % 2 ) *Ptd++ = OSSPD_tool_conv(*Pts++) & 0x0F ;
    for ( I = 0 ; I < (Ls / 2) ; I++)
    {
        *Ptd =  (OSSPD_tool_conv(*Pts++) << 4) & 0xF0 ;
        *Ptd = *Ptd + (OSSPD_tool_conv(*Pts++) & 0x0F)  ;
        Ptd ++;
    }
    return((unsigned char*)Ptd);
}

unsigned long OSSPD_tool_asc2long(unsigned char *Pts,unsigned char Ls)
{
    unsigned char I;
    unsigned long Lg1,Lg2;

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

void OSSPD_tty_mode(int mode, int tty_fd)
{
  switch (mode)
  {
		case SERIAL_COM_OPEN:
			(void)tcgetattr(tty_fd, &g_tty_attrbk);
			cfmakeraw(&g_tty_attr);
			cfsetspeed(&g_tty_attr, B115200);
			tcsetattr(tty_fd, TCSADRAIN, &g_tty_attr);
			break;
		case SERIAL_COM_CLOSE:
			tcsetattr(tty_fd, TCSADRAIN, &g_tty_attrbk);
			break;
	}
}

int OSSPD_tty_open(int _f)
{ 
	int etcFile,etcSection,i;

	//这里从配置读设备结点，考虑到配置文件可以从下装器修改，还是用程序里的宏
	/*
	memset(SERIAL_COM,0,sizeof(SERIAL_COM));
	etcFile=Os__param_load(SPD_CONFIG_FILE);
	etcSection=Os__param_findSection(etcFile,"SERVER_CONFIG",0);
	Os__param_get(etcSection,"SERIAL_COM",SERIAL_COM,sizeof(SERIAL_COM));
	//对读取的串口名称进行处理
	i = 0;
	while(i < sizeof(SERIAL_COM))
	{
		if(SERIAL_COM[i] == ' ')
		{
			SERIAL_COM[i] = '\0';
			break;
		}
		i++;
	}
	*/
		
	if(g_tty_fd<=0 && _f==1)
	{
		if(access(D_UART_NAME,F_OK) != 0)
			return -1;
	
		g_tty_fd = open(D_UART_NAME,O_RDWR|O_NOCTTY);
		
		if(g_tty_fd>0)
			OSSPD_tty_mode(SERIAL_COM_OPEN,g_tty_fd);
		else
			return -1;
	}
	else if(g_otg_fd<=0 && _f==2)
	{
		if(access(D_OTG_NAME,F_OK) != 0)
			return -1;
	
		g_otg_fd = open(D_OTG_NAME,O_RDWR|O_NOCTTY);
	
		if(g_otg_fd>0)
			OSSPD_tty_mode(SERIAL_COM_OPEN,g_otg_fd);
		else
			return -1;

	}
	return 0;
}

void OSSPD_tty_close(void)
{

	if(g_tty_fd > 0)
	{
		OSSPD_tty_mode(SERIAL_COM_CLOSE,g_tty_fd);
		close(g_tty_fd);
		g_tty_fd=-1;
	}

	if(g_otg_fd > 0)
	{
		OSSPD_tty_mode(SERIAL_COM_CLOSE,g_otg_fd);
		close(g_otg_fd);
		g_otg_fd=-1;
	}
}

/*
*检测串口数据
有效数据- 0
other - 1
*/
int OSSPD_tty_checkRead(int tty_fd)
{
  	tv.tv_sec = 0;       
	tv.tv_usec = 200;
	FD_ZERO(&rfds);
	FD_SET(tty_fd,&rfds); 
	ret = select(tty_fd+1,&rfds,NULL,NULL,&tv);
	if(ret>0)
	{
		memset(AUCCMD,0,sizeof(AUCCMD));
		//读13位长度，第14位是校验位，在stx和etx验证过后再读
		read(tty_fd, AUCCMD, 13);
		
		if((AUCCMD[0]==SPD_STX)&&
			(AUCCMD[12]==SPD_ETX))
		{
			unsigned char _lrc[1];
			memset(_lrc,0,sizeof(_lrc));
			read(tty_fd,_lrc,1);
			if(_lrc[0]==OSSPD_tool_lrc(AUCCMD,13))
			{
				printf("TERMINAL GOT CMD\n");
				return 0;
			}
			else
			{
				printf("CMD ERR\n");
				tcflush(tty_fd, TCIOFLUSH);
				return 1;
			}
		}
		else
		{
			if(memcmp(AUCCMD,"quit",4)==0)
				;//_Os_spd_OCSwitch(0);
			else if(strlen(AUCCMD) == 0)
			{
				OSSPD_tty_close();
				return 1;
			}
			else
				printf("CMD ERR\n");
			tcflush(tty_fd, TCIOFLUSH);
		}
	}
	return 1;
}



void OSSPD_cmd_send(int connType,unsigned char *ucCmd)
{
	unsigned char buffer[2],ucTmp[5];
	int i=0,l=0,iReadLen;

	l=strlen(ucCmd);
	
	if(l>255)
		return;

	memset(ucTmp,0,sizeof(ucTmp));
	memset(buffer,0,sizeof(buffer));
	sprintf(ucTmp,"%d",l);
	OSSPD_tool_asc2hex(buffer,1,ucTmp,strlen(ucTmp));	
	switch(connType)
	{
		case C_UART:
			write(g_tty_fd,&buffer[0],1);	

			for(i=0;i<l;i++)
				write(g_tty_fd,&ucCmd[i],1);
			break;
		case C_OTG:
			write(g_otg_fd,&buffer[0],1);	

			for(i=0;i<l;i++)
				write(g_otg_fd,&ucCmd[i],1);
			break;
		case C_NW:
			OSSPD_nw_send(buffer,1);
			OSSPD_nw_send(ucCmd,l);
			break;
	}
}

void OSSPD_cmd_title(int connType,unsigned char ucCmd)
{
	unsigned char buffer[14],U_VER[5];
	int i=0,eF,eS;

	memset(buffer,0,sizeof(buffer));
	buffer[0]=SPD_STX;
	buffer[12]=SPD_ETX;

	memset(U_VER,0,sizeof(U_VER));
	eF=Os__param_load(SPD_CONFIG_FILE);
  	eS=Os__param_findSection(eF,"SERVER_CONFIG",0);
 	Os__param_get(eS,"VER",U_VER,2);	
	Os__param_unload(eF);
	
	memcpy(&buffer[2],U_VER,2);
	buffer[1]=ucCmd;//DOWNCMD_RESPFILE;
	buffer[13]=OSSPD_tool_lrc(buffer,13);
	switch(connType)
	{
		case C_UART:
			for(i=0;i<14;i++)
				write(g_tty_fd,&buffer[i],1);
			break;
		case C_OTG:
			for(i=0;i<14;i++)
				write(g_otg_fd,&buffer[i],1);
			break;
		case C_NW:
			OSSPD_nw_send(buffer,14);
			break;
	}
}

int OSSPD_nw_open(void)
{
	int i;
	if(client_socket<=0)
	{
		int etcFile,etcSection;
    	struct sockaddr_in client_addr,server_addr;

		memset(SERVER_IP,0,sizeof(SERVER_IP));
		etcFile=Os__param_load(SPD_CONFIG_FILE);
  		etcSection=Os__param_findSection(etcFile,"SERVER_CONFIG",0);
 		Os__param_get(etcSection,"SERVER_IP",SERVER_IP,sizeof(SERVER_IP));
 		Os__param_unload(etcFile);
		//对读取的网络IP地址进行处理
 		i = 0;
 		while(i < sizeof(SERVER_IP))
 		{
 			if(SERVER_IP[i] == ' ')
 			{
 				SERVER_IP[i] = '\0';
 				break;
 			}
 			i++;
 		}
	
    	bzero(&client_addr,sizeof(client_addr)); 
    	client_addr.sin_family = AF_INET; 
    	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
		//系统自动分配一个空闲端口
    	client_addr.sin_port = htons(0);
    	client_socket=socket(AF_INET,SOCK_STREAM,0);
		if(client_socket<0)
        {close(client_socket);client_socket=-1;return -1;}

    	if(bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    	{close(client_socket);client_socket=-1;return -2;}
    
    	bzero(&server_addr,sizeof(server_addr));
    	server_addr.sin_family = AF_INET;
    	if(inet_aton(SERVER_IP,&server_addr.sin_addr) == 0)
   		{close(client_socket);client_socket=-1;return -3;}
	
		server_addr.sin_port = htons(SERVER_PORT);
    	socklen_t server_addr_length = sizeof(server_addr);
    	if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
		{close(client_socket);client_socket=-1;return -4;}	
	
	}
	return 0;
}

void OSSPD_nw_close(void)
{
	if(client_socket>0)
	{
    	close(client_socket);
		client_socket=-1;
	}
}



int OSSPD_nw_send(unsigned char *pucInBuf,unsigned short uiInLen)
{
    int ret;
    struct timeval tv;    
    int err_cnt = 0;

    tv.tv_sec = 20; 
    tv.tv_usec = 0;    

    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));

	while(err_cnt<5)
	{
    	ret = send(client_socket, pucInBuf, uiInLen, 0);
		if(ret>0)
			break;
	}


    return 0;
}


int OSSPD_nw_recv(unsigned char * pucBuf,unsigned int *puiLen,unsigned short uiBlock)
{
    int ret,i,flags;

	flags = fcntl(client_socket, F_GETFL, 0);
	if(uiBlock)
		fcntl(client_socket, F_SETFL, flags & ~O_NONBLOCK);
	else
		fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

    ret = recv(client_socket, pucBuf, *puiLen, 0);

	if(ret==-1)
    {
		*puiLen=0;
		return -1;
	}
    else  
      *puiLen = ret;

    return 1;
}

/*
*检测网口数据
有效数据- 0
other - 1
*/
int OSSPD_nw_checkRead()
{
	int l=13;

	l=13;
	memset(AUCCMD,0,sizeof(AUCCMD));
	ret=OSSPD_nw_recv(AUCCMD,&l,3);
	//printf("--> %d\n",ret);
	if(ret>0)
	{
		if((AUCCMD[0]==SPD_STX)&&
			(AUCCMD[12]==SPD_ETX))
		{
			unsigned char _lrc[1];
			memset(_lrc,0,sizeof(_lrc));
			l=1;
			OSSPD_nw_recv(_lrc,&l,3);
			if(_lrc[0]==OSSPD_tool_lrc(AUCCMD,13))
			{
				return 0;
			}
			else
			{
				printf("CMD ERR\n");
				_nwflush();
				return 1;	
			}
		}
		else
		{
			printf("CMD ERR\n");
			_nwflush();
		}

	}
	return 0;
}


/*
创建挂载目录并挂载
*/
void OSSPD_mt_mount(unsigned char * _dev)
{
	unsigned char ucParam[100];
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"mkdir %s",SPD_MT);
	system(ucParam);
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"mount %s %s",_dev,SPD_MT);
	system(ucParam);
}

/*
卸载并删除挂载目录
*/
void OSSPD_mt_unmount()
{
	unsigned char ucParam[100];
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"umount %s",SPD_MT);
	system(ucParam);
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"rmdir -rf %s",SPD_MT);
	system(ucParam);
}



/**
挂载/卸载设备
_active		0-umount
			1-mount
_dev	 	default-tf
			2-usb
return 		0-success
			-1-device not exist
*/
int OSSPD_ms_active(int _active,int _dev)
{
	unsigned char ucParam[100];
	
	if(_active)
	{
		if(_dev==2)
		{
			if(access(SPD_DEV_USB,F_OK)==0)
				OSSPD_mt_mount(SPD_DEV_USB);
			else
				return -1;
		}
		else
		{
			if(access(SPD_DEV_TF,F_OK)==0)
				OSSPD_mt_mount(SPD_DEV_TF);
			else
				return -1;
		}
	}
	else
		OSSPD_mt_unmount();
	return 0;
}

/**
根据配置更新系统
*/
int OSSPD_package_localUpdate(void)
{
	int ret = -1, i = 0;
	unsigned char ucParam[100],ucPath[200],ucID[6],ucUpdate[2],ucEdit[2],ucHide[2];
	int eFile,eSection,c=0,curUpNum=0,talUpNum=0;

	//读取配置参数,根据配置更新	
	memset(ucPath,0,sizeof(ucPath));
	sprintf(ucPath,"%s%s",SPD_PLN_PATH,SPD_FILE_CONF);
	eFile=Os__param_loadFullPath(ucPath);

	memset(&msg,0,sizeof(UP_SER_MSG));
	msg.msgtyp = MSG_TYPE_SERVER;
	msg.msg_data.ucOption = 2;
	msg.msg_data.ucProcess = 0;
	msgsnd(msg_id, &msg, sizeof(UP_SER_MSG), IPC_NOWAIT);

	if(eFile>0)
	{
  		//获取conf.ini文件中APP的个数
		while(1)
		{
			memset(ucParam,0,sizeof(ucParam));
			sprintf(ucParam,"APP%d",c+1);
			
			ret = Os__param_findSection(eFile,ucParam,0);
			
			if(ret<=0)
				break;
			//计算需要更新的应用的总数量
			memset(ucUpdate,0,sizeof(ucUpdate));
			Os__param_get(ret,"UPDATE",ucUpdate,1);
			if(ucUpdate[0] == 0x31)
				talUpNum++;
			c++;
		}
		ret = -1;
		for(i=0;i<c;i++)
		{
			memset(ucParam,0,sizeof(ucParam));
			sprintf(ucParam,"APP%d",(i+1));
			eSection=Os__param_findSection(eFile,ucParam,0);

			memset(ucID,0,sizeof(ucID));
			Os__param_get(eSection,"ID",ucID,5);
			
			memset(ucEdit,0,sizeof(ucEdit));
			Os__param_get(eSection,"EDIT",ucEdit,1);
			
			memset(ucHide,0,sizeof(ucHide));
			Os__param_get(eSection,"HIDDEN",ucHide,1);

			memset(ucUpdate,0,sizeof(ucUpdate));
			Os__param_get(eSection,"UPDATE",ucUpdate,1);
			//根据UPDATE标志进行更新与否
			if(ucUpdate[0] == 0x31)
			{
				//更新
				memset(ucPath,0,sizeof(ucPath));
				sprintf(ucPath,"%s%d.dta",SPD_PLN_PATH,atoi(ucID));
				if(access(ucPath,F_OK) == 0)
				{
					//解压并处理

					memset(ucParam,0,sizeof(ucParam));
					sprintf(ucParam,"/bin/unzip %s -d /",ucPath);
					
					if(system(ucParam)==0)
					{
						if(ucEdit[0] != 0x30)
						{
							//修改conf.ini文件,将ucEdit和ucHide值写入conf.ini
							int cFile,cSection;
							memset(ucPath,0,sizeof(ucPath));
							sprintf(ucPath,"%s%s%s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH,SPD_FILE_CONF);
							cFile=Os__param_loadFullPath(ucPath);
							if(cFile > 0)
							{
								cSection=Os__param_findSection(cFile,"APP0",0);
								Os__param_set(cSection,"EDIT",ucEdit);
								Os__param_set(cSection,"HIDDEN",ucHide);
								Os__param_saveFullPath(cFile,ucPath);
								Os__param_unload(cFile);
							}
						}
						OSSPD_package_allocate();
					}
					else
					{
						ret = -1;
						goto _exit;
					}
					//删除下装目录下的文件
					memset(ucParam,0,sizeof(ucParam));
					sprintf(ucParam,"rm -rf %s/* ",SPD_DOWN_PATH);
					system(ucParam);
				}
				else
				{
					ret = -1;
					goto _exit;
				}
				
				memset(&msg,0,sizeof(UP_SER_MSG));
				msg.msgtyp = MSG_TYPE_SERVER;
				msg.msg_data.ucOption = 2;
				msg.msg_data.ucProcess = (++curUpNum)*100/talUpNum;
				msgsnd(msg_id, &msg, sizeof(UP_SER_MSG), IPC_NOWAIT);
				
		  		ret = 0;
			}
		}
		Os__param_unload(eFile);
	}

_exit:
	
	memset(&msg,0,sizeof(UP_SER_MSG));
	msg.msgtyp = MSG_TYPE_SERVER;
	msg.msg_data.ucProcess = 0;

	if(ret == 0)
  		msg.msg_data.ucOption = 4;
	else
  		msg.msg_data.ucOption = 3;
  		
	msgsnd(msg_id, &msg, sizeof(UP_SER_MSG), IPC_NOWAIT);

	return ret;
}


void OSSPD_exit()
{
  	printf("\nSPD EXIST\n");
	OSSPD_nw_close();
  	OSSPD_tty_close();
	exit(0);
}


/*
*获取加密文件夹的全部名字

int OSSPD_file_getEncryptFolder(unsigned char *ucPath,unsigned char *ucFileName,unsigned char *ucRet)
{
	unsigned char ucFolderName[33],ucTmp[129],ucFile[100];
	MD5_CTX md5;
	int fd,i=0;

	memset(ucFolderName,0,sizeof(ucFolderName));
	
 	//计算文件夹前8位，由文件取30个字节计算md5产生
 	memset(ucFile,0,sizeof(ucFile));
	sprintf(ucFile,"%s%s",ucPath,ucFileName);

	fd=open(ucFile,O_RDONLY,0777);
	if (fd < 0)
			return SPD_FILE;

	memset(ucTmp,0,sizeof(ucTmp));
	read(fd,ucTmp,30);
	close(fd);
	
	MD5Init(&md5);         
 	MD5Update(&md5,ucTmp,30);
	MD5Final(&md5,ucFolderName);        

	memset(ucTmp,0,sizeof(ucTmp));
	//计算文件夹后8位，由应用号计算md5产生
 	MD5Init(&md5);         
 	MD5Update(&md5,ucFileName,5);
	MD5Final(&md5,&ucFolderName[8]);        

	OSSPD_tool_hex2asc(ucRet,ucFolderName,32);
	printf("%s\n",ucRet);
	return 0;
}*/

/*
*根据应用列表对应用改和删除
*/
void OSSPD_file_appListEdit()
{
	unsigned char ucParam[50],ucCmd[20],ucApp[10],ucFile[50];
	int eFile,eSection,c=1,ret=1,tp=0;

	memset(ucFile,0,sizeof(ucFile));
	sprintf(ucFile,"%s%s%s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	eFile=Os__param_loadFullPath(ucFile);

	//遍历新配置，根据EDIT做出处理，顺序已经在下装的时候调整好了
	if(eFile>0)
	{
	 	while(1)
	 	{
			memset(ucApp,0,sizeof(ucApp));
			sprintf(ucApp,"APP%d",c);
			ret=Os__param_findSection(eFile,ucApp,0);
			if(ret<=0)
				break;
			
			memset(ucParam,0,sizeof(ucParam));
			Os__param_get(ret,"EDIT",ucParam,1);
	
	
			//覆盖应用（把交易文件删掉）
			if(ucParam[0]==PROCESS_COVER)
			{
				memset(ucParam,0,sizeof(ucParam));
				Os__param_get(ret,"ID",ucParam,5);
				memset(ucCmd,0,sizeof(ucCmd));
				sprintf(ucCmd,"rm -rf %s%s%s",SPD_APP_PATH,ucParam,SPD_APP_DATA);
				system(ucCmd);
				Os__param_set(ret,"EDIT","0");
			}
			//应用需要删除
			else if(ucParam[0]==PROCESS_DEL)
			{
				memset(ucParam,0,sizeof(ucParam));
				Os__param_get(ret,"ID",ucParam,5);
				memset(ucCmd,0,sizeof(ucCmd));
				sprintf(ucCmd,"rm -rf %s%s",SPD_APP_PATH,ucParam);
				ret=system(ucCmd);
				tp=Os__param_rmSection(eFile,ucApp);	
			}
			c++;
		}
	}
	//更新配置文件
	tp=Os__param_saveFullPath(eFile,ucFile);
	Os__param_unload(eFile);
	//复制到APPMANAGER目录
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"cp %s %s  >/dev/null 2>&1",ucFile,SPD_APPMANAGER_PATH);
	system(ucParam);
	
}

int OSSPD_file_getAppCount()
{
	unsigned char ucParam[50];
	int eFile,eSection,c=1,ret=1;
	
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	eFile=Os__param_loadFullPath(ucParam);
	if(eFile>0)
 	while(1){
		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"APP%d",c);
		ret=Os__param_findSection(eFile,ucParam,0);
		if(ret<=0)
			break;
		c++;
	}
	Os__param_unload(eFile);
	return c-1; 
}


/*
根据appid更新配置
*/
void OSSPD_file_updateAppInList(int editSection)
{
	unsigned char ucParam[30],ucEdtParam[30],ucPath[30],ucNew[10];
	int eFile,eSection,c=1,i=0;

	memset(ucPath,0,sizeof(ucPath));
	sprintf(ucPath,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	eFile=Os__param_loadFullPath(ucPath);
	if(eFile>0)
	{

		c=OSSPD_file_getAppCount();
		memset(ucEdtParam,0,sizeof(ucEdtParam));
		Os__param_get(editSection,"ID",ucEdtParam,sizeof(ucEdtParam));
		for(i=0;i<c;i++)
		{
			memset(ucNew,0,sizeof(ucNew));
			sprintf(ucNew,"APP%d",(i+1));
			
			eSection=Os__param_findSection(eFile,ucNew,1);
			memset(ucParam,0,sizeof(ucParam));
			Os__param_get(eSection,"ID",ucParam,sizeof(ucParam));
			if(memcmp(ucParam,ucEdtParam,5)==0)
			{
				memset(ucParam,0,sizeof(ucParam));
				Os__param_get(editSection,"ID",ucParam,sizeof(ucParam));
				Os__param_set(eSection,"ID",ucParam);
		
				memset(ucParam,0,sizeof(ucParam));
				Os__param_get(editSection,"NAME",ucParam,sizeof(ucParam));
				Os__param_set(eSection,"NAME",ucParam);

				memset(ucParam,0,sizeof(ucParam));
				Os__param_get(editSection,"EDITDATE",ucParam,sizeof(ucParam));
				Os__param_set(eSection,"EDITDATE",ucParam);

				memset(ucParam,0,sizeof(ucParam));
				Os__param_get(editSection,"HIDDEN",ucParam,sizeof(ucParam));
				Os__param_set(eSection,"HIDDEN",ucParam);
	
				memset(ucParam,0,sizeof(ucParam));
				Os__param_get(editSection,"VERSION",ucParam,sizeof(ucParam));
				Os__param_set(eSection,"VERSION",ucParam);

				memset(ucParam,0,sizeof(ucParam));
				Os__param_get(editSection,"ITEMCODE",ucParam,sizeof(ucParam));
				Os__param_set(eSection,"ITEMCODE",ucParam);
	
				Os__param_saveFullPath(eFile,ucPath);
				break;
			}
		}
		Os__param_unload(eFile);
	}
}

/*
在配置文件里的最后增加一项
*/
void OSSPD_file_addAppInList(int addSection)
{
	unsigned char ucParam[30],ucPath[30],ucNew[10],ucID[10];
	int eFile,eSection,c=1,i;

	memset(ucPath,0,sizeof(ucPath));
	sprintf(ucPath,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	eFile=Os__param_loadFullPath(ucPath);

	if(eFile>0)
	{
		c=OSSPD_file_getAppCount();
		//判断新加应用是否已经存在
		memset(ucID,0,sizeof(ucID));
		Os__param_get(addSection,"ID",ucID,sizeof(ucID));
		for(i=1;i<=c;i++)
		{
			memset(ucNew,0,sizeof(ucNew));
			sprintf(ucNew,"APP%d",i);
			eSection=Os__param_findSection(eFile,ucNew,0);
			
			memset(ucParam,0,sizeof(ucParam));
			Os__param_get(eSection,"ID",ucParam,sizeof(ucParam));
			
			if(strcmp(ucParam,ucID) == 0)
				return;
		}
		
		memset(ucNew,0,sizeof(ucNew));
		sprintf(ucNew,"APP%d",(c+1));

		eSection=Os__param_findSection(eFile,ucNew,1);

		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"ID",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"ID",ucParam);
		
		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"NAME",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"NAME",ucParam);

		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"EDITDATE",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"EDITDATE",ucParam);

		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"HIDDEN",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"HIDDEN",ucParam);
	
		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"VERSION",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"VERSION",ucParam);

		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"ITEMCODE",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"ITEMCODE",ucParam);
	
		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"OWNER",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"OWNER",ucParam);
		
		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"CHECKCODE",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"CHECKCODE",ucParam);
		
		memset(ucParam,0,sizeof(ucParam));
		Os__param_get(addSection,"EDIT",ucParam,sizeof(ucParam));
		Os__param_set(eSection,"EDIT",ucParam);
		
		Os__param_saveFullPath(eFile,ucPath);
		
		Os__param_unload(eFile);
	}
}


/*
根据daemon名称获取daemon的状态
取注释标志
*/
int OSSPD_file_getDaemonState(unsigned char *ucDaemon)
{
	unsigned char ucBuf[100];	 
	FILE *wordFile;
	char *str1,*str2,*ptr;
	int ret=0;
		
	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"%s%s",SPD_APPMANAGER_PATH,SPD_FILE_DAEMONSH);
	wordFile = fopen(ucBuf,"r");
	memset(ucBuf,0,sizeof(ucBuf));	   
	if(wordFile==NULL)
		return 0;
	while(fgets(ucBuf, 100, wordFile))
	{
		str1=ucBuf;
		ptr=strstr(str1,ucDaemon);
		if(ptr)
		{
			if(memcmp(&ucBuf[0],"#",1)==0)
				ret=0;
			else
				ret=1;	
			
			break;	
		}
	}

	fclose(wordFile);
	
	return ret;

}
/*
根据驱动名称获取驱动的状态
ucFlg 1-取调试标志 2-取注释标志
*/
int OSSPD_file_getDriverState(unsigned char *ucDriver,int ucFlg)
{
	unsigned char ucBuf[100];	 
	FILE *wordFile;
	char *str1,*str2,*ptr;
	int ret=0;
		
	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"%s%s",SPD_DRIVER_PATH,SPD_FILE_DRVSH);
	wordFile = fopen(ucBuf,"r");
	memset(ucBuf,0,sizeof(ucBuf));	   
	if(wordFile==NULL)
		return 0;
	while(fgets(ucBuf, 100, wordFile))
	{
		str1=ucBuf;
		ptr=strstr(str1,ucDriver);
		if(ptr)
		{
			if(ucFlg==1)
			{
				if(memcmp(&ucBuf[strlen(ucBuf)-2],"1",1)==0)
					ret=1;
				else
					ret=0;
			}
			else if(ucFlg==2)
			{
				if(memcmp(&ucBuf[0],"#",1)==0)
					ret=0;
				else
					ret=1;	
			}
			
			break;	
		}
	}

	fclose(wordFile);
	
	return ret;

}

/*
根据下载的驱动和库，替换掉原来的文件
*/
void OSSPD_file_createSysFile(unsigned char ucType)
{
	unsigned char _type[10],_folder[100],ucBuf[100],ucFile[50],ucTmp[50];
	struct dirent* ent = NULL;
	DIR *pDir;
	char *str1,*str2,*ptr;
	
	memset(_folder,0,sizeof(_folder));
	memset(_type,0,sizeof(_type));
	switch(ucType)
	{
	case TYPE_DRV:	
		sprintf(_type,".ko.");
		sprintf(_folder,"%s%s",SPD_DOWN_PATH,SPD_DRIVER_PATH);
		break;
	case TYPE_LIB:
		sprintf(_type,".so.");
		sprintf(_folder,"%s%s",SPD_DOWN_PATH,SPD_LIB_PATH);
		break;
	case TYPE_FONT:
		sprintf(_type,".ft.");
		sprintf(_folder,"%s%s",SPD_DOWN_PATH,SPD_FONT_PATH);
		break;		
	case TYPE_DAEMON:
		sprintf(_type,".dm.");
		sprintf(_folder,"%s%s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH);
		break;		
	}

	pDir=opendir(_folder);  
	while (NULL != (ent=readdir(pDir)))
	{
		if (ent->d_type & DT_DIR )
		{}//这里不做递归
		else
		{
				if(strstr(ent->d_name,_type)!=NULL)
				{
					memset(ucBuf,0,sizeof(ucBuf));
					switch(ucType)
					{
						case TYPE_DRV:
							str2=".ko.";
							sprintf(ucBuf,"%s",SPD_DRIVER_PATH);
							break;
						case TYPE_LIB:
							str2=".so.";
							sprintf(ucBuf,"%s",SPD_LIB_PATH);
							break;
						case TYPE_FONT:
							str2=".ft.";
							sprintf(ucBuf,"%s",SPD_FONT_PATH);
							break;	
						case TYPE_DAEMON:
							str2=".dm.";
							sprintf(ucBuf,"%s",SPD_APPMANAGER_PATH);
							break;
					}

					//取得文件名
					str1=ent->d_name;
					ptr = strstr(str1, str2);
					if(ptr)
					{
						memset(ucFile,0,sizeof(ucFile));
						memcpy(ucFile,str1,(ptr-str1+3));
						//这里是用户请求删除
						if(memcmp(&ucFile[0],"~",1)==0)
						{
							memset(ucTmp,0,sizeof(ucTmp));
							sprintf(ucTmp,"rm -rf %s%s*",ucBuf,&ucFile[1]);
							//printf("%s\n",ucTmp);
							system(ucTmp);
							memset(ucTmp,0,sizeof(ucTmp));
							sprintf(ucTmp,"rm -rf %s%s",_folder,str1);
							//printf("%s\n",ucTmp);
							system(ucTmp);
						}
						//更新
						else
						{
							//删除原本的软链和源
							memset(ucTmp,0,sizeof(ucTmp));
							sprintf(ucTmp,"rm -rf %s%s*",ucBuf,ucFile);
							//printf("%s\n",ucTmp);
							system(ucTmp);
						

							memset(ucBuf,0,sizeof(ucBuf));
							switch(ucType)
							{
							case TYPE_DRV:
								//复制源
								sprintf(ucBuf,"mv %s%s %s",_folder,str1,SPD_DRIVER_PATH);
								//printf("%s\n",ucBuf);
								system(ucBuf);
								//做软连接
								memset(ucBuf,0,sizeof(ucBuf));
								sprintf(ucBuf,"ln -s %s%s %s%s",SPD_DRIVER_PATH,str1,SPD_DRIVER_PATH,ucFile);
								//printf("%s\n",ucBuf);
								system(ucBuf);
								break;
							case TYPE_LIB:
								//复制源
								sprintf(ucBuf,"mv %s%s %s",_folder,str1,SPD_LIB_PATH);
								//printf("%s\n",ucBuf);
								system(ucBuf);
								memset(ucBuf,0,sizeof(ucBuf));
								//做软联
								sprintf(ucBuf,"ln -s %s%s %s%s",SPD_LIB_PATH,str1,SPD_LIB_PATH,ucFile);
								//printf("%s\n",ucBuf);
								system(ucBuf);
								//更改lib的权限
								system("chmod 755 /lib/*");
								break;
							case TYPE_FONT:
								sprintf(ucBuf,"mv %s%s %s",_folder,str1,SPD_FONT_PATH);
								//printf("%s\n",ucBuf);
								system(ucBuf);
								//更改font的权限
								system("chmod 755 /fonts/*");
								break;
							case TYPE_DAEMON:
								//复制源
								sprintf(ucBuf,"mv %s%s %s",_folder,str1,SPD_APPMANAGER_PATH);
								//printf("%s\n",ucBuf);
								system(ucBuf);
								memset(ucBuf,0,sizeof(ucBuf));
								//做软联
								sprintf(ucBuf,"ln -s %s%s %s%s",SPD_APPMANAGER_PATH,str1,SPD_APPMANAGER_PATH,ucFile);
								//printf("%s\n",ucBuf);
								system(ucBuf);
								break;
							}
						}
					}
					
				}
			}

	}
	
	//把driver下的start.sh拷贝过去
	memset(ucBuf,0,sizeof(ucBuf));
	switch(ucType)
	{
		case TYPE_DRV:
		sprintf(ucBuf,"mv %s%s %s",_folder,SPD_FILE_DRVSH,SPD_DRIVER_PATH);
		//printf("%s\n",ucBuf);
		system(ucBuf);
		//更改drivers的权限
		system("chmod 755 /drivers/*");
		break;
		case TYPE_DAEMON:
		sprintf(ucBuf,"mv %s%s %s",_folder,SPD_FILE_DAEMONSH,SPD_APPMANAGER_PATH);
		//printf("%s\n",ucBuf);
		system(ucBuf);
		//更改daemon的权限
		system("chmod 755 /daemon/*");
		break;
	}
	closedir(pDir);
}


/*
删除下载的应用中需要删除的文件
*/
void OSSPD_file_deleteAppFile(unsigned char *ucPath)
{
	unsigned char ucParam[100],ucFile[50];
	struct dirent* ent = NULL;
	DIR *pDir;
	int i,j;
	
	pDir = opendir(ucPath);

	if(pDir == NULL)
		return;
	while (NULL != (ent=readdir(pDir)))
	{
		if (ent->d_type & DT_DIR)
		{
			/*
			memset(ucParam,0,sizeof(ucParam));
			if(*(ucPath+strlen(ucPath)-1) == '/')
				sprintf(ucParam,"%s%s",ucParam,ent->d_name);
			else
				sprintf(ucParam,"%s/%s",ucParam,ent->d_name);
			OSSPD_file_deleteAppFile(ucParam);
			*/
		}
		else
		{
			//取得文件名
			memset(ucFile,0,sizeof(ucFile));
			strcpy(ucFile,ent->d_name);
			//检查文件名字中是否有空格有空格进行处理
			if(strstr(ent->d_name," "))
			{
				i = 0, j = 0;
				while(i < 255)
				{
					if(ent->d_name[i] == ' ')
					{
						ucFile[j++] = '\\';
						ucFile[j++] = ent->d_name[i++];
						continue;
					}
					else if(ent->d_name[i] == '\0')
					{
						break;
					}
					ucFile[j++] = ent->d_name[i++];
				}
			}

			if(strlen(ucFile) > 0 && memcmp(&ucFile[0],"~",1)==0)
			{
				memset(ucParam,0,sizeof(ucParam));
				if(*(ucPath+strlen(ucPath)-1) == '/')
					sprintf(ucParam,"rm -rf %s%s",ucPath,&ucFile[1]);
				else
					sprintf(ucParam,"rm -rf %s/%s",ucPath,&ucFile[1]);
				system(ucParam);

				memset(ucParam,0,sizeof(ucParam));
				if(*(ucPath+strlen(ucPath)-1) == '/')
					sprintf(ucParam,"rm -rf %s%s",ucPath,&ucFile[0]);
				else
					sprintf(ucParam,"rm -rf %s/%s",ucPath,&ucFile[0]);
				system(ucParam);

			}
		}
	}
	closedir(pDir);

}

/*
根据传入的文件类型，生成配置
*/
void OSSPD_file_createSysIni(unsigned char ucType)
{
	char _type[10],_folder[20],ucBuf[50],ucPath[50],ucName[30];
	int eFile,eSection,c=1,istate;
	struct dirent* ent = NULL;
	char *str1,*str2,*ptr;
	DIR *pDir;

	memset(_type,0,sizeof(_type));
	memset(_folder,0,sizeof(_folder));
	memset(ucBuf,0,sizeof(ucBuf));
	memset(ucPath,0,sizeof(ucPath));
	switch(ucType)
	{
	case TYPE_DRV:
		sprintf(_type,".ko.");
		sprintf(_folder,"%s",SPD_DRIVER_PATH);
		sprintf(ucBuf,"touch %s%s%s",SPD_DOWN_PATH,SPD_DRIVER_PATH,SPD_FILE_CONF);
		system(ucBuf);
		sprintf(ucPath,"%s%s%s",SPD_DOWN_PATH,SPD_DRIVER_PATH,SPD_FILE_CONF);
		break;
	case TYPE_LIB:
		sprintf(_type,".so.");
		sprintf(_folder,"%s",SPD_LIB_PATH);
		sprintf(ucBuf,"touch %s%s%s",SPD_DOWN_PATH,SPD_LIB_PATH,SPD_FILE_CONF);
		system(ucBuf);
		sprintf(ucPath,"%s%s%s",SPD_DOWN_PATH,SPD_LIB_PATH,SPD_FILE_CONF);
		break;
	case TYPE_FONT:
		sprintf(_type,".ft.");
		sprintf(_folder,"%s",SPD_FONT_PATH);
		sprintf(ucBuf,"touch %s%s%s",SPD_DOWN_PATH,SPD_FONT_PATH,SPD_FILE_CONF);
		system(ucBuf);
		sprintf(ucPath,"%s%s%s",SPD_DOWN_PATH,SPD_FONT_PATH,SPD_FILE_CONF);
		break;	
	case TYPE_DAEMON:
		sprintf(_type,".dm.");
		sprintf(_folder,"%s",SPD_APPMANAGER_PATH);
		sprintf(ucBuf,"touch %s%s%s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH,SPD_FILE_DAEMONLST);
		system(ucBuf);
		memset(ucBuf,0,sizeof(ucBuf));
		sprintf(ucBuf,"cp %s%s %s%s",SPD_APPMANAGER_PATH,SPD_FILE_DAEMONSET,SPD_DOWN_PATH,SPD_APPMANAGER_PATH);
		system(ucBuf);
		sprintf(ucPath,"%s%s%s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH,SPD_FILE_DAEMONLST);
		break;
	}
	eFile=Os__param_loadFullPath(ucPath);

	pDir=opendir(_folder);  
	while (NULL != (ent=readdir(pDir)))
	{
		if (ent->d_type & DT_DIR )
		{}//这里不做递归
		else
		{
				if(strstr(ent->d_name,_type)!=NULL)
				{
					memset(ucBuf,0,sizeof(ucBuf));
					switch(ucType)
					{
						case TYPE_DRV:
							sprintf(ucBuf,"KO%d",c);
							str2=".ko.";
							break;
						case TYPE_LIB:
							sprintf(ucBuf,"LIB%d",c);
							str2=".so.";
							break;
						case TYPE_FONT:
							sprintf(ucBuf,"FONT%d",c);
							str2=".ft.";
							break;	
						case TYPE_DAEMON:
							sprintf(ucBuf,"DAEMON%d",c);
							str2=".dm.";
							break;
					}
					eSection=Os__param_findSection(eFile,ucBuf,1);
					str1=ent->d_name;
					ptr = strstr(str1, str2); 
					Os__param_set(eSection,"version",ptr+4);
					
					memset(ucName,0,sizeof(ucName));
					memcpy(ucName,str1,(ptr-str1+3));
					Os__param_set(eSection,"name",ucName);
					switch(ucType)
					{
					case TYPE_DRV:
						istate=OSSPD_file_getDriverState(ucName,1);
						memset(ucBuf,0,sizeof(ucBuf));
						sprintf(ucBuf,"%d",istate);
						Os__param_set(eSection,"dbg",ucBuf);
	
						istate=OSSPD_file_getDriverState(ucName,2);
						memset(ucBuf,0,sizeof(ucBuf));
						sprintf(ucBuf,"%d",istate);
						Os__param_set(eSection,"insmod",ucBuf);
						break;
					case TYPE_DAEMON:
						istate=OSSPD_file_getDaemonState(ucName);
						memset(ucBuf,0,sizeof(ucBuf));
						sprintf(ucBuf,"%d",istate);
						Os__param_set(eSection,"insmod",ucBuf);
						break;
					default:
						break;
					}
					c++;
				}
			}
	}


	closedir(pDir);
	Os__param_saveFullPath(eFile,ucPath);
	Os__param_unload(eFile);
}



/*
* 新增应用
*/
void OSSPD_file_addApp(int eSection)
{
	unsigned char ucParam[100],ucID[6],ucApp[33],ucPath[100], ucEditState;

	//程序需要覆盖，则把原程序删除
	memset(ucParam,0,sizeof(ucParam));
	Os__param_get(eSection,"EDIT",ucParam,1);
	ucEditState = ucParam[0];
	memset(ucID,0,sizeof(ucID));
	Os__param_get(eSection,"ID",ucID,5);

	//取得加密后的文件夹名字,现在不用加密
	if(SPD_EPT_APP)
	{
	//memset(ucApp,0,sizeof(ucApp));
	//OSSPD_file_getEncryptFolder(SPD_APP_PATH,ucID,ucApp);
	}

	if(ucEditState==PROCESS_COVER)
	{
		memset(ucParam,0,sizeof(ucParam));
		if(SPD_EPT_APP)
			;//sprintf(ucParam,"rm -rf %s%s",SPD_APP_PATH,ucApp);
		else
			sprintf(ucParam,"rm -rf %s%s",SPD_APP_PATH,ucID);
		system(ucParam);
	}

	//复制程序
	memset(ucParam,0,sizeof(ucParam));
	if(SPD_EPT_APP)
	{
	//	memset(ucPath,0,sizeof(ucPath));
	//	sprintf(ucPath,"%s%s",SPD_DOWN_PATH,SPD_APP_PATH);
	//	memset(ucApp,0,sizeof(ucApp));
	//	OSSPD_file_getEncryptFolder(ucPath,ucID,ucApp);
	//	sprintf(ucParam,"cp -rf %s%s%s %s%s",SPD_DOWN_PATH,SPD_APP_PATH,ucID,SPD_APP_PATH,ucApp);
	}
	else
	{	
		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"chmod 755 %s%s%s/* >/dev/null 2>&1",SPD_DOWN_PATH, SPD_APP_PATH, ucID);
		system(ucParam);
		memset(ucParam,0,sizeof(ucParam));
		//如果应用下的image文件夹存在
		sprintf(ucParam,"%s%s%s%s", SPD_DOWN_PATH, SPD_APP_PATH, ucID, SPD_FILE_PNG);
		if(access(ucParam,F_OK) == 0)
		{
			sprintf(ucParam,"chmod 755 %s/* >/dev/null 2>&1", ucParam);
			system(ucParam); 
		}
		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"cp -rf %s%s%s %s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_APP_PATH,ucID,SPD_APP_PATH);
		system(ucParam);
		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"%s%s",SPD_APP_PATH,ucID);
		OSSPD_file_deleteAppFile(ucParam);
		sprintf(ucParam,"%s%s",ucParam,SPD_FILE_PNG);
		OSSPD_file_deleteAppFile(ucParam);
	}

	//新增
	if(ucEditState==PROCESS_ADD)
		OSSPD_file_addAppInList(eSection);
	//更新和覆盖去更新配置
	else
		OSSPD_file_updateAppInList(eSection);


	//删除临时文件
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"rm -rf %s%s/*",SPD_DOWN_PATH,SPD_APP_PATH);
	system(ucParam);

	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"rm -rf %s%s%s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	system(ucParam);
}

/*
*根据传入类型，烧写uboot和kernel
*/
void OSSPD_file_flashWrite(unsigned char ucType)
{
	unsigned char ucParam[100],ucCMD[100];
	int ret=0;

	//判断文件存在不存在
	memset(ucParam,0,sizeof(ucParam));
	switch(ucType)
	{
		case TYPE_UBOOT:
			sprintf(ucParam,"%s%s%s",SPD_DOWN_PATH,SPD_UBOOT_PATH,SPD_FILE_BOOT);
			break;
		case TYPE_KERNEL:
			sprintf(ucParam,"%s%s%s",SPD_DOWN_PATH,SPD_KERNEL_PATH,SPD_FILE_KERNEL);
			break;
	}
	//printf("%s \n",ucParam);
	ret=OSSPD_tool_folderExist(ucParam);	
	//printf("ret %d\n",ret);
	if(ret==0)
		return;
	//printf("ready to write\n");
	switch(ucType)
	{
	case TYPE_UBOOT:
		memset(ucCMD,0,sizeof(ucCMD));
		sprintf(ucCMD,"flash_eraseall %s",SPD_MTD0);
		//printf("%s\n",ucCMD);
		if(system(ucCMD))
		{
			memset(ucCMD,0,sizeof(ucCMD));
			sprintf(ucCMD,"dd if=%s of=%s conv=sync",ucParam,SPD_MTD0);
			//printf("%s\n",ucCMD);
			if(system(ucCMD))
			{needReboot=1;}
		}
		break;
	case TYPE_KERNEL:
		memset(ucCMD,0,sizeof(ucCMD));
		sprintf(ucCMD,"flash_eraseall %s",SPD_MTD2);
		//printf("%s\n",ucCMD);
		if(system(ucCMD))
		{
			memset(ucCMD,0,sizeof(ucCMD));
			sprintf(ucCMD,"dd if=%s of=%s conv=sync",ucParam,SPD_MTD2);
			//printf("%s\n",ucCMD);
			if(system(ucCMD))
			{needReboot=1;}
		}
		break;
	}

}

int OSSPD_package_request(int connType)
{
	int iLen=0,iRead=0,i,iT=0,iAppID=0,iType=0,iReadLen;
	unsigned char cpCMD[50],ucTmp[50],ucPath[50];
	unsigned char ucApp[5];
	
	//读一个字节长度
	memset(AUCCMD,0,sizeof(AUCCMD));
	switch(connType)
	{
		case C_UART:	
			read(g_tty_fd, AUCCMD, 1);
			break;
		case C_OTG:
			read(g_otg_fd, AUCCMD, 1);
			break;
		case C_NW:
			iReadLen=1;
			OSSPD_nw_recv(AUCCMD,&iReadLen,1);
			break;
	}
	iLen=AUCCMD[0];
		


	//读请求数据
	memset(AUCCMD,0,sizeof(AUCCMD));
	switch(connType)
	{
		case C_UART:
			while(iLen>0)
			{
				iRead= read(g_tty_fd, &AUCCMD[iT], sizeof(AUCCMD));
		
				iT+=iRead;
				iLen-=iRead;

				iRead=0;
			}
			break;
		case C_OTG:
			while(iLen>0)
			{
				iRead= read(g_otg_fd, &AUCCMD[iT], sizeof(AUCCMD));
		
				iT+=iRead;
				iLen-=iRead;

				iRead=0;
			}
			break;
		case C_NW:
			iReadLen=sizeof(AUCCMD);
			OSSPD_nw_recv(AUCCMD,&iReadLen,1);
			iT=iReadLen;
			break;	
	}
	iLen=iT;

	//校验lrc
	if(AUCCMD[iLen-1]!=OSSPD_tool_lrc(AUCCMD,iLen-1))
		return SPD_LRC;
	//处理数据，每4个字节表示一个文件
	else
	{
		iT=(iLen-1)/4;
		for(i=0;i<iT;i++)
		{
			memset(ucTmp,0,sizeof(ucTmp));
			iAppID= (AUCCMD[i*4+2]<<8)+(AUCCMD[i*4+3]);
			iType=0;
			//这里当itype=1时，是请求程序列表，其他为请求程序
			//appmanager目录下有点特别，有两个列表，应用程序列表和daemon列表，itype=2时候是请求dm列表
			switch(AUCCMD[i*4+1])
			{
			case REQUEST_FILETYPE_CONFIG:
				sprintf(ucTmp,"%s ",SPD_FILE_CONF);
				iType=1;
				break;
			case REQUEST_FILETYPE_SETTING:
				sprintf(ucTmp,"%05d/%s ",iAppID,SPD_FILE_CONF);
				break;
			case REQUEST_FILETYPE_BIN:
				//sprintf(ucTmp,"%05d/%s ",iAppID,SPD_FILE_APP);
				sprintf(ucTmp,"%05d/%05d ",iAppID,iAppID);
				break;
			case REQUEST_FILETYPE_BINLOGO:
				sprintf(ucTmp,"%05d/%s ",iAppID,SPD_FILE_ICO);
				break;
			case REQUEST_FILETYPE_BINIMG:
				sprintf(ucTmp,"%05d%s -rf ",iAppID,SPD_FILE_PNG);
				break;
			case REQUEST_FILETYPE_DAEMON:
				iType=2;
				sprintf(ucTmp," ");
				break;
			case REQUEST_FILETYPE_SCRIPT:
				sprintf(ucTmp,"%05d/%s ",iAppID,SPD_FILE_SCRIPT);
				break;
			default:
				memset(ucTmp,0,sizeof(ucTmp));
				break;
			} 
			
			
			if(strlen(ucTmp)>0)
			{
			 	memset(cpCMD,0,sizeof(cpCMD));
				memset(ucPath,0,sizeof(ucPath));
				switch(AUCCMD[i*4])
				{
				case REQUEST_FOLDERTYPE_APP:
					sprintf(ucPath,"mkdir -p %s%s%d/ >/dev/null 2>&1",SPD_DOWN_PATH,SPD_APP_PATH,iAppID);
					system(ucPath);
					sprintf(cpCMD,"cp %s%s %s%s%d/",SPD_APP_PATH,ucTmp,SPD_DOWN_PATH,SPD_APP_PATH,iAppID);
					break;	
				case REQUEST_FOLDERTYPE_DRIVER:
					if(iType)
					{
					//如果请求drive配置，则从目录中遍历*.ko.*,动态生成文件
					memset(cpCMD,0,sizeof(cpCMD));
					sprintf(ucPath,"mkdir -p %s%s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_DRIVER_PATH);
					system(ucPath);
					OSSPD_file_createSysIni(TYPE_DRV);
					}
					else
					{
					memset(cpCMD,0,sizeof(cpCMD));
					sprintf(ucPath,"mkdir -p %s%s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_DRIVER_PATH);
					system(ucPath);
					sprintf(cpCMD,"cp %s%s %s%s",SPD_DRIVER_PATH,ucTmp,SPD_DOWN_PATH,SPD_DRIVER_PATH);
					}
					break;
				case REQUEST_FOLDERTYPE_LIB:
					if(iType==1)
					{
					memset(cpCMD,0,sizeof(cpCMD));
					sprintf(ucPath,"mkdir -p %s%s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_LIB_PATH);
					system(ucPath);
					OSSPD_file_createSysIni(TYPE_LIB);
					}
					else
					{
					sprintf(ucPath,"mkdir -p %s%s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_LIB_PATH);
					system(ucPath);
					sprintf(cpCMD,"cp %s%s %s%s",SPD_LIB_PATH,ucTmp,SPD_DOWN_PATH,SPD_LIB_PATH);
					}
					break;
				case REQUEST_FOLDERTYPE_FONT:
					if(iType==1)
					{
					memset(cpCMD,0,sizeof(cpCMD));
					sprintf(ucPath,"mkdir -p %s%s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_FONT_PATH);
					system(ucPath);
					OSSPD_file_createSysIni(TYPE_FONT);
					}
					else
					{
					sprintf(ucPath,"mkdir -p %s%s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_FONT_PATH);
					system(ucPath);
					sprintf(cpCMD,"cp %s%s %s%s",SPD_FONT_PATH,ucTmp,SPD_DOWN_PATH,SPD_FONT_PATH);
					}
					break;
				case REQUEST_FOLDERTYPE_APPMANAGER:
					if(iType==2)
					{
					memset(cpCMD,0,sizeof(cpCMD));
					sprintf(ucPath,"mkdir -p %s%s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_APPMANAGER_PATH);
					system(ucPath);	
					OSSPD_file_createSysIni(TYPE_DAEMON);
					}
					else
					{
					sprintf(ucPath,"mkdir -p %s%s >/dev/null 2>&1",SPD_DOWN_PATH,SPD_APPMANAGER_PATH);
					system(ucPath);
					sprintf(cpCMD,"cp %s%s %s%s",SPD_APPMANAGER_PATH,ucTmp,SPD_DOWN_PATH,SPD_APPMANAGER_PATH);
					}
					break;
				default:
					memset(cpCMD,0,sizeof(cpCMD));
					break;
				}
			}

			if(strlen(cpCMD)>0)
			{
				system(cpCMD);
				//printf("%s\n",cpCMD);
			}
		}

		//打包
		system("rm -rf /tmp/sandpos.zip");
		memset(ucTmp,0,sizeof(ucTmp));
		sprintf(ucTmp,"/bin/zip -q -r -1 /tmp/sandpos.zip %s/* ",SPD_DOWN_PATH);
		if(system(ucTmp)==0)
		{
			//用完后把文件夹删掉
			memset(ucTmp,0,sizeof(ucTmp));
			sprintf(ucTmp,"rm -rf %s/*",SPD_DOWN_PATH);
			system(ucTmp);
			//准备发送
			iT=OSSPD_package_send(connType);
			usleep(100*1000);
			//printf("PACKAGE SEND SUCCESS RET %d\n",iT);
		}
	}

}

/*
*发送打包数据
*/
int OSSPD_package_send(int connType)
{
	int file_fd,byte_read,i=0,total=0;
	char buffer[1024],ucTmp[30];
	int ret=SPD_SUCCESS,wret;
	unsigned char *pucFile,*pucPtr;
	//看下文件存在不存在
	if((file_fd=open("/tmp/sandpos.zip",O_RDONLY))==-1)
		return SPD_FILE;

	OSSPD_cmd_title(connType,DOWNCMD_RESPFILE);


	//发送4个字节的文件长度
	memset(buffer,0,sizeof(buffer));
	memset(ucTmp,0,sizeof(ucTmp));
	byte_read = lseek(file_fd, 0, SEEK_END); 
	sprintf(ucTmp,"%02x",byte_read);
	OSSPD_tool_asc2hex(buffer,4,ucTmp,strlen(ucTmp));	

	switch(connType)
	{
	case C_UART:
		for(i=0;i<4;i++)
			write(g_tty_fd,&buffer[i],1);	
		break;
	case C_OTG:
		for(i=0;i<4;i++)
			write(g_otg_fd,&buffer[i],1);	
		break;
	case C_NW:
		OSSPD_nw_send(buffer,4);
		break;
	}


	lseek(file_fd, 0, SEEK_SET); 
	while(byte_read=read(file_fd,buffer,sizeof(buffer)))
	{
		if((byte_read==-1)&&(errno!=EINTR)) 
		{ret=SPD_FILE;break;}
		else if(byte_read>0)
		{
			switch(connType)
			{
			case C_UART:
				for(i=0;i<byte_read;i++)
					write(g_tty_fd,&buffer[i],1);	
				break;
			case C_OTG:
				for(i=0;i<byte_read;i++)
					write(g_otg_fd,&buffer[i],1);	
				break;
			case C_NW:
				OSSPD_nw_send(buffer,byte_read);
				break;
			}
		}
	}

	/*
	//一边读一边发送好像比较慢，改成开辟空间，读完后一次性发送
	lseek(file_fd, 0, SEEK_SET); 
	pucFile = (unsigned char *)malloc(SPD_FILESIZE_MAX);
	pucPtr=pucFile;

	while(byte_read>0)
	{
		byte_read=read(file_fd,pucPtr,512);
		pucPtr+=byte_read;
		total+=byte_read;
	}
	
	pucPtr-=total;	
	for(i=0;i<total;i++)
	{
		write(g_tty_fd,pucPtr,1);	
		pucPtr++;
	}
	
	free(pucFile);
	*/
	close(file_fd);
	return ret;
}



/*
*根据配置分配下装文件
*/
void OSSPD_package_allocate(void)
{ 
	unsigned char ucParam[100];
	int eFile,eSection,_id;

	//处理daemon
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s%s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH,SPD_FILE_DAEMONSH);
	if(OSSPD_tool_folderExist(ucParam))
	{
		//遍历文件夹，并ln出需要的库
		OSSPD_file_createSysFile(TYPE_DAEMON);
	}
	//处理app
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH);
	if(OSSPD_tool_folderExist(ucParam))
	{
		sprintf(&ucParam[strlen(ucParam)],"%s",SPD_FILE_CONF);
		eFile=Os__param_loadFullPath(ucParam);

		if(eFile>0)
		{
			eSection=Os__param_findSection(eFile,"APP0",0);

			//新增
			if(eSection>0)
	 			OSSPD_file_addApp(eSection);
			//删除，修改
			else
				OSSPD_file_appListEdit();
			Os__param_unload(eFile);
		}
	
		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"cp %s%s* %s",SPD_DOWN_PATH,SPD_APPMANAGER_PATH,SPD_APPMANAGER_PATH);	
		system(ucParam);

		memset(ucParam,0,sizeof(ucParam));
		sprintf(ucParam,"rm -rf %s",SPD_DOWN_PATH);	
		system(ucParam);
	}
	//处理drivers	
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_DOWN_PATH,SPD_DRIVER_PATH);
	if(OSSPD_tool_folderExist(ucParam))
	{
		//遍历文件夹，并ln出需要的库
		OSSPD_file_createSysFile(TYPE_DRV);
	}
	//处理libs
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_DOWN_PATH,SPD_LIB_PATH);
	if(OSSPD_tool_folderExist(ucParam))
	{
		//遍历文件夹，并ln
		OSSPD_file_createSysFile(TYPE_LIB);
	}
	//处理fonts
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_DOWN_PATH,SPD_FONT_PATH);
	if(OSSPD_tool_folderExist(ucParam))
	{
		OSSPD_file_createSysFile(TYPE_FONT);
	}
	//处理uboot
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_DOWN_PATH,SPD_UBOOT_PATH);
	if(OSSPD_tool_folderExist(ucParam))
	{
		OSSPD_file_flashWrite(TYPE_UBOOT);
	}
	//处理kernel
	memset(ucParam,0,sizeof(ucParam));
	sprintf(ucParam,"%s%s",SPD_DOWN_PATH,SPD_KERNEL_PATH);
	if(OSSPD_tool_folderExist(ucParam))
	{
		OSSPD_file_flashWrite(TYPE_KERNEL);
	}
}

/*
处理下发的程序或者配置文件
*/
int OSSPD_package_download(int connType)
{
	int file_fd=0,read_byte=0,l=0,iReadLen;
	unsigned char ucBuf[1024];
	int ret=SPD_SUCCESS;
	long long iLen=0;
	
	//读四个字节长度
	memset(AUCCMD,0,sizeof(AUCCMD));
	switch(connType)
	{
		case C_UART:
			read(g_tty_fd, AUCCMD, 4);
			break;
		case C_OTG:
			read(g_otg_fd, AUCCMD, 4);
			break;
		case C_NW:
			iReadLen=4;
			OSSPD_nw_recv(AUCCMD,&iReadLen,1);
			break;
	}
	iLen=(AUCCMD[0]<<32)+(AUCCMD[1]<<16)+(AUCCMD[2]<<8)+(AUCCMD[3]<<0);

	//创建文件
	system("rm -rf /tmp/update.zip");
	file_fd=open("/tmp/update.zip",O_RDWR|O_CREAT,0777);
	if (file_fd < 0)
			return SPD_FILE;
	
	printf("\nreceiving file...\n");

	//写文件
	while(1)
	{
		switch(connType)
		{
			case C_UART:
				read_byte=read(g_tty_fd,ucBuf,sizeof(ucBuf));
				break;
			case C_OTG:
				read_byte=read(g_otg_fd,ucBuf,sizeof(ucBuf));
				break;
			case C_NW:
				iReadLen=sizeof(ucBuf);
				OSSPD_nw_recv(ucBuf,&iReadLen,0);
				read_byte=iReadLen;
				break;
		}
		lseek(file_fd,l, SEEK_SET); 
		l+=read_byte;
		if(read_byte>0)
		{
			if(l>=iLen)
				read_byte=read_byte-(l-iLen);
				
			ret=write(file_fd, ucBuf, read_byte);

		}
		
		if (ret<0)
		{ret=SPD_WRITEFILE;break;}
		if(l>=iLen)
		{ret=SPD_SUCCESS;break;}
	}

	close(file_fd);
	
	//解压并处理
	if(ret==SPD_SUCCESS)
	{
		
		if(system("/bin/unzip /tmp/update.zip -d /  >/dev/null 2>&1")==0)
		{
			OSSPD_cmd_title(connType,DOWNCMD_REQUEST);
			OSSPD_cmd_send(connType,"ok");
			usleep(20*1000);
			OSSPD_cmd_title(connType,DOWNCMD_REQUEST);
			OSSPD_cmd_send(connType,"ok");
			
			//处理
			//printf("allocate\n");
			OSSPD_package_allocate();	
			/*
			switch(connType)
			{
				case C_UART:
					//处理完了清除串口，防止pc段不接受完成指令就发送
					tcflush(g_tty_fd, TCIOFLUSH);
					break;
				case C_OTG:
					//处理完了清除串口，防止pc段不接受完成指令就发送
					tcflush(g_otg_fd, TCIOFLUSH);
					break;
				case C_NW:
					_nwflush();
					break;
			}
			*/
		}
		else
			printf("failed\n");
	}
	//删除下装目录下的文件
	memset(AUCCMD,0,sizeof(AUCCMD));
	sprintf(AUCCMD,"rm -rf %s/* ",SPD_DOWN_PATH);
	system(AUCCMD);
}


/*
*加密数据判断&包类型判断
*@param 通讯类型
*/
void OSSPD_package_deal(int connType)
{
	//判断处理类型
	switch(AUCCMD[1])
	{
	case DOWNCMD_REQUEST:
		OSSPD_package_request(connType);
		break;
	case DOWNCMD_RESPFILE:
		OSSPD_package_download(connType);
		break;
	case DOWNCMD_LOCAL:
		OSSPD_package_localUpdate();
		break;
	case DOWNCMD_RESPKEY:
		break;
	default:
		printf("CMD ERR\n");
		switch(connType)
		{
			case C_UART:
				tcflush(g_tty_fd, TCIOFLUSH);
				break;
			case C_OTG:
				tcflush(g_otg_fd, TCIOFLUSH);
			case C_NW:
				_nwflush();
				break;
		}
		break;
	}
	
	system("sync");
}

void _os_spd_existSignal()
{
	signal(SIGINT, OSSPD_exit);
	signal(SIGTERM, OSSPD_exit);
}


void _closeUpdate()
{
	int etcFile,etcSection;

	etcFile=Os__param_load(SPD_CONFIG_FILE);
	etcSection=Os__param_findSection(etcFile,"MODULE_ACTIVE",0);
	Os__param_set(etcSection,"SMT_OPEN","0");
	Os__param_save(etcFile,SPD_CONFIG_FILE);
	Os__param_unload(etcFile);
	system("sync");
 
}


/*
*下装函数入口
*处理串口,tf/usb数据
*/
void _Os_spd_checkUpdate()
{
	//读配置文件，来控制串口和网络的开关
	//由于这里配置文件是外部改变的，所以需要一直读	
	etcFile=Os__param_load(SPD_CONFIG_FILE);
	etcSectionSMT=Os__param_findSection(etcFile,"MODULE_ACTIVE",0);
  	Os__param_get(etcSectionSMT,"SMT_OPEN",SMT_OPEN,1);	
	etcSection=Os__param_findSection(etcFile,"SERVER_CONFIG",0);
 	Os__param_get(etcSection,"CONNECT_STATE",CONNECT_S,1);	

  	/***************
	1-UART 2-OTG2UART 3-WLAN 4-WIFI 5-TF 6-U DISK 
  	****************/
  	if(SMT_OPEN[0]==0x31)
	{
		if(CONNECT_S[0]==0x31)
		{	
			if(OSSPD_tty_open(SPD_C_UART)==0)
			{
	 			//非阻塞读串口数据
				SPD_DATA_FLG=OSSPD_tty_checkRead(g_tty_fd);
	
				if(SPD_DATA_FLG==0)
					OSSPD_package_deal(C_UART);
			}
			
		}	
		
		if(CONNECT_S[0]==0x32)
		{
			if(OSSPD_tty_open(SPD_C_OTG)==0)
			{				
	 			//非阻塞读串口数据
				SPD_DATA_FLG=OSSPD_tty_checkRead(g_otg_fd);
				if(SPD_DATA_FLG==0)
					OSSPD_package_deal(C_OTG);
			}
		}
		
		//初始化网络服务	
		if(CONNECT_S[0]==0x33||CONNECT_S[0]==0x34)
		{
			if(OSSPD_nw_open()==0)
			{
				SPD_DATA_FLG=OSSPD_nw_checkRead();
				if(SPD_DATA_FLG==0)
					OSSPD_package_deal(C_NW);
			}
		}
			
		//检测TF卡和U盘
		if(CONNECT_S[0]==0x35||CONNECT_S[0]==0x36)
		{
			AUCCMD[1]=DOWNCMD_LOCAL;
			OSSPD_package_deal(NULL);
					
			_closeUpdate();
		}
		
		sleep(1);	
		
	}
	else
	{
		
		if(CONNECT_S[0]==0x31||CONNECT_S[0]==0x32)
			OSSPD_tty_close();
		if(CONNECT_S[0]==0x33||CONNECT_S[0]==0x34)
			OSSPD_nw_close();
		
		sleep(3);
	}
	
	Os__param_unload(etcFile);
}


/*
清除socket接收缓冲
*/
void _nwflush(void)
{
	struct timeval tmOut;      
	tmOut.tv_sec = 0;
	tmOut.tv_usec = 0;
	fd_set         fds;
	FD_ZERO(&fds);
	FD_SET(client_socket, &fds);

	int nRet;
	char tmp[2];

	memset(tmp, 0, sizeof(tmp));

	while(1)
	{   
		nRet= select(FD_SETSIZE, &fds, NULL, NULL, &tmOut);
        if(nRet== 0)
			break;
		recv(client_socket, tmp, 1,0);
	}
}

void _Os_spd_ini()
{
	unsigned char ucBuf[100];
	
	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",SPD_APP_PATH);	
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",SPD_FONT_PATH);	
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",SPD_DOWN_PATH);	
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",SPD_DRIVER_PATH);	
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",SPD_FONT_PATH);	
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",SPD_BIDDY_PATH);	
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"mkdir %s >/dev/null 2>&1",SPD_LIB_PATH);	
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"touch %s%s >/dev/null 2>&1",SPD_DRIVER_PATH,SPD_FILE_DRVSH);	
	system(ucBuf);
	
	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"touch %s%s >/dev/null 2>&1",SPD_APPMANAGER_PATH,SPD_FILE_DAEMONSH);
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"touch %s%s >/dev/null 2>&1",SPD_APPMANAGER_PATH,SPD_FILE_DAEMONSET);
	system(ucBuf);

	memset(ucBuf,0,sizeof(ucBuf));
	sprintf(ucBuf,"touch %s%s >/dev/null 2>&1",SPD_APPMANAGER_PATH,SPD_FILE_CONF);
	system(ucBuf);
}

int main(int argc, char *argv[])
{
	int i=0;
	
	
	
	//设置了dbg调试标志
	if(_checkdbg(argc,argv)!=0)
		return -1;
	
	//spd的退出信号，用于关闭端口，回收资源
	_os_spd_existSignal();
	
	//初始化文件夹和必要文件
	_Os_spd_ini();
	
	//创建消息队列
	_msg_create();
	
	
		
	while(1){
		_Os_spd_checkUpdate();
	}
	return 0;
}
