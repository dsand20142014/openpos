/**********************************************************
 *                       SAND Printer API v10
 * ********************************************************
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <string.h>
#include <unistd.h>    
#include <sys/types.h>  
#include <sys/stat.h>  
#include <termios.h> 
#include <errno.h> 

#include "ss205new.h"
#include "sand_print.h"
//#include "asc1224.c"
//#include "asc816.c"
#include "osdriver.h"

#define USE_Serial_PRINTER 12
#define FPGA_DEV "/dev/lattice"
#define DRVNAME  "/dev/printer"


#define SS205_IOC_MAGIC   'S'
#define SS205_PRINT_CHKSTATUS        _IOWR(SS205_IOC_MAGIC, 8, int)

#ifdef QC430_v01

//#include "oslib.h"



typedef enum{NAME=0,SPEED,PARITY,DATA,STOP,FLOWCTL,MAX}comdesc_t;

#ifdef DEBUG
#define debug(format, arg...) do{printf("%s: %d " format "\n" , __FUNCTION__ ,__LINE__, ## arg); } while (0)
#else
#define debug(format,arg...) 
#endif

static int speed_arr[] = { B460800,B230400, B115200,B38400, B19200, B9600, B4800, B2400, B1200, B300,
					B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static int name_arr[] = {460800,230400, 115200,38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,  
					19200,  9600, 4800, 2400, 1200,  300, };
static int print_fd = 0;
static const char * szComDesc = "/dev/s3c2410_serial0,115200,N,8,1,1";

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

static int Os__serialprint_Open(void)
{
	comdesc_t i;
	int fpga_fd;	
	
	if((fpga_fd = open(FPGA_DEV,O_RDWR)) < 0 ||\
		ioctl(fpga_fd,USE_Serial_PRINTER) < 0){
		debug("FPGA CONTROL ERROR!\n");
		return -2;
	}else
		close(fpga_fd);	

	print_fd = serial_open(szComDesc);
	if(print_fd < 0)
		return -1;	
	
	return 0;
}

static void Os__serialprint_Close(void)
{
	if(print_fd > 0){
		close(print_fd);
		print_fd = -1;
	}	
	return;
}
/*
*print_Control:????ӡ?????Ϳ???????
*@cmd:????????,??16???Ƶ?2λ?????ɵ??ַ???
*??ʽ???룬????2λ????0????,
* 0x1b 56 n(n= 00??01)
*return_val:0--?ɹ???-1 --????;
*/
static int Os__serialprint_Control(char *cmd)
{
	char buf[64],*p;
	int buf_len,i,ret;
	char tmp,tmpL,tmpH;	
		
	if(Os__serialprint_Open() < 0)		
		return -1;
	p = buf;
	while(*cmd){
		if( *cmd == ' ')
			cmd++;
		else if( *cmd == '0' && (*(cmd+1) == 'X'||\
				*(cmd+1) == 'x')){
			cmd += 2;
		}else			
			*p++ = *cmd++;
	}
	*p = 0;
	if((buf_len=strlen(buf))%2){
		debug("CMD input error!\n");
		ret = -1;
		goto error;
	}
	for(i = 0,p =buf;i< buf_len;i += 2){
		if( buf[i] >= '0' && buf[i] <= '9')
			tmpH = buf[i] - '0';
		else if(buf[i] >= 'a' && buf[i] <= 'f')
			tmpH = buf[i] - 'a' + 10;
		else if(buf[i] >= 'A' && buf[i] <= 'F')
			tmpH = buf[i] - 'A' + 10;

		if( buf[i+1] >= '0' && buf[i+1] <= '9')
			tmpL = buf[i+1] - '0';
		else if(buf[i+1] >= 'a' && buf[i+1] <= 'f')
			tmpL= buf[i+1] - 'a' + 10;
		else if(buf[i+1] >= 'A' && buf[i+1] <= 'F')
			tmpL = buf[i+1] - 'A' + 10;
		tmp = tmpH<<4|tmpL;
		*p++ = tmp;
	}
	
	ret = write(print_fd,buf,buf_len/2);
	if(ret == buf_len/2)
		ret = 0;
	
error:
	Os__serialprint_Close();
	return ret;
}

static int Os__serialprint_Init(void)
{
	char cmd[4];
	char value = 0;
	int ret;
	char i;
	
	if(Os__serialprint_Open() < 0)		
		return -1;
	cmd[0] = 0x1b;
	cmd[1] = 0x40;
	ret = write(print_fd,cmd,2);
	cmd[1] = 0x53;
	ret = write(print_fd,cmd,2);
	cmd[0] = 0x1d;
	cmd[1] = 0x45;
	cmd[2] = 0x0;
	ret = write(print_fd,cmd ,3);
	cmd[0] = 0x10;
	cmd[1] = 0x04;
	cmd[2] = 0x01;
	ret = write(print_fd,cmd,3);
	ret = read(print_fd,&value,1);
	if(ret !=1 || value != 0x16 ){
		ret = -0x1e;
		debug("Print offline:value %02x\n",value);
		goto out;
	}
	for(i = 2;i < 5;i++){
		cmd[2] = i;
		ret = write(print_fd,cmd,3);
		ret = read(print_fd,&value,1);
		if(ret != 1 || value != 0x12){
			ret = -value;
			break;
		}		
	}

out:if(i == 5 && value == 0x12)
		ret = 0;
	Os__serialprint_Close();
	return ret;
}

static int Os__serialprint_Cut(void)
{
	int ret;
	char cmd1[] = {0x1d,0x56,0x01};
	char cmd2[] = {0x0d,0x0a};
	if(Os__serialprint_Open() < 0)		
		return -1;
	ret = write(print_fd,cmd1,3);
	usleep(10000);
	ret = write(print_fd,cmd2,2);
	Os__serialprint_Close();
	return 0;
}

static int Os__serialprint_Feed(int line)
{
	int ret,i;
	char cmd[] = {0x0d,0x0a};
	if(Os__serialprint_Open() < 0)		
		return -1;
	for(i=0;i < line;i++){
		ret = write(print_fd,cmd,2);
	}
	Os__serialprint_Close();
	return 0;
}

static int Os__serialprint_Print(char * szData,int dataLen)
{
	int ret,i;
	char nl = 0x0d;
	if(Os__serialprint_Open() < 0)		
		return -1;
	if(!szData){
		ret = -2;
		goto error;
	}
		
	ret = write(print_fd,szData,dataLen);	
	ret = write(print_fd,&nl,1);
		
error:	
	Os__serialprint_Close();
	return 0;
}

int Os__xlinefeed(unsigned int line)
{	
  
    return Os__serialprint_Feed(line);	
}

unsigned char Os__xprint (unsigned char *data)
{
	  Uart_Printf("\nun cmple\n");
	  return 0;
}

int  Os__linefeed (unsigned int line)
{
	  Uart_Printf("\nun cmple\n");
		return 0;
}

int Os__graph_xprint (unsigned char * pixel_line, unsigned char size)
{
	 	Uart_Printf("\nun cmple\n");
		return 0;
}


int Os__sign_xprint(unsigned char *ucPath)
{
	 	Uart_Printf("\nun cmple\n");
		return 0;
}


int  Os__fontmode (fmode_t fmode)
{
		Uart_Printf("\nun cmple\n");
		return 0;
}

int  gb2312_16x16(unsigned char *chinese,unsigned char *dotdat)
{
		Uart_Printf("\nun cmple\n");
		return 0;
}
int OSPRN_XPrintGB2312(unsigned char ucASCIIFont,
											 unsigned char ucGBFont, 
											 unsigned char * Text)
{
		Uart_Printf("\nun cmple\n");
		return 0;
}

DRVOUT *OSPRN_PrintASCII(unsigned char *ucFont, unsigned char *Text)
{
		Uart_Printf("\nun cmple\n");
		return 0;
}

DRV_OUT *Os__print(unsigned char *data)
{
		Uart_Printf("\nun cmple\n");
		return 0;	
}

int Os__checkpaper()
{
	char value;
	char cmd[3];
	int ret;
	if(Os__serialprint_Open() < 0)		
		return -1;
	cmd[0] = 0x10;
	cmd[1] = 0x04;
	cmd[2] = 0x04;
	ret = write(print_fd,cmd,sizeof(cmd));
	if(ret != sizeof(cmd)){
		ret = -2;
		goto out;
	}
	ret = read(print_fd,&value,sizeof(value));
//	printf("\nvalue %02x\n",value);
	if(ret != sizeof(value)){
		ret = -3;
		goto out;
	}
	if(value == 0x12)
		ret = 0;
	else
		ret = 1;
out:
	Os__serialprint_Close();
		return ret;
}

unsigned char Os__GB2312_xprint(unsigned char * pdata, unsigned char size)
{
	int datalen = strlen(pdata);
	char ret;
	/*
	switch(size){
		case 0x1c:
		Os__serialprint_Control("0x1b5502");	
		break;
		case 0x1d:
		Os__serialprint_Control("0x1b5602");
		break;
		case 0x1e:
		Os__serialprint_Control("0x1b5702");
		break;
		default:
		Os__serialprint_Control("0x1b5701");
		break;		
	}
	*/
	Os__serialprint_Control("0x1b5701");
	ret = Os__serialprint_Print(pdata,datalen);
	
	return ret;
	
	
}

int Os__papercut(void)
{
	return Os__serialprint_Cut();
}
#else

static unsigned char  prnBuf[2400][48];/*100 24x24 */
static unsigned char zhbf[72],enbf[72];

extern unsigned char FONT_ASCII1224[][48];
extern unsigned char FONT_ASCII816_2[][16];
extern unsigned char FONT_1616[];

typedef int (*prnfunc_t)(unsigned char *,unsigned char *);

#define  get_byte_bit(uc8,bit)				(((uc8)>>(bit)) & 0x01)

static void Modify24Dot(unsigned char *DotDataBuf)
{
    unsigned int dotbuf[24];
    unsigned int Moddotbuf[24];
    unsigned char i,j;
    for (i=0; i<24; i++)
    {
        Moddotbuf[i] = 0;
        dotbuf[i] = 0;
    }
    for (i=0; i<24; i++)
    {
        dotbuf[i] += *(DotDataBuf+3*i);
        dotbuf[i] <<= 8;
        dotbuf[i] += *(DotDataBuf+3*i+1);
        dotbuf[i] <<= 8;
        dotbuf[i] += *(DotDataBuf+3*i+2);
    }

    for (j=0; j<24; j++)
    {
        for (i=j; i<24; i++)
            Moddotbuf[j] |= (dotbuf[i] & (0x00800000>>j))>>(i-j);
        for (i=0; i<j; i++)
            Moddotbuf[j] |= (dotbuf[i] & (0x00800000>>j))<<(j-i);
    }

    for (i=0; i<24; i++)
    {
        *(DotDataBuf+3*i+2) = (Moddotbuf[i] & 0xff);
        *(DotDataBuf+3*i+1) = ((Moddotbuf[i]>>8) & 0xff);
        *(DotDataBuf+3*i) = ((Moddotbuf[i]>>16) & 0xff);
    }
}

int  gb2312_24x24(unsigned char *chinese,unsigned char *dotdat)
{
    unsigned char qm,wm;
    unsigned long offset;
    FILE *gb2312;

//   if((gb2312=fopen("xx.bin","rb"))==NULL){
    if ((gb2312=fopen("/fonts/GB2312_24x24.bin","rb"))==NULL)
    {
        Uart_Printf("Can't open GB2312_24x24.bin,Please add it?");
        return -1;
    }

    qm  = *(chinese) - 0xa1;
    wm  = *(chinese+1)   - 0xa1;

    offset = ( (qm-15)*94 + wm )*72L;
//   Uart_Printf ("qm:%d,wm:%d\n",qm,wm);
    fseek(gb2312,offset,SEEK_SET);
    fread(dotdat,72,1,gb2312);
    fclose(gb2312);
    Modify24Dot(dotdat);
    return 0;
}

int  gb2312_16x16(unsigned char *chinese,unsigned char *dotdat)
{
    unsigned char qm,wm;
    unsigned int offset;
    FILE *gb2312;

    qm  = *(chinese);
    wm  = *(chinese+1);
    if ((qm < 0x80)||(wm < 0x40)) return 0;
    if ((qm >= 0xB0)&&(wm >= 0xA1))
    {
        qm = qm - 0xB0;
        wm = wm - 0xA1;
        offset = qm * 94 + wm;
    }
    else if (qm <= 0xA0)
    {
        qm = qm - 0x81;
        wm = wm - 0x40;
        offset = qm * 190 + wm + 6768;
        if (wm > 0x3E) offset --;
    }
    else if ((qm >= 0xAA)&&(wm <= 0xA0))
    {
        qm = qm - 0xAA;
        wm = wm - 0x40;
        offset = qm * 96 + wm + 12847;
    } 
   
    
    offset *= 32;

    if ((gb2312=fopen("/fonts/jf3s1516.hz","rb"))==NULL)
    {
        Uart_Printf("Can't open /fonts/jf3s1516.hz,Please add it?");
        return -1;
    }
//	Uart_Printf("qm:%#x(%#x),wm:%#x(%#x),offset:%#lx \n",qm,*(chinese),wm,*(chinese+1),offset);
    fseek(gb2312,offset,SEEK_SET);
    fread(dotdat,32,1,gb2312);
    fclose(gb2312);
//	for (i=0;i<32;i++) Uart_Printf("%#x ",dotdat[i]); Uart_Printf("\n");
    return 0;
}

int ascii12x24(unsigned char *ch,unsigned char *dotdat)
{
    memcpy(dotdat,FONT_ASCII1224[*ch - ' '],48);
// return 0;
}

int ascii8x16(unsigned char *ch, unsigned char *dotdat)
{
//   ch = ch - ' ';
    memcpy(dotdat,FONT_ASCII816_2[*ch],16);
//   return 0;
}

/*******************************************************
*                                                      *
*                 API                                  *
*                                                      *
********************************************************/


int openSS205(void)
{
    return open(DRVNAME, O_RDWR) ;
}

void closeSS205(int fd)
{
    close(fd);
}


int  Os__fontmode (fmode_t fmode)
{
    int  rtn = -1;
    int fd ;

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("function:%s open %s error.\n",__func__,DRVNAME) ;
        return -1 ;
    }
    rtn =  ioctl(fd,SS205_PRINT_FONT,&fmode) ;
    close(fd);
    return rtn ;
}

static OSPRNCFG defCfg =
{
    .uiVoltage = 50,
    .fmode = {
        .row = 24,
        .col = 24,
    },
};


/*1:no paper.*/
int Os__checkpaper()
{
    int  rtn = -1;
    int fd ;

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("function:%s open %s error.\n",__func__,DRVNAME) ;
        return -1 ;
    }
    rtn =  ioctl(fd,SS205_PRINT_CHECKPAPER,&fd) ;
    close(fd);
    return rtn ;
}


/*
* zy:
* newadd to clean print buf
* 20100324
*/
int Os__clear_printbuf()
{
    int  rtn = -1;
    int fd ;

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("function:%s open %s error.\n",__func__,DRVNAME) ;
        return -1 ;
    }
    rtn =  ioctl(fd,SS205_PRINT_END,&fd) ;
    close(fd);
    return rtn ;
}



int OSCFG_PrinterSettingLoad(OSPRNCFG *cfg)
{
    OSPRNCFG *prncfg=NULL;
    int  rtn = -1;
    int fd ;

    if (!cfg) prncfg = &defCfg;
    else prncfg = cfg;

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("OSCFG_PrinterSettingLoad open %s error.\n",DRVNAME) ;
        return -1 ;
    }
    rtn =  ioctl(fd,SS205_PRINT_CFGPARAM,&prncfg);
    close(fd);
    return rtn ;
}

/*
description : printer goes forward n_steps.
variable :
				n_steps: steps.
 return :
 				0: ok
 				-1: error.
*/
int Os__prn_forward (unsigned char n_steps)
{
    return 0 ;
}
/*
the same as Os__prn_xforward
*/
int Os__prn_xforward (unsigned char n_steps)
{

    return Os__prn_forward(n_steps);
}

/*
description : printer goes backword n_steps.
variable :
				n_steps: steps.
 return :
 				0: ok
 				-1: error.
*/

int Os__prn_backward (unsigned char n_steps)
{

    return 0 ;
}
/*
the same as Os__prn_backward
*/

int OSPRN_XReverseFeed (unsigned char n_steps)
{
    return Os__prn_backward(n_steps);
}

/*
description :printer feed paper n lines
variable :
				line:lines
Return :
				0: ok
			 -1: error
*/
int  Os__linefeed (unsigned int line)
{
    int  rtn = -1;
    int fd ;
    long timeout;
    unsigned char print_status;

    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("function:%s open %s error.\n",__func__,DRVNAME) ;
        return -1 ;
    }
    
    rtn =  ioctl(fd,SS205_FEED_PAPER,&line) ;
    close(fd);

    return rtn ;

}
/*
the same as Os__linefeed
*/

int Os__xlinefeed(unsigned int line)
{
    return Os__linefeed(line);
}

void colExbit(unsigned char ch,unsigned char rst[])
{
    int i=0;
    for (i=0; i<4; i++)
    {
        rst[0] |= (((ch>>i)&0x01)<<2*i)|(((ch>>i)&0x01)<<(2*i +1));
        rst[1] |= (((ch>>(4+i))&0x01)<<2*i)|(((ch>>(4+i))&0x01)<<(2*i +1));
    }
}

int _printer_state()
{
    int	tmp = 0;
    int	fd_prn = 0;

    fd_prn  = open("/dev/ss205",O_RDWR);
    if (fd_prn < 0)
        return (-1);

    if (ioctl(fd_prn, SS205_PRINT_CHKSTATUS, &tmp) < 0)
    {
        close(fd_prn);
        tmp = -1;
    }

    close(fd_prn);
    return tmp;

}


int ultimate_print(unsigned char ucASCIIFont,
                   unsigned char ucGBFont, unsigned char * Text,int dlen)
{

    int index,trsflags = 0; /*0:default,
                       1,twice col,
                       2,twice row
                       3,both twice col and row
                       4,both fourth col and row
                       */
    int en = 2; /*1: 8x16 2: 12x24(default)*/
    int zh = 2; /*1: 8x16 2: 12x24(default)*/
    int en_row,en_col,zh_row,zh_col;
    int high,x,y;
    int m,n;
    int fd,rtn;
	int val;
     int state=0;
  
   long timeout; 
    unsigned char print_status;
  
    unsigned char rst[2];

    prnfunc_t zh_func=gb2312_24x24,en_func=ascii12x24;
// unsigned char *zhbf,*enbf;
    if (!Text)
    {
        Uart_Printf("OSPRN_XPrintGB2312 Arg Error.");
        return -1;
    }
//  dlen = 48; while((dlen>=0)&&(Text[--dlen]==0x20));
//  dlen=strlen(Text);
//   Uart_Printf("dlen:%d\r\n",dlen);

    /*open device file.*/
    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("function:%s open %s error.\n",__func__,DRVNAME) ;
        return -1;
    }
#if 0
	//zy add
	val = 1;
	ioctl(fd, SS205_PRINT_SETVF, &val);
#endif

	
    /*default*/
//   zh_func = gb2312_24x24; en_func = ascii12x24;
    switch (ucGBFont)
    {
    case 0x01:/*zh:16X16 en:8*16*/
        trsflags = 0;
        en = 1;
        zh = 1;
        zh_func = gb2312_16x16;
        en_func = ascii8x16;
        break;
    case 0x1c:/*zh:16X16 en:8*16  (both twice COL)*/
        trsflags = 1;
        en = 1;
        zh = 1;
        zh_func = gb2312_16x16;
        en_func = ascii8x16;
        break;
    case 0x1d:/*zh:16X16 en:8*16  (both twice ROW)*/
        trsflags = 2;
        en = 1;
        zh = 1;
        zh_func = gb2312_16x16;
        en_func = ascii8x16;
        break;
    case 0x1e:/*zh:16X16 en:8*16  (both twice COL and ROW)*/
        trsflags = 3;
        en = 1;
        zh = 1;
        zh_func = gb2312_16x16;
        en_func = ascii8x16;
        break;
    case 0x1b:/*zh:16X24 (fourth col and row)*/
        trsflags = 6;
        if (ucASCIIFont ==0x31)
        {
            en = 1;
            en_func = ascii8x16;
        }
        else
        {
            en = 2;
            en_func = ascii12x24;
        }
        //		 zh = 2;
        zh_func = gb2312_24x24;
        break;
    case 0x0c:/*zh:16X24 (twice col)*/
        trsflags = 1;
        if (ucASCIIFont ==0x31)
        {
            en = 1;
            en_func = ascii8x16;
        }
        else
        {
            en = 2;
            en_func = ascii12x24;
        }
        //		  zh = 2;
        zh_func = gb2312_24x24;
        break;
    case 0x0d:/*zh:16X24 (twice  row)*/
        trsflags = 2;
        if (ucASCIIFont ==0x31)
        {
            en = 1;
            en_func = ascii8x16;
        }
        else
        {
            en = 2;
            en_func = ascii12x24;
        }
        //		  zh = 2;
        zh_func = gb2312_24x24;
        break;
    case 0x0e:/*zh:16X24 (twice col and row)*/
        trsflags = 3;
        if (ucASCIIFont ==0x31)
        {
            en = 1;
            en_func = ascii8x16;
        }
        else
        {
            en = 2;
            en_func = ascii12x24;
        }
        //		  zh = 2;
        zh_func = gb2312_24x24;
        break;
    default:  /*zh:16X24*/
        //		  trsflags = 0;en = 2;zh = 2;
        zh_func = gb2312_24x24;
        en_func = ascii12x24;
        break;
    }

    if (zh == 2)
    {
        zh_row=24;
        zh_col=3;
    }
    else
    {
        zh_row=16;
        zh_col=2;
    }
    if (en == 2)
    {
        en_row=24;
        en_col=2;
    }
    else
    {
        en_row=16;
        en_col=1;
    }

    high = zh_row>en_row?zh_row:en_row;
    x=0;
    y=0;
    memset((unsigned char *)prnBuf,0,2400*48);

//    Uart_Printf("dlen:%d,zh:%d(%d,%d),en:%d(%d,%d),high:%d\n",
//	        dlen,zh,zh_col,zh_row,en,en_col,en_row,high);

    for (index = 0; index<dlen;)   /*number of ch.*/
    {
   
   /*
				if(Text[index]==0xA3)
				{
						iText[index]=Text[index]ndex++;
						-0xA0+0x20;
						goto en_f;
				}
    */	
        if (Text[index]>=0x80)   /*zh*/
        {
            switch (trsflags)
            {
            case 4:   /*both fourth col and row*/
            {
                break;
                x+=4*zh_col;
            }
            case 3:   /*both twice col and row*/
            {
                if (x>48-2*zh_col)
                {
                    x=0;    /*48-6*/
                    y += 2;
                }
                memset((unsigned char *)zhbf,0,zh_row*zh_col);
               // zh_func(&Text[index],zhbf);

								 if(Text[index]==0xA3)
                {	
                
									Text[index+1]=Text[index+1]-0xA0;
                	memcpy(zhbf,&FONT_1616[Text[index+1]*32],32);
              	}
                else 
                	zh_func(&Text[index],zhbf);
								
                for (m = 0; m<zh_row; m++)
                    for (n = 0; n<zh_col; n++)
                    {
                        memset(rst,0,2);
                        colExbit(zhbf[m*zh_col+n],rst);

                        prnBuf[high*y+2*m][x+2*n] = rst[1];
                        prnBuf[high*y+2*m][x+2*n+1] = rst[0];

                        prnBuf[high*y+2*m+1][x+2*n] = rst[1];
                        prnBuf[high*y+2*m+1][x+2*n+1] = rst[0];
                    }

                x+=2*zh_col;
                break;
            }
            case 2:   /*twice row*/
            {
                if (x>48-zh_col)
                {
                    x=0;    /*48-3*/
                    y += 2;
                }
                memset((unsigned char *)zhbf,0,zh_row*zh_col);
                
                //zh_func(&Text[index],zhbf);
                 if(Text[index]==0xA3)
                {	
                	Text[index+1]=Text[index+1]-0xA0;
                	memcpy(zhbf,&FONT_1616[Text[index+1]*32],32);
              	}
                else 
                	zh_func(&Text[index],zhbf);
                
                
                for (m = 0; m<zh_row; m++)
                    for (n = 0; n<zh_col; n++)
                    {
                        prnBuf[high*y+2*m][x+n] = zhbf[m*zh_col+n];
                        prnBuf[high*y+2*m+1][x+n] = zhbf[m*zh_col+n];
                    }
                x+=zh_col;
                break;
            }
            case 1:   /*twice col*/
            {
                if (x>48-2*zh_col)
                {
                    x=0;    /*48-6*/
                    y++;
                }
                memset((unsigned char *)zhbf,0,zh_row*zh_col);
               // zh_func(&Text[index],zhbf);

								 if(Text[index]==0xA3)
                {	
                	Text[index+1]=Text[index+1]-0xA0;
                	memcpy(zhbf,&FONT_1616[Text[index+1]*32],32);
              	}
                else 
                	zh_func(&Text[index],zhbf);

                for (m = 0; m<zh_row; m++)
                    for (n = 0; n<zh_col; n++)
                    {
                        memset(rst,0,2);
                        colExbit(zhbf[m*zh_col+n],rst);
                        prnBuf[high*y+m][x+2*n] = rst[1];
                        prnBuf[high*y+m][x+2*n+1] = rst[0];
                    }
                x+=2*zh_col;
                break;
            }
            case 0:/**/
            default:
            {
                if (x>48-zh_col)
                {
                    x=0;    /*48-3*/
                    y++;
                }
                memset((unsigned char *)zhbf,0,zh_row*zh_col);
                if(Text[index]==0xA3)
                {	
                	Text[index+1]=Text[index+1]-0xA0;
                	memcpy(zhbf,&FONT_1616[Text[index+1]*32],32);
              	}
                else 
                	zh_func(&Text[index],zhbf);
                	
                		
                for (m = 0; m<zh_row; m++)
                    for (n = 0; n<zh_col; n++)
                        prnBuf[high*y+m][x+n] = zhbf[m*zh_col+n];
                x+=zh_col;
                break;
            }
            }

            index+=2;

        }
        else if (Text[index]>=0x20)   /*en*/
        {
en_f:
            switch (trsflags)
            {
            case 4:   /*both fourth col and row*/
            {
                break;
            }
            case 3:   /*both twice col and row*/
            {
                if (x>48-2*en_col)
                {
                    x=0;    /*48-2*/
                    y += 2;
                }
                memset((unsigned char *)enbf,0,en_row*en_col);
                en_func(&Text[index],enbf);
                for (m = 0; m<en_row; m++)
                    for (n = 0; n<en_col; n++)
                    {
                        memset(rst,0,2);
                        colExbit(enbf[m*en_col+n],rst);
                        prnBuf[high*y+2*m][x+2*n] = rst[1];
                        prnBuf[high*y+2*m][x+2*n+1] = rst[0];
                        prnBuf[high*y+2*m+1][x+2*n] = rst[1];
                        prnBuf[high*y+2*m+1][x+2*n+1] = rst[0];
                    }
                x+= 2*en_col;
                break;
            }
            case 2:   /*twice row*/
            {
                if (x>48-en_col)
                {
                    x=0;    /*48-2*/
                    y += 2;
                }
                memset((unsigned char *)enbf,0,en_row*en_col);

                en_func(&Text[index],enbf);
                for (m = 0; m<en_row; m++)
                    for (n = 0; n<en_col; n++)
                    {
                        prnBuf[high*y+2*m][x+n] = enbf[m*en_col+n];
                        prnBuf[high*y+2*m+1][x+n] = enbf[m*en_col+n];
                    }
                x+=en_col;
                break;
            }
            case 1:   /*twice col*/
            {
                if (x>48-2*en_col)
                {
                    x=0;    /*48-2*/
                    y++;
                }
                memset((unsigned char *)enbf,0,en_row*en_col);
                en_func(&Text[index],enbf);
                for (m = 0; m<en_row; m++)
                    for (n = 0; n<en_col; n++)
                    {
                        memset(rst,0,2);
                        colExbit(enbf[m*en_col+n],rst);
                        prnBuf[high*y+m][x+2*n] = rst[1];
                        prnBuf[high*y+m][x+2*n+1] = rst[0];
                    }
                x+= 2*en_col;
                break;
            }
            case 0:/**/
            default:
            {
                if (x>48-en_col)
                {
                    x=0;    /*48-2*/
                    y++;
                }
                memset((unsigned char *)enbf,0,en_row*en_col);
                en_func(&Text[index],enbf);
                for (m = 0; m<en_row; m++)
                    for (n = 0; n<en_col; n++)
                        prnBuf[high*y+m][x+n] = enbf[m*en_col+n];
                x+=en_col;
                break;
            }
            }

            index++;
        }
        else   /*ctl*/
        {
            // if (x>48-en_col){ x=0;y++;}  /*48-2*/
            index++;
            x+=en_col;
        }
        //	 if (x >= 48) {y++; x = 0;}
        
        if (x >= 48)
        {
           break;
           /*
            x = 0;
            switch (trsflags)
            {
            case 4://both fourth col and row/
                y +=4;
                break;
            case 3://both twice col and row/
                y += 2;
                break;
            case 2://twice row/
                y += 2;
                break;
            case 1://twice col/
            case 0:
            default:
                y++;
                break;
            }*/
            
        }
    }


 
    if (x != 0)   //half of line -> a line
    {
        switch (trsflags)
        {
        case 4://both fourth col and row/
            y +=4;
            break;
        case 3://both twice col and row/
            y +=2;
            break;
        case 2://twice row/
            y += 2;
            break;
        case 1://twice col/
        case 0:
        default:
            y++;
            break;
        }
    }
    
    
    /*
    * = 0 successfully
    * = 1 lack of paper
    * = 2 printer hot
    * = 4 fifo overflow
    */ 
    
  //  printf("\n[-%d-%d-%s]\n\n",y,high,prnBuf);
    
    rtn = write(fd,prnBuf,y*high*48); //Uart_Printf("x:%d,y:%d,len:%d,ret:%d\n",x,y,y*24*48,rtn);
   
    if(rtn<0)
    {     
      //0=OK   1=no paper   2=hot   3=no paper & hot
      state=_printer_state();
      if(state>0)
      {
       // Os__clr_display(255);
       // if(state==1)
	//		    Os__GB2312_display(0,0,(unsigned char *)"???Ŵ?ӡֽ");  
      //  if(state==2)
        //  Os__GB2312_display(0,0,(unsigned char *)"??ӡ??????"); 
       // if(state==3)
        //  Os__GB2312_display(0,0,(unsigned char *)"??ӡ?????Ȼ???ֽ");   
        while(state!=0)  
        {          
          sleep(1);
          state=_printer_state();
        }
        rtn = write(fd,prnBuf,y*high*48);
      }
    }
    //Os__GB2312_display(0,0,(unsigned char *)"???ڴ?ӡ     ");
    
    close(fd);

    if (rtn < 0) {
        printf("%s:%d-- write result = %d \n",__FUNCTION__,__LINE__,rtn);
		printf("%s:%d-- errno = %d \n",__func__,__LINE__,errno);
        return (errno); //KO
    } else
        return 0; // OK
}

int OSPRN_XPrintGB2312(unsigned char ucASCIIFont,
                       unsigned char ucGBFont, unsigned char * Text)
{
    return ultimate_print(ucASCIIFont,ucGBFont,Text,strlen(Text));
}

DRV_OUT *Os__print(unsigned char *data)
{
    ultimate_print(0,0,data,strlen(data));
    return NULL;
}
/*
The same as Os__print.
*/

unsigned char Os__xprint (unsigned char *data)
{
    ultimate_print(0,0,data,strlen(data));
    return strlen(data);
}
/*
 description: printting chinese in terminal graph.
 variable :
 			pdata :data
 			size :the length of data.
 return :
 				0: ok
 				-1: error.
*/
/********************************************************************
函数： uchar Os__GB2312_xprint (uchar * Text, uchar Size) ;
等待： 是
说明： 在终端上打印一串中文信息，打印应为国标码。
     'Size' 打印字符串长度。
打印格式： 0: 普通
         0x1C: 双倍宽度
         0x1D: 双倍高度
         0x1E: 双倍尺寸（双倍高度及双倍宽度）
　　     0x1B: 等比尺寸（等高等宽）
返回值： OK（成功）/KO（失败）.
********************************************************************* 
*/ 
unsigned char Os__GB2312_xprint(unsigned char * pdata, unsigned char size)
{
    int retval ;

    switch (size)
    {
    case 0x00:
        retval = ultimate_print(0,0,pdata,strlen(pdata));
        break;
    case 0x01:
        retval = ultimate_print(0,0x01,pdata,strlen(pdata));
        break;
    case 0x1c:
        retval = ultimate_print(0,0x1c,pdata,strlen(pdata));
        break;
    case 0x1d:
        retval = ultimate_print(0,0x1d,pdata,strlen(pdata));
        break;
    case 0x1e:
        retval = ultimate_print(0,0x1e,pdata,strlen(pdata));
        break;
    default:
        retval = ultimate_print(0,0,pdata,strlen(pdata));
        break;
    }

    return retval;
}


/*
description: print acsii according to font.
variable:
          ucFont:font
          Text: It will be printting Text.
return:

*/
DRVOUT *OSPRN_PrintASCII(unsigned char *ucFont, unsigned char *Text)
{
    switch (*ucFont)
    {
    case 0x00:/*16X24*/
        // OSPRN_XPrintGB2312(0,0,Text);
        ultimate_print(0,0,Text,strlen(Text));
        break;
    case 0x30:/*5X7*/
        break;
    case 0x35:/*8X16*/
        //OSPRN_XPrintGB2312(0,0x01,Text);
        ultimate_print(0,0x01,Text,strlen(Text));
        break;
    default:
        break;
    }
}
unsigned char OSPRN_XPrintASCII(unsigned char ucFont,unsigned char *pucPtr)
{
    OSPRN_PrintASCII(&ucFont, pucPtr);
    return 0;
}

/*
description : printting acsii string in GB2312 way.
variable :
			ptAscii: ascii string.
 return :
 				0: ok
 				-1: error.
*/
unsigned char *Os__conv_ascii_GB2312(unsigned char *ptAscii)
{
    Uart_Printf("Os__conv_ascii_GB2312 not support ! \n");

    return ptAscii;
}


/* new add, zhangy */
unsigned char *Os__conv_utf8_GB2312(unsigned char *ptUTF8)
{
    //Uart_Printf("%s not support ! \n", __func__);
    return NULL;
}


/*
description : printting graph.
variable :
				pixel_line :the print pixel
				size : length of pixel_line.
 return :
 				0: ok
 				-1: error.
*/


static unsigned char garray[48];

/***************************************************************
作用： 在打印纸上打印图形
函数： uchar Os__graph_xprint (uchar * PixelLine, uchar Size) ;
等待： 是
说明： 打印一行PixelLine定义的行像素
      'PixelLine' 行像素定义指针
　　   黑色像素为 1
      白色像素为 0
      像素由8bit构成一个byte,第一个byte是左边有效，第二个byte是右边有效。
      'Size'是行定义尺寸
      图形行像素最大是384像素，所以最大PixLine是48个byte
返回值：OK（成功）/KO（失败）.
***************************************************************** 
*/ 

int Os__graph_xprint (unsigned char * pixel_line, unsigned char size)
{
    int fd,rt;
    /*open device file.*/
    fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0 )
    {
        Uart_Printf ("function:%s open %s error.\n",__func__,DRVNAME) ;
        return 1;
    }
    /*
    if (size==48)
        rt = write(fd,pixel_line,size);
    else if (size<48)
    {
        memset (garray,0,48);
        memcpy(garray,pixel_line,size);
        rt = write(fd,garray,48);
    }
    else
    {
        rt =-1;
    } 
    */
    rt = write(fd,pixel_line,size);

    close(fd);

    /* liugm 1/24/2011 */
    if (rt < 0) {
        printf("%s:%d-- write result = %d \n",__FUNCTION__,__LINE__,rt);
        return (errno); // return KO,
    } else
        return 0; //OK
}

///////////// for sign //////////////////////////////////////////////
int	dataLength;
unsigned char ucBmpData[SIGN_MAXBMP];
unsigned char ucPrintData[320][48];

//??ͼƬ 0-?ɹ? -1-ʧ??
int SING_BMP_read(unsigned char *ucPath)
{
    int fd,ret,ip=0,iheight=0;
    unsigned char ucBuf[SIGN_MAXBMP];

    dataLength=0;
    memset(ucBmpData,0,SIGN_MAXBMP);
    memset(ucBuf,0,SIGN_MAXBMP);
    fd=open(ucPath,O_RDONLY );
    if (fd<0)
        return -1;
    ret=read(fd,ucBuf,sizeof(ucBuf));

    //?ж?bmp?Ƿ񳬹?20k
    if (ret>SIGN_MAXBMP)
        ret=-2;

    //?ж??Ƿ???bmp?ļ?
    if (ret>0)
        if (ucBuf[0]!=0x42||ucBuf[1]!=0x4d)
            ret=-3;

    //?ж??Ƿ??ǵ?ɫͼ
    if (ret>0)
        if (ucBuf[28]!=0x01||ucBuf[29]!=0x00)
            ret=-4;

    //?жϿ????Ƿ???384
    if (ret>0)
        if (ucBuf[18]!=0x80||ucBuf[19]!=0x01)
            ret=-5;
	
    //?жȸ߶??Ƿ?????320
    if (ret>0)
    {
       iheight+=ucBuf[22];
	iheight+=ucBuf[23]<<8;
	iheight+=ucBuf[24]<<16;
	iheight+=ucBuf[25]<<24;
        if (iheight>320)
       	ret=-6;
    }
		
    //ȡƫ??��
    if (ret>0)
    {
        ip+=ucBuf[10];
        ip+=ucBuf[11]<<8;
        ip+=ucBuf[11]<<16;
        ip+=ucBuf[12]<<24;
        dataLength=ret-ip;
        memcpy(ucBmpData,&ucBuf[ip],dataLength);
        ret=0;
    }
    close(fd);
    return ret;
}

//??װ????ӡ????
void SIGN_BMG_getPrintData(void)
{
    int line=dataLength/48;
    int i=0,j=0,p;

    memset(ucPrintData,0,sizeof(ucPrintData));

    for (i=0; i<line; i++)
        for (j=0; j<48; j++)
        {
            p=(line-i-1)*48;
            ucPrintData[i][j]=~ucBmpData[p+j]&0xff;
        }
}

int Os__sign_print(void)
{
	return Os__sign_xprint("/tmp/sign.bmp");
}

/***********************************************************
* = 0 successfully
* = 1 lack of paper
* = 2 printer hot
* = 4 fifo overflow
************************************************************ 
*/ 
int Os__sign_xprint(unsigned char *ucPath)
{
		unsigned char ucBuf[49];
		unsigned char ucTmp[SIGN_MAXBMP];
    int ret=-1, i,j;

    ret = SING_BMP_read(ucPath);
    if (ret < 0) {
        return ret;
    }

    if (ret==0)
        SIGN_BMG_getPrintData();

#if 0
    if (ret==0)
    {
        for (i = 0; i < dataLength/48; i++)
 						Os__graph_xprint((unsigned char *)ucPrintData[i],48);
		}
#endif

    int fd = open(DRVNAME, O_RDWR) ;
    if (fd < 0) {
        printf("%s:%d--can't open file \n",__FUNCTION__,__LINE__);
        ret = -1;
        goto exit;
    }

  	memset(ucTmp,0,sizeof(ucTmp));
  	
  
  	for (i = 0; i < dataLength/48; i++)
 			memcpy(&ucTmp[i*48],&ucPrintData[i],48);
 
  	ret = write(fd,ucTmp,dataLength);
       
    if (ret < 0) {
        printf("%s:%d-- write result = %d \n",__func__,__LINE__,ret);
		printf("%s:%d-- errno = %d \n",__func__,__LINE__,errno);

        ret = errno; // return KO,
        goto exit;
    }
    ret = 0;

exit:
    close(fd);
    return ret;
}


/////////// end of sign ////////////////////////////////////////


unsigned char OSPRN_BeginPrint(unsigned char *pucParam)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
    return 0;
}
unsigned char OSPRN_EndPrint(void)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
    return 0;
}

unsigned char OSPRN_CheckPaper(unsigned char ucFlag)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
    return 0;
}
DRV_OUT* OSPRN_Feed(unsigned char ucLineNB)
{
    return;
}

unsigned char OSPRN_XFeed(unsigned char ucLineNB)
{
    Uart_Printf("%s:not implement!!!\n",__func__);
    return 0;
}

/**************************************************************** 
 * Function :  Os_printer_idle
 * Version: 1
 * Description: detect printer is idle
 * Author: liugm
 * Edit Date: 3/17/2011 11:37a
 ****************************************************************
 */


int Os__printer_idle()
{
    int	tmp = 0;
    int	fd_prn = 0;

    fd_prn  = open("/dev/ss205",O_RDWR);
    if (fd_prn < 0)
    {
        printf("[%s:%d]--open error \n",__func__,__LINE__);
        return (-1);
    }

    if (ioctl(fd_prn, SS205_PRINT_END, &tmp) < 0)
    {
        close(fd_prn);
        printf("[%s:%d]--ioctl error \n",__func__,__LINE__);
        tmp = -1;
    }
out:
    close(fd_prn);
    return tmp;
}


#endif
