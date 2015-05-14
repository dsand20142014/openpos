#ifndef __MAIN_H
#define __MAIN_H

#include "sand_main.h"

#define MALLOC(len)         malloc(len)
#define FREE(p)             free(p)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define APP_HIDE            1
#define APP_DISPLAY         0

#define FILE_UPDATE_PC          0x10
#define FILE_OVERWRITE_PC				0x11
#define FILE_SAVE_PC						0x12
#define FILE_DEL_PC						  0x13

#define FILE_UPDATE_POS         0x20
#define FILE_OVERWRITE_POS			0x21
#define FILE_SAVE_POS						0x22
#define FILE_DEL_POS						0x23

#define APP_MAX_CNT         40
#define APP_MAX_NAME_LEN    32
#define APP_MAX_LABLE_LEN   32
#define APP_SHM_KEY         1100
#define APP_MAS_QUE_KEY     1200

#define APP_PATH          "/app/"//"/mnt/pos/app/"
//#define DATA_PATH					"/mnt/pos/data/"
#define LIB_PATH					"/lib/"
#define DRIVER_PATH				"/driver/"
#define FONT_PATH					"/fonts/"

#define LINUXPOS_INFO_FILE	CONFIG_PATH"linuxpos.dat"
#define APP_INFO_FILE     	CONFIG_PATH"appinfo.dat"
#define FONT_INFO_FILE			CONFIG_PATH"font.dat"
#define LIB_INFO_FILE				CONFIG_PATH"lib.dat"
#define DRIVER_INFO_FILE		CONFIG_PATH"driver.dat"

#define KEY_INFO_FILE     	CONFIG_PATH"keyinfo.dat"
#define ZK_GB2312_LCD     	CONFIG_PATH"gb2312.dat"
#define ZK_GB2312_PRN     	CONFIG_PATH"gb2312prn.dat"


#define APP_MQ_FLAG         (0777|IPC_CREAT)
#define APP_MSG_TYPE        100
#define APP_SHM_SIZE        8096

#define OSEVENT_PROCESSED   0
#define OSEVENT_SAVEDATA	      0
#define OSEVENT_GETDATA         1
#define OSEVENT_OUT		  	      2
#define NOK     1         


/* key */
#define KEY_ENTER       0x3A
#define KEY_CLEAR       0x3B
#define KEY_00_PT       0x3C
#define KEY_BCKSP       0x3D
#define KEY_F1          0x40
#define KEY_F2          0x41
#define KEY_F3          0x42
#define KEY_F4          0x43
#define KEY_F5          0x44
#define KEY_F6          0x45
#define KEY_F7          0x46
#define KEY_F8          0x47
#define KEY_F9          0x48
#define KEY_F           0x45
#define KEY_R           0x46
#define KEY_SYS         0x3F
#define KEY_PAPER_FEED  0x3E
#define KEY_PAPEUP      0x44
#define KEY_PAPEDN      0x4B
#define KEY_F           0x45    /*=KEY_F6*/
#define KEY_R           0x46    /*=KEY_F7*/
#define KEY_UP          0x50    /*上页*/
#define KEY_DOWN        0x51    /*下页*/

/* display*/
#define ASCIIFONT57     	0x30
#define ASCIIFONT816    	0x31
#define ASCIIFONT612		0x32
#define ASCIIFONT1224		0x33
#define ASCIIFONT78	    	0x35
#define ARABICFONT16		0x34
#define ARABICFONT216	   	0x35

#define APP_STANDBY			1000  // 10s
#define POWER_OFF_CNT       6
#define LIGHT_OFF_VALUE     g_cfg_pow.light_off_tm *100
#define LIGHT_OFF_TM        ((LIGHT_OFF_VALUE > 0) ? LIGHT_OFF_VALUE : APP_STANDBY)



#define lcd_clear(line)     OSMMI_ClrDisplay(0x30,line)
#define lcd_display(x,y,s)  OSMMI_DisplayASCII(0x30,x,y,(unsigned char*)s)

/* define debug format */
#define __printf(fmt,args...)  \
do {\
	printf("[%s: %d at %s]---"fmt,__FILE__,__LINE__,__FUNCTION__,##args);\
}while(0)

#define WIDGET_LABEL	0
#define WIDGET_BUTTON 	1
#define WIDGET_ICONBUTTON	2

typedef struct _app_list 
{
	unsigned short  app_id;
	char app_name[APP_MAX_NAME_LEN];
	char app_lable[APP_MAX_LABLE_LEN];
	unsigned long app_addr;
	void *so_handle;
	pid_t child_id;
}APP_LIST;

typedef struct {
	char file[20];
	char shm_key[8];
	char mas_que_key[8];/*master message queue key */
	char app_que_key[8];/*application message queue key */
	char app_id[8];
}APP_PARAM;


struct message_data {

	int api_id;
	
	int line;
	int colum;
	
	int gprs_d;
	int gprs_s;
	int modem_dial;
	int r1;
	int r2;
	int r3;
	int r4;
	
	unsigned char mtext[100];
};

struct lcd_messaage {


	long  mtype;
	struct message_data msg_data;
	
};

/* added liugm, 2011/5/9  */
struct __request {
	int src_msgque_key;
	int src_id;
	int dst_id;
	int length;
	unsigned char data[256];
};
struct __request g_req_ack;



/*  globle var */
volatile unsigned char g_app_timeout;
unsigned char g_app_cnt;

APP_LIST  g_app_list[APP_MAX_CNT];
int g_app_dead_list[9];
int g_app_dead;
int g_shm_id;
int g_mas_que_id;
int g_app_que_id[APP_MAX_CNT];
int g_tms_set_flag;
int g_have_maintenanced;

int SECURITY_STATE;

int app_event_tms(OSEVENTID EventID, OSEVENT *pEvent);

#endif


