/*
  *File:      GPRS under linux OS based on PPP for PS4000
  *Data:    July 6, 2009
  *Author: huzhigang<huzhigang@sand.cn>
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include<sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <termios.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <oscfg_param.h>
#include <appdef.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define dbg //printf

#define PowerOn      0
#define PowerOff      1
#define GprsReset    2
#define DEVICE_NAME  	"/dev/lattice"
#define CFG_GSM_FILE	"/mnt/pos/config/osgsm.cfg"

#ifndef bzero
#define bzero(a, b)             memset(a, 0, b)
#endif

#define	SUCCESS 	0x00
#define FAILED    	0x01
#define BAUD_ERROR	0x02

#define MAXINTERFACES 16
int fd_s3c2410_serial0;
volatile unsigned int failed_times =0;
#ifdef QC430_v01
unsigned char gprs_serial_num[21] = "/dev/s3c2410_serial1";
#else
unsigned char gprs_serial_num[21] = "/dev/s3c2410_serial0";
#endif
unsigned long gprs_uart_speed = 115200;
unsigned char OSGSM_GprsCheckDial(unsigned short uiTimeout);
unsigned char OSGSM_GprsSetAPN(unsigned char* pucPtr);
unsigned char WNetSendCmd(unsigned char *cmd, unsigned char *rsp, unsigned short ms);


unsigned char *long_to_asc(unsigned char *Ptd, unsigned char Lgd, unsigned long *Pts )
{
        unsigned char I,
        Oct,
        *Pt0,
        Tb[10];
        unsigned long Lg1,
        Lg2;

        Lg1 = *Pts;
        Lg2 = 100000000L ;
        for (I = 0; I< 6; I++) { //5---->6
                Oct = (unsigned char)(Lg1 / Lg2) ;
                Tb[2*I] = Oct / 10 + 0x30 ;
                Tb[2*I+1] = Oct % 10 + 0x30;
                Lg1 = Lg1 % Lg2;
                Lg2 = Lg2 / 100;
        }

        (void) memset( Ptd, 0x30, Lgd ) ;
        Ptd += Lgd ;
        Pt0 = Ptd ;
        if ( Lgd > 10 ) Lgd = 10 ;
        for ( I=0; I < Lgd; I++) *--Ptd = Tb[9-I] ;

        return((unsigned char*)Pt0);
}


/*GPRS serial number setting*/
unsigned char Gprs_UartNo_Setting(unsigned char No )
{
        switch (No) {
        case '0':
                strcpy(gprs_serial_num,"/dev/s3c2410_serial0");
                break;

        case '1':
                strcpy(gprs_serial_num,"/dev/s3c2410_serial1");
                break;

        case '2':
                strcpy(gprs_serial_num,"/dev/s3c2410_serial2");
                break;
        default:
                printf("Gprs_UartNo_Setting parameter fault!\n");
                return FAILED;
        }
        return SUCCESS;
}
/*GPRS Uart speed setting*/
unsigned char Gprs_UartSpeed_Setting(unsigned long Uart_Speed)
{
        unsigned char 	cmd[200];
        unsigned char	baud_set[10];
        unsigned long	baud_val;
        char 	rsp[200];

        gprs_uart_speed = Uart_Speed;

        memset(baud_set, 0x00, sizeof(baud_set));
        memset(cmd, 0x00, sizeof(cmd));
        memset(rsp, 0x00, sizeof(rsp));
		
	    strcpy(cmd, "AT+IPR=");
        long_to_asc(baud_set, sizeof(baud_set), &baud_val);
        strncpy(cmd, baud_set, strlen(baud_set));
        WNetSendCmd(cmd, rsp, 3000);

        return SUCCESS;
}
/*
struct _cfg_gsm {
	unsigned char  version;
	unsigned char  de_flag;
	unsigned char  uart_no;
	unsigned long  uart_speed;
	unsigned char  always_online;
	unsigned char  gsm_cdma;
	unsigned char  APN[32];
	unsigned char  username[40];
	unsigned char  password[20];
	unsigned char  manu_id[40];
	unsigned char  mode_id[40];
	unsigned char  vern_id[40];
	unsigned char  IMEI[40];
	unsigned char  CCID[40];

	unsigned char  crc[2];
};
*/

/*GPRS Parameter Setting*/
unsigned char OSCFG_GsmSettingLoad_new (void)
{
        //FILE *fp;
        int fp;
        struct _cfg_gsm  s_cfg_gsm;
        char result;

		printf("[%s:%d] -- CFG_GSM_FILE = %s \n",__func__,__LINE__,CFG_GSM_FILE);

		if ((fp = open(CFG_GSM_FILE, O_RDONLY)) < 0){
                printf("[%s:%d]-- Open the CFG_GSM_FILE failed!\n",__func__,__LINE__);
                return FAILED;
        }
  

        if (read(fp, &s_cfg_gsm, sizeof(s_cfg_gsm)) < 0) {
                printf("Read the CFG_GSM_FILE failed!\n");
				close(fp);
                return FAILED;
        }
		
        close(fp);
		

		Uart_Printf("%s\n",s_cfg_gsm.APN);
		
        OSGSM_GprsSetAPN(s_cfg_gsm.APN);

        return 0;

};




unsigned int  test_ppp0 (void)
{
        unsigned int fd, intrface, retn = 0,i = 1;
        char *ppp0_name= "ppp0";
        struct ifreq buf[MAXINTERFACES];
        struct arpreq arp;
        struct ifconf ifc;

        if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) {
                ifc.ifc_len = sizeof buf;
                ifc.ifc_buf = (caddr_t) buf;

                if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) {
                        intrface = ifc.ifc_len / sizeof (struct ifreq);
                        while (intrface-- > 0) {
                                if (strcmp(ppp0_name,buf[intrface].ifr_name)==0) {
                                        //printf ("net device %s\n", buf[intrface].ifr_name);
                                        if (!(ioctl (fd, SIOCGIFFLAGS, (char *) &buf[intrface]))) {
                                                if (buf[intrface].ifr_flags & IFF_PROMISC) {
                                                        //puts ("the interface is PROMISC");
                                                        retn++;
                                                }
                                        } else {
                                                char str[256];
                                                sprintf (str, "cpm: ioctl device %s", buf[intrface].ifr_name);
                                                perror (str);
                                        }

                                        if (buf[intrface].ifr_flags & IFF_UP) {
                                                //puts("the interface status is UP");
                                        } else {
                                                //puts("the interface status is DOWN");
                                        }

                                        if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface]))) {
                                                //puts ("IP address is:");
                                                //puts(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
                                                //puts("");
                                        } else {
                                                char str[256];
                                                sprintf (str, "cpm: ioctl device %s", buf[intrface].ifr_name);
                                                perror (str);
                                        }
                                        i=0;
                                        break;
                                }

                        }

                } else
                        perror ("cpm: ioctl");

        } else
                perror ("cpm: socket");
        close (fd);
        return i;
}




unsigned char open_port(void)
{
        //fd_s3c2410_serial0 = open("/dev/s3c2410_serial0", O_RDWR|O_NOCTTY|O_NDELAY);
        fd_s3c2410_serial0 = open(gprs_serial_num, O_RDWR|O_NOCTTY|O_NDELAY);
        //printf("fd=%d\n",fd_s3c2410_serial0);
        if (fd_s3c2410_serial0 == -1) {
                //printf("Unable to open uart\n");
                return FAILED;
        } else
                //fcntl(fd, F_SETFL, 0);
                return SUCCESS;
}



unsigned char setup_uart(int fd)
{
        struct termios oldtio, newtio;
        if ((tcgetattr(fd, &oldtio)) != 0) {
                //printf("Save old error!\n");
                return FAILED;
        }
        bzero(&newtio, sizeof(newtio));
        newtio.c_cflag |= (CLOCAL | CREAD);
        newtio.c_cflag &= ~CSIZE;
        newtio.c_cflag &= ~CSTOPB;// 1 stop bit
        newtio.c_cflag &= ~PARENB;// No parity
        newtio.c_cflag |= CS8; // 8 bits data

        /*设置波特率*/
        //cfsetispeed(&newtio, B115200);
        //cfsetospeed(&newtio, B115200);
        switch (gprs_uart_speed) {
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

        newtio.c_cc[VTIME] = 0;
        newtio.c_cc[VMIN]  = 0;
        tcflush(fd, TCIFLUSH);

        if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
                //printf("Set new error!\n");
                return FAILED;
        }

        return SUCCESS;
}

unsigned char GprsInit(void)
{
        unsigned char 	result;

        //OSCFG_GsmSettingLoad ();

        result  = open_port();
        if (result == FAILED) return FAILED;

        result = setup_uart(fd_s3c2410_serial0);
        if (result == FAILED)
                return  (FAILED);
        else
                return SUCCESS;
}

unsigned char GprsExit(void)
{
        unsigned char 	result;

        result = close(fd_s3c2410_serial0);
        if (result ==SUCCESS) {
                //printf("close the fd!!!\n");
                return SUCCESS;
        } else {
                return FAILED;
        }
}
/*GPRS模块接受AT命令*/
unsigned char WNetSendCmd(unsigned char *cmd, unsigned char *rsp, unsigned short ms)
{
        int i;
        unsigned char c, str[200];
        unsigned char 	*pucPtr;

        memset(str, 0, sizeof(str));
        strcpy(str, cmd);
        i = 0;
        str[strlen(cmd)] = 13;
        str[strlen(cmd)+1] = '\0';
        i = write(fd_s3c2410_serial0, str, strlen(str));


        dbg("\ni=%d\n",i);
        dbg("str1=%s\n", str);

        if (i <= 0)  return FAILED;

        usleep(100000);
        memset(str, 0x00, sizeof(str));
        usleep(200000);

        i = read(fd_s3c2410_serial0, str, 200);
        dbg("i=%d\n",i);
        dbg("str2=%s\n", str);


        strcpy(rsp,str);

        return SUCCESS;
}

/*GPRS打开电源*/
unsigned char OSGSM_GprsPowerOn(void)
{
        int fd, result;
        fd = open(DEVICE_NAME, O_RDWR);

        if (fd == -1) {
                printf("open device %s error\n",DEVICE_NAME);
        }

        result = ioctl(fd, PowerOn);
        if (result != 0)
                printf("GRPS_PowerOn  ioctl failed!!!\n");

        close(fd);
        return SUCCESS;

}

/*GPRS关闭电源*/
unsigned char OSGSM_GprsPowerOff(void)
{
        int fd, result;
        fd = open(DEVICE_NAME, O_RDWR);

        if (fd == -1) {
                printf("open device %s error\n",DEVICE_NAME);
        }

        result = ioctl(fd, PowerOff);
        if (result != 0)
                printf("GRPS_PowerOn  ioctl failed!!!\n");

        close(fd);
        return SUCCESS;

}


/*检测GPRS通讯链路是否建立成功*/
unsigned char OSGSM_GprsCheckDial(unsigned short uiTimeout)
{

        unsigned short Timeout;
        unsigned int  result;
        Timeout = uiTimeout;


        result = test_ppp0();
        if (result != 0) {
                do {
                        result = test_ppp0();
                        if (result == 0)  return (SUCCESS);
                        usleep(10000);
                        Timeout --;

                } while (Timeout != 0);

                failed_times++;
                return (FAILED);
        }
        failed_times = 0;

        return (SUCCESS);
}



/*指示终端开始GPRS预拔号*/
unsigned char OSGSM_GprsDial(void)
{
#if 0
        int p_status;
        int Timeout=120;
        char result;

        result =  OSGSM_GprsCheckDial(1);
        if (result != 0x00) {				/*if the gprs on line ,then exit immediately!!!*/
                pid_t PID = fork();
                if (PID == 0) {	/*child*/
                        if (execl("/usr/sbin/pppd", "pppd", "call", "gprs", NULL) < 0) {
                                //printf("Gprs Dial failed\n");
                                exit(1);
                        }
                }

                else if (PID < 0) {
                        //printf("process failed\n");
                        exit(1);
                }

                else {		//parent
                        wait(&p_status);
                }
                //OSGSM_GprsCheckDial(1200);
        }
#endif

        return 0;

}

/*指示终端开始GPRS预拔号*/
unsigned char OSGSM_GprsDial_Daemon(void)
{
        int p_status;
        int Timeout=120;
        char result;

        result =  OSGSM_GprsCheckDial(1);
        if (result != 0x00) {				/*if the gprs on line ,then exit immediately!!!*/
                pid_t PID = fork();
                if (PID == 0) {	/*child*/
                        if (execl("/usr/sbin/pppd", "pppd", "call", "gprs", NULL) < 0) {
                                //printf("Gprs Dial failed\n");
                                exit(1);
                        }
                }

                else if (PID < 0) {
                        exit(1);
                }

                else {		//parent
                        wait(&p_status);
                }
                //OSGSM_GprsCheckDial(1200);
        }
}



/*关闭GPRS通讯链路*/
unsigned char OSGSM_GprsHangupDial(void)
{
#if 0
        int p_status;
        char str[200];
        pid_t PID = fork();

        if (PID == 0) { //child
                if (execl("/etc/ppp/ppp-off", "ppp-off", NULL) < 0) {
                        printf(" Close the Gprs failed\n");
                        exit(1);
                }
        } else if (PID < 0) {
                printf("process failed\n");
                exit(1);
        } else { //parent
                wait(&p_status);
        }
        failed_times = 0;
#endif
        return 0;
}

static unsigned char OSGSM_GprsHangupDial_PR(void)
{
        int p_status;
        char str[200];
        pid_t PID = fork();

        if (PID == 0) { //child
                if (execl("/etc/ppp/ppp-off", "ppp-off", NULL) < 0) {
                        printf(" Close the Gprs failed\n");
                        exit(1);
				}
        } else if (PID < 0) {
                printf("process failed\n");
                exit(1);
        } else { //parent
				system("killall dial");
				system("/daemon/dial -g1 -gprs >/dev/null 2>&1 &");	

		        wait(&p_status);
        }
        failed_times = 0;
}


/*设置GPRS通讯的APN*/
#if 0
unsigned char OSGSM_GprsSetAPN(unsigned char* pucPtr)
{

        FILE *fp;
        int i=0;
        int tem=0;
        char rbuff[1024];
        char wbuff[1024];
        char APN[30];
        strcpy(APN, pucPtr);

        fp=fopen("/etc/ppp/gprs-connect-chat","r+");
        if (NULL == fp) {
                printf("file open failed");
                exit(1);
        }

        memset(rbuff,0,1024);
        memset(wbuff,0,1024);
        fread(rbuff,1024,1,fp);
        while (i<1024) {
                if (rbuff[i]=='I' &&rbuff[i+1]=='P') break;
                i++;
        }

        i+=5;
        tem=i;
        memcpy(wbuff,rbuff,i);
        memcpy(wbuff+i, APN, strlen(APN));

        while (i < 1024) {
                if (rbuff[i]=='"') break;
                i++;
        }

        memcpy(wbuff+tem+strlen(APN), rbuff+i,strlen(rbuff)-i);

        ftruncate(fileno(fp), 0);
        fseek(fp,0,SEEK_SET);
        fwrite(wbuff,strlen(wbuff),1,fp);
        fclose(fp);

        return SUCCESS;

}
#else
unsigned char OSGSM_GprsSetAPN(unsigned char* pucPtr)
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
	
	strncpy(APN, pucPtr,sizeof(APN)-1);

	fp = fopen(chat_script_file, "r+");

	if(NULL == fp)
	{
		printf("[%s:%d] file open failed",__func__,__LINE__); 
		return FAILED;
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
				//printf("%d\n", rbuff[tem+5]);
				apn_buf[j] = rbuff[tem+5];
			}
			break;
		}
		i++;
	}

	if (strcmp(apn_buf,  pucPtr) == 0x00) {
		fclose(fp);
		return SUCCESS;
	}
	//printf("apn_buf=%s\n", apn_buf);
	//printf("pucPtr =%s\n", pucPtr);
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
	//OSGSM_GprsHangupDial_PR();

	return SUCCESS;

}

#endif
/*GPRS硬重启*/
unsigned char OSGSM_HW_GprsReset(void)
{
        int fd, result, i;
  /*
        OSGSM_GprsPowerOff();
        usleep(300000);
        OSGSM_GprsPowerOn();
*/
        fd = open(DEVICE_NAME, O_RDWR);

        if (fd == -1) {
                printf("open device %s error\n",DEVICE_NAME);
        }

        result = ioctl(fd, GprsReset);
        if (result != 0)
                printf("GRPS_RESET  ioctl failed!!!\n");
        close(fd);


        return SUCCESS;

}

/*GPRS软重启*/
unsigned char OSGSM_GprsReset(void)
{
#if 0
        int i;
        int fd;
        unsigned char c,str[200];
        unsigned char *at_cmd = "AT+CFUN=1";

        if (failed_times == 4) {
                OSGSM_HW_GprsReset();
                failed_times = 0;
                return (SUCCESS);
        }

        WNetSendCmd(at_cmd, str, 1);
#endif
        return SUCCESS;
}


/*GPRS智能拨号*/
unsigned char OSSGSM_GprsSmartDial(void)
{
        char result;
        unsigned int i;

        i = 0;
        do {
                OSGSM_GprsDial();
                result = OSGSM_GprsCheckDial(500);
                if (result == 0x00) break;
                else {
                        OSGSM_GprsReset();
                        i++;
                        if (i == 2) {
#if 1
                                OSGSM_GprsDial();
                                result = OSGSM_GprsCheckDial(500);
                                if (result == 0x00)  break;
#endif
                        }

                }
        } while (i <2);
        return (SUCCESS);

}

unsigned int GprsSigalDetect(void)
{
        int	detect_result = 0;
        char 	cmd[200]="AT+CSQ"; //AT指令
        char 	rsp[200];
        unsigned int signal_value;

        memset(rsp, 0, sizeof(rsp));
        WNetSendCmd(cmd, rsp, 3000);

        if ((rsp[15] != 0x00)&&(rsp[16] != 0x00)) {
                signal_value = (rsp[15]  - 0x30)*10 + (rsp[16] - 0x30);
        } else {
                signal_value = 0x00;
        }
        printf("-->signal_value = %d\n", signal_value);

        if (signal_value <= 7) 									detect_result = 0;		/*0-7, signal is bad for communication*/
        else if ( (signal_value >   7) && (signal_value <= 13)) 		detect_result = 1;		/*8--31,signal is ok for communication,*/
        else if ( (signal_value > 13) && (signal_value <= 19)) 	detect_result = 2;		/*if the signal value exceed 31,meaning signal is zero*/
        else if ( (signal_value > 19) && (signal_value <= 25)) 	detect_result = 3;
        else if ( (signal_value > 25) && (signal_value <= 31)) 	detect_result = 4;
        else detect_result = 0;
  
        return detect_result;
}


#if 0
unsigned int gprs_signal_detect_result(void)
{
        unsigned char result_check, result_detect;

        result_check = OSGSM_GprsCheckDial(1);
        if (result_check ==0x01) {
                GprsInit();
                result_detect   = GprsSigalDetect();/*result range from 1 to 5*/
        } else
                result_detect = 4;

        printf("signal value = %d\n", result_detect);
        GprsExit();
        return result_detect;
}
#endif

unsigned char OSGSM_Gsm_Call(unsigned char *phnum)
{
        char 	cmd[200];
        char 	rsp[200];
        unsigned char 	*pucPtr;

        memset(cmd, 0x00, sizeof(cmd));
        strcpy(cmd, "ATD");
        strncpy(cmd+3, phnum, strlen(phnum));
        cmd[3+strlen(phnum)] = ';';

        memset(rsp, 0x00, sizeof(rsp));
        WNetSendCmd(cmd, rsp, 3000);

        pucPtr = (unsigned char *)strstr( rsp, "OK");
        if ( pucPtr ) {
                return	SUCCESS;
        }
        return FAILED;
}

unsigned char OSGSM_Gsm_Call_Check(void)
{
        unsigned char 	str[200];
        unsigned char 	*pucPtr;

        read(fd_s3c2410_serial0, str, 200);

        pucPtr = (unsigned char *)strstr(str, "OK");
        if (!pucPtr)
                return FAILED;
        else
                return SUCCESS;
}

unsigned char OSGSM_Gsm_Hang_up(void)
{
        char 	cmd[200];
        char 	rsp[200];
        unsigned char 	*pucPtr;

        memset(cmd, 0x00, sizeof(cmd));
        strcpy(cmd, "ATH");

        memset(rsp, 0x00, sizeof(rsp));
        WNetSendCmd(cmd, rsp, 3000);

        pucPtr = (unsigned char *)strstr( rsp, "OK");
        if ( pucPtr ) {
                return	SUCCESS;
        }
        return FAILED;
}

unsigned char OSGSM_Gsm_Answer(void)
{
        char 	cmd[200];
        char 	rsp[200];
        unsigned char 	*pucPtr;

        memset(cmd, 0x00, sizeof(cmd));
        strcpy(cmd, "ATA");

        memset(rsp, 0x00, sizeof(rsp));
        WNetSendCmd(cmd, rsp, 3000);

        pucPtr = (unsigned char *)strstr( rsp, "OK");
        if ( pucPtr ) {
                return	SUCCESS;
        }
        return FAILED;
}

unsigned char GSM_ChangeBand(unsigned char ucBandCode)
{
        char 	cmd[200];
        char 	rsp[200];
        unsigned char band_par[10];
        unsigned char 	*pucPtr;

        memset(cmd, 0x00, sizeof(cmd));
        strcpy(cmd, "AT+WMBS=");

        memset(band_par, 0x00, sizeof(band_par));
        band_par[0] = ucBandCode;

        strcat(cmd, band_par);
        memset(rsp, 0x00, sizeof(rsp));

        GprsInit();
        WNetSendCmd(cmd, rsp, 3000);
        GprsExit();

        pucPtr = (unsigned char *)strstr( rsp, "OK");
        if ( pucPtr ) {
                return	SUCCESS;
        }

        pucPtr = (unsigned char *)strstr( rsp, "ERROR");
        if ( pucPtr ) {
                return BAUD_ERROR;
        }

        return FAILED;
}

unsigned char OSGSM_GprsSetDialParam(WIRELESS_PARAM wl)
{
        printf("%s:not implement!!!\n",__func__);
        return 0;
}

#define NETWORK_GPRS_MODULE
//#undef  NETWORK_GPRS_MODULE

#ifdef NETWORK_GPRS_MODULE
void set_ppp_restart(unsigned char flag)
{
    int shm_id;
    unsigned char *shm_addr;
    key_t shm_key = 2100;
	
	printf("[%s:%d] --ppp reset ! \n",__func__,__LINE__);

    if (( shm_id = shmget(shm_key, 4, 777|IPC_CREAT))<0 ){
        fprintf(stderr, "### shmget failed.%s:%d \n",__FUNCTION__,__LINE__);
        return;
    }

    shm_addr = (unsigned char *)shmat(shm_id,NULL,0);

    *((unsigned char*)shm_addr) = flag;

    shmdt(shm_addr);

    return;
}
#else
void set_ppp_restart(unsigned char )
{
}
#endif

/* added 2010-11-19 */
unsigned char Os__netif_exist(char *if_name)
{
    if (strcmp(if_name,"ppp0") == 0) {
        return OSGSM_GprsCheckDial(10);
    }
    return 1;
}

#define PAP_SECRETS      1
#define CHAP_SECRETS     2

const char *pap_head[] = {
    "# Secrets for authentication using PAP",
    "# client	server	     secret			IP addresses"
};

const char *chap_head[] = {
    "# Secrets for authentication using CHAP",
    "# client	server	     secret			IP addresses"
};


static int ppp_set_ap(char type, char *user,char*pwd)
{
    int retval = 0;
    FILE *fp;
    char *path ;
    
    switch (type) {
    case PAP_SECRETS:
        path = (char*)"/etc/ppp/pap-secrets";
        break;

    case CHAP_SECRETS:
    default:
        path = (char*)"/etc/ppp/chap-secrets";
        break;

    }
    if ((fp = fopen(path, "w+")) == NULL){ // read & write & truncat
        fprintf(stderr,"%s:%d fopen fail \n",__func__,__LINE__);
        retval = -1;
        goto exit;
    }

	switch(type){    
	case PAP_SECRETS:
		fputs(pap_head[0],fp);
		fputs("\x0D\x0A",fp);

		fputs(pap_head[1],fp);
		fputs("\x0D\x0A",fp);
		break;
	case CHAP_SECRETS:
		fputs(chap_head[0],fp);
		fputs("\x0D\x0A",fp);

		fputs(chap_head[1],fp);
		fputs("\x0D\x0A",fp);
	}

    fputs("    ",fp);fputc('"',fp);fputs(user,fp);fputc('"',fp);fputs("      ",fp);fputc('*',fp);
    fputs("    ",fp);fputc('"',fp);fputs(pwd, fp);fputc('"',fp);fputs("      ",fp);fputc('*',fp);
    fputs("\x0D\x0A",fp);

exit:
    fclose(fp);
    return retval;
}


#if 0
static int ppp_set_script(char *user, char *ppp_call_name)
{
    int retval = 0;
    FILE *fp;
    char buffer[256];
    long next_line_pos = 0;
    int  offset = 0;
    int c ;
    //char *path =(char*)"/etc/ppp/peers/gprs";

    char *path = ppp_call_name;


    if ((fp = fopen(path, "r+")) == NULL){ // read & write
        fprintf(stderr,"%s:%d fopen fail \n",__func__,__LINE__);
        retval = -1;
        goto exit;
    }

    while (fgets(buffer,256,fp) != NULL)
    {
        if (strncmp(buffer,"user",4)==0)
            break;
    }

    if (feof(fp)){
        fprintf(stderr,"%s:%d To EOF \n",__func__,__LINE__);
        retval = -1;
        goto exit;
    }

    next_line_pos = ftell(fp);

    fseek(fp, next_line_pos - strlen(buffer) ,SEEK_SET);

    //printf("[%s:%d] b4b4 \n",__func__,__LINE__);

    while ((c = fgetc(fp))!= 0x0D && (c != 0x0A )) {
        
		//printf("%x %c\n",c,c);

        if (c=='"') {
            if ((ftell(fp)+strlen(user)) >= next_line_pos) {
                break;
            }
            fputs(user,fp);
            fputc('"',fp);
            do {
                fputc(' ',fp);
            } while (ftell(fp) < next_line_pos - 2);
            fputc(0xd,fp);
            fputc(0xa,fp);
            break;
        }
    }

    //printf("[%s:%d] b5b5 \n",__func__,__LINE__);
exit:
    fclose(fp);
    return retval;
}
#endif

static int ppp_set_script(char *user, char *ppp_call_name)
{
    char *path = ppp_call_name;
    FILE *file_src,*file_dest;
    char string[100], buffer[100],cmd[100];
	char my_user[100];
	int len;
	int retval = 0;
	
    memset(string, 0, 100);
    memset(buffer, 0, 100);
    memset(cmd, 0, 100);
	memset(my_user,0,100);

    len = strlen(user);	
	if (len == 0)
		return 0;

	/* exp : user "cmnet"  */
	my_user[0]='"';
    strcat(my_user , user);
	my_user[strlen(my_user)]='"';

	//printf("[%s:%d]---my_user=%s\n",__func__,__LINE__,my_user);

    strcpy(buffer , "user ");
    strcat(buffer , my_user);

    len = strlen(buffer);

    buffer[len++] = 0x0D;
    buffer[len++] = 0x0A;

    if ((file_src=fopen(path, "r+"))==NULL) {
        fprintf(stderr,"%s:%s",__func__,strerror(errno));
        return -1;
    }

    if ((file_dest=fopen("/etc/ppp/peers/.gprs-cdma.sys", "w+"))==NULL) {
        fprintf(stderr,"%s:%s",__func__,strerror(errno));
        return -1;
    }

    fseek(file_src,0,0);

    while (fgets(string,255,file_src)!= NULL) {

		//printf("[%s:%d]---string %s\n",__func__,__LINE__,string);
        if (strstr(string,"user")) {
            fputs(buffer,file_dest);
        } else
            fputs(string,file_dest);
    }
    fclose(file_src);
    fclose(file_dest);

    /* rm file */
    strcpy(cmd , "rm -f ");
    strcat(cmd ,  path); /* follow  "/etc/ppp/peers/gprs" */
    system(cmd);

    /* mv  file */
    strcpy(cmd , "mv -f /etc/ppp/peers/.gprs-cdma.sys ");
    strcat(cmd ,  path); /* follow   "/etc/ppp/peers/gprs" */
    system(cmd);

	return retval;
}


/*
* Os__ppp_set_secrets 
* Set PPP secrets files 
*/
int OSGSM_ppp_set_secrets(char *user,char *pwd,char *pppd_call_name)
{
    int retval = 0;
	char lo_user[40],lo_pwd[40]; // lower char buffer
	char *psrc,*pdest;

	memset(lo_user,0,sizeof(lo_user));
	memset(lo_pwd,0,sizeof(lo_pwd));

	for (psrc = user ,pdest = lo_user ; *psrc != 0 ; psrc++,pdest++){
		*pdest = tolower((int)(*psrc));
	}

	for (psrc = pwd , pdest = lo_pwd ;  *psrc != 0 ; psrc++,pdest++){
		*pdest = tolower((int)(*psrc));
	}



    printf("user=%s \n",lo_user);
    printf("pwd=%s \n", lo_pwd);
    printf("name=%s \n",pppd_call_name);


    /* user and password for pap-secrets */
	if ((retval = ppp_set_ap(PAP_SECRETS,lo_user, lo_pwd)) < 0)
        return -1;

    //printf("[%s:%d] a1a1 \n",__func__,__LINE__);

	/* user and paeeword for chap-secrets */
    if ((retval = ppp_set_ap(CHAP_SECRETS,lo_user, lo_pwd))< 0)
        return -1;

    //printf("[%s:%d] a2a2 \n",__func__,__LINE__);
	/* user for /etc/peers/gprs or cdma */
    if ((retval = ppp_set_script(lo_user,pppd_call_name)) < 0 )
        return -1;

    //printf("[%s:%d] a3a3 \n",__func__,__LINE__);
	/* let sand_comm_d restart */

    return retval;

}

/*===========================   END OF FILE  ==========================*/

