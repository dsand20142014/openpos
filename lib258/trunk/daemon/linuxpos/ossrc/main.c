/**************************************************************
* porject : linux pos
*
* 2011/6/7 liugm. Using Os__Mifare_polling() for check mifare card
*                 and OSDRV_Abort() to stop ICC amd MFC device
* 2013/4/1 XiangLiu. Add Os_Wait_Event_NoKey() to wait only key event
*                    when TEST/SETTING running because they're moved
*                    from linuxpos to QLCD.
*****************************************************************/
#include <stdarg.h>
#include "sys_linux.h"
#include "main.h"
//#include "oserr.h"
#include "oscfg.h"
#include "oscfg_param.h"
#include "osdown.h"
#include "osdrv.h"
#include "sand_key.h"
#include "sand_timer.h"
#include "osmifare.h"
#include "osspd.h"
#include <dlfcn.h>
#include <iconv.h>
#include <linux/input.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h> 
#include "applist.h"
#include "ostools.h"
#include "osevent.h"
#include "devSignal.h"
#include "param.h" //xl
#include "modules.h"
#include "fsync_drvs.h"
#include "sysglobal.h"
#include "sysparam.h"

#define RZSZ_DWN    0
#define SHM_TEST	0
#define LINUXPOS_FROM_REBOOT  0
#define LINUXPOS_FROM_CONFIG  1
#define OSEVENTID_CIRCLE      0xf0

/* 
 * MSG API ID define for qlcddaemon 
 * Add by xiangliu 2013.01.24
 */
#define MSG_ID_GPS		5
#define MSG_ID_GPRS		10
#define MSG_ID_LOW_BATTERY	11
#define MSG_ID_PRINTER	30
#define MSG_ID_WIFI     15
#define MSG_ID_SETDATE	16
#define MSG_ID_APP      40
#define ID_UP_FONT      20
#define ID_TF_DETECT    21
#define ID_CHRG_DETECT  22
#define ID_CHAG_BAR     23
#define ID_BATT_VALUE   24
#define ID_PRN_HOT      25
#define ID_USBDISK_DETECT    26
#define nbytes 140
#define CURRENT_APP_SHM_ID  67835
#define MAX_APP_COUNT 36

OSEVENT_B     OSEvent;
OSEVENTIN   OSEventIn;
OSEVENTOUT  OSEventOut;
unsigned char OSEventBuffer[MAXAPPSHAREBUF];

extern void lcd_init(void);

static void sand_execl_app(void);
int  app_init(void);
int  app_running(void);
static void  app_menu_display(unsigned char ucMenuCurrPage);
void app_load_applications(void);
void lcd_bright(unsigned char bright);
int  app_power_off_count(void);
static int current_app = 1;//通过qlcddamo消息传递来判断是否在9宫格界面.1表示在9宫格   -1表示在具体的应用里面
static int current_app_id = -1;
static bool MAIN_FORM = true; // xl add because of current_app not realtime

static void app_event_select(unsigned char ucKey,OSEVENT_B *pEvent);
//extern APPLIST applist[MAX_APP_COUNT];
APPINFOLIST applist[MAX_APP_COUNT];
APPINFOLIST applist_tmp[MAX_APP_COUNT];
/* compatable  with ps100 */
unsigned char app_wait_key(unsigned char ucFlag,unsigned int uiTimeout);
static int app_event_single(int num,OSEVENT_B *pEvent);
//static int  app_event_common(OSEVENTID EventID,OSEVENT_B *pEvent);
static int  app_event_circle(OSEVENTID EventID,OSEVENT_B *pEvent);
static void app_create_ipcs(int app_count,int);
static int  app_write_event(OSEVENT_B *pevent);
static int  app_read_event(OSEVENT_B *pevent);
static int  app_active_process(int num);
static int  app_wait_complete(void);
static int  app_exit_process(int num);
static int  app_remove_process(int count);
static void send_msg_qlcd(int id, int value);

extern int Os_open_key_fd(void);
extern int Os_open_mag_fd(void);
extern int set_app_id(unsigned short appid);
const key_t shm_key = APP_SHM_KEY;
const key_t mas_que_key=APP_MAS_QUE_KEY;
/*每套应用的共享内存id*/
const key_t app_que_key[APP_MAX_CNT]={1210,1211,1212,1213,1214,1215,1216,1217,1218,1219,
                                      1220,1221,1222,1223,1224,1225,1226,1227,1228,1229,
                                      1230,1231,1232,1233,1234,1235,1236,1237,1238,1239,
                                      1240,1241,1242,1243,1244,1245,1246,1247,1248,1249,};

static unsigned short  ucOSCurrAppID;
static unsigned short  ucOSMasterAppID;
static bool REGISTER_FLAG = FALSE;
int manager(void *p){return 0;}

#define MAIN_INIT            0
#define MAIN_LIST_APPS       1
#define MAIN_LOAD_APPS       2
#define MAIN_POWER_EVENT     3
#define MAIN_DISP_MENU       4
#define MAIN_DEV_STATUS      5
#define MAIN_CONFIG          6
#define MAIN_TIMEOUT         7
#define MAIN_CONFIG_NOAPP    8      

/*  state of device event
*   you can add other device event 
*/ 
#define MAIN_MAG_EVENT       20
#define MAIN_ICC_EVENT       21
#define MAIN_KEY_EVENT       22
#define MAIN_MIF_EVENT       23


static int main_state;
static int ucCount;
static unsigned char ucMenuPage;

static void  app_listting(void);
static void app_mag_event(DRV_OUT *);
static int app_icc_event(DRV_OUT *);
static int app_key_event(DRV_OUT *);
static int app_mifare_event(DRV_OUT *);
static void app_mifare_abort(void);
static int app_mifare_set_exist(void);

#define lcd_clear(line)     OSMMI_ClrDisplay(0x30,line)
#define lcd_display(x,y,s)  Os__display_qt(0x30, 0, 0, (unsigned char*)s, WIDGET_LABEL, 0, 0);	 
//#define printf(fmt,...) _debug(fmt,...)

static int battery_times = 0;
static unsigned char DEBUG = 0;

static void _debug(char *fmt,...)
{
    va_list ap;
    char string[512];

    if (DEBUG) {

        va_start(ap,fmt);
        vsprintf(string,fmt,ap);
        fprintf(stderr,string);
        va_end(ap);
    } else {
        return;
	}
}



/*
 * Linuxpos handle error/exception here, it can not exit.
 * Add by xiangliu 2013.1.24
 */
static void handle_error(char *err)
{
	if (err) {
		_debug("----Linuxpos ERROR: %s\n", err);
	}
}

//1-auto 
int check_MFI_AUTO(void)
{
    int iret;
    iret=access("/drivers/mfi",F_OK);

    if(iret!=0) 
        return 1;
    else
        return 0;
} 


/* added for mifare abort completely */
static void app_mifare_interrupt(int sig)
{
    app_mifare_abort();
	handle_error("mifare abort!");
}

static void app_listting(void)
{
	int i;
	int j = 0;
	int len = 0;

	g_app_cnt = Os__applist_get(applist, MAX_APP_COUNT);//xl 130411//APPLIST_list_load();//loadAppList();

	for(i = 0; i < g_app_cnt; i++)
	{
		/*用来判断是否为隐藏如果隐藏就不启动*/
		if((applist[i].hidden[0] == '0')&&atoi((char *)applist[i].id) > 9)
		{
			strcpy(applist_tmp[j].id, applist[i].id);
			strcpy(applist_tmp[j].name, applist[i].name);
//			strcpy(applist_tmp[j].path, "/app/");
//			strcat(applist_tmp[j].path, applist[i].id);
			strcpy(applist_tmp[j].hidden, applist[i].hidden);
			j++;
		}
	}
	g_app_cnt = j;
	
#ifdef TEST // TEST and SETTING ware delete by xl
	/*????Ϊ???úͲ???*/
	strcpy(applist_tmp[g_app_cnt].id, "000000");
	strcpy(applist_tmp[g_app_cnt+1].id, "000000");
	for(i = 0; i < app_cnt_tmp; i++)
	{
		if(atoi((char *)applist[i].id) == 1)
		{
			strcpy(applist_tmp[g_app_cnt].id, applist[i].id);
			strcpy(applist_tmp[g_app_cnt].name, applist[i].name);
			strcpy(applist_tmp[g_app_cnt].path, applist[i].path);
			strcpy(applist_tmp[g_app_cnt].hidden, applist[i].hidden);
			j++;
		}
		if(atoi((char *)applist[i].id) == 2)
		{
			strcpy(applist_tmp[g_app_cnt+1].id, applist[i].id);
			strcpy(applist_tmp[g_app_cnt+1].name, applist[i].name);
			strcpy(applist_tmp[g_app_cnt+1].path, applist[i].path);
			strcpy(applist_tmp[g_app_cnt+1].hidden, applist[i].hidden);
			j++;
		}
	}
#endif
	
	/* copy applist_tmp to g_app_list(main.h) */
	for(i=0;i<g_app_cnt;i++)
	{
		g_app_list[i].app_id = atoi((char *)applist_tmp[i].id);
		//strncpy(g_app_list[i].app_lable, (char *)applist_tmp[i].path, strlen((char *)applist_tmp[i].path));
		strncpy(g_app_list[i].app_name, (char *)applist_tmp[i].name, strlen((char *)applist_tmp[i].name));
				
		/*strcat(g_app_list[i].app_lable,"/");
		strcat(g_app_list[i].app_lable,(char *)applist_tmp[i].id);*/
		
		strcpy(g_app_list[i].app_name, (char *)applist_tmp[i].id); 
		//strcpy(g_app_list[i].app_lable, (char *)applist_tmp[i].name);
		/* show chinese on screen xl add 130410 */
		len = strlen((char *)applist_tmp[i].name);
		if (len > 12) {
			strncpy(g_app_list[i].app_lable, (char *)applist_tmp[i].name, 12);
			strcat(g_app_list[i].app_lable, "\n");
			strcat(g_app_list[i].app_lable, (char *)applist_tmp[i].name+12);
		}else {
			strcpy(g_app_list[i].app_lable, (char *)applist_tmp[i].name);
		}
	}
#ifdef TEST
	if(atoi((char *)applist_tmp[g_app_cnt].id) == 1)
	{
		i = g_app_cnt;
		g_app_list[i].app_id = atoi((char *)applist_tmp[i].id);
		strncpy(g_app_list[i].app_lable, (char *)applist_tmp[i].path, strlen((char *)applist_tmp[i].path));
		strncpy(g_app_list[i].app_name, (char *)applist_tmp[i].name, strlen((char *)applist_tmp[i].name));
				
		strcat(g_app_list[i].app_lable,"/");
		strcat(g_app_list[i].app_lable,(char *)applist_tmp[i].id);
		
		strcpy(g_app_list[i].app_name, (char *)applist_tmp[i].id);
		strcpy(g_app_list[i].app_lable, (char *)applist_tmp[i].name);
	}
	if(atoi((char *)applist_tmp[g_app_cnt+1].id) == 2)
	{
		i = g_app_cnt+1;
		g_app_list[i].app_id = atoi((char *)applist_tmp[i].id);
		strncpy(g_app_list[i].app_lable, (char *)applist_tmp[i].path, strlen((char *)applist_tmp[i].path));
		strncpy(g_app_list[i].app_name, (char *)applist_tmp[i].name, strlen((char *)applist_tmp[i].name));
				
		strcat(g_app_list[i].app_lable,"/");
		strcat(g_app_list[i].app_lable,(char *)applist_tmp[i].id);
		
		strcpy(g_app_list[i].app_name, (char *)applist_tmp[i].id);
		strcpy(g_app_list[i].app_lable, (char *)applist_tmp[i].name);
	}
#endif	
}

/* 安装信号处理函数，处理SIGINT和SIGTERM信号 */
static void app_sigaction(void)
{
    struct sigaction siga;
    siga.sa_handler = app_mifare_interrupt;
    siga.sa_flags  = 0;
    memset (&siga.sa_mask, 0, sizeof(sigset_t));
    sigaction(SIGINT, &siga, NULL);

    struct sigaction sigb;
    sigb.sa_handler = app_mifare_interrupt;
    sigb.sa_flags  = 0;
    memset (&sigb.sa_mask, 0, sizeof(sigset_t));
    sigaction(SIGTERM, &sigb, NULL);

    return;
}

/* send message to app */
static int sendmsg(int app_id, char *message)
{
	APPMSG msg;

    msg.m_type = APP_MSG_TYPE;
    strcpy(msg.m_data, message);
    return sys_msgque_send(g_app_que_id[app_id], &msg, APP_MSG_LEN);
}

/*
 * Circle event from one app to other apps
 * Return -1 when error
 * Add by xiangliu 03.1.29 
 */
static int app_circle_event()
{
    //unsigned char num;
    int num;
    int retval=NOK; // not processed
	int c_app = current_app_id;

	OSEVENT_B *pEvent = &OSEvent;

	if(pEvent == NULL || g_app_cnt < 2) {
		perror("----Linuxpos ERROR----app_circle_event: Event is NULL or APP < 2!");
		goto err; // can not return - linuxpos will block
	}

    pEvent->Type = OSEVENTTYPE_CIR;
    pEvent->Index = OSEVENTID_CIRCLE; //<--FIXME

	retval = app_read_event(pEvent);
	if (retval < 0) {
		perror("----Linuxpos ERROR----app_circle_event ERROR after app_read_event");
		return -1;
	}
	
	/* copy memory to app */
	memcpy(((struct seven *)pEvent)->pt_eve_in->e_cash_register.ecr_msg, 
			((struct seven *)pEvent)->pt_eve_out->xxdata, MAXAPPSHAREBUF-1);

	_debug("\n----Linuxpos Debug----<app_circle_event> will send to other app\n");

	/* send msg to all apps */
    for(num=0; num<g_app_cnt; num++)
    {
		if(c_app == num) { // do not send to itself
			continue;
		}
	    app_event_single(num,pEvent);
        if(pEvent->pOut->ucStatus == 5)//OSEVENT_PROCESSED)
        { 
			/* copy memory to curret app */
			memset(((struct seven *)pEvent)->pt_eve_in->e_cash_register.ecr_msg, 0, MAXAPPSHAREBUF);
			memcpy(((struct seven *)pEvent)->pt_eve_in->e_cash_register.ecr_msg, 
					((struct seven *)pEvent)->pt_eve_out->xxdata, MAXAPPSHAREBUF-1); 
					//strlen((char *)((struct seven *)pEvent)->pt_eve_out->xxdata));
			retval = app_write_event(pEvent);
			_debug("\n----Linuxpos Debug---- get ACCEPT from %d\n", num);
//, msg[%d]: %s\n", num, strlen((char *)((struct seven *)pEvent)->pt_eve_out->xxdata), (char *)((struct seven *)pEvent)->pt_eve_in->e_cash_register.ecr_msg);

			if (retval < 0) {
				printf("----Linuxpos ERROR ----<app_circle_event> app_write_event error!\n"); 
				return retval;
			}

			retval = sendmsg(c_app, "ACCEPT");
			if (retval < 0) {
				printf("----Linuxpos ERROR ----<app_circle_event> sys_msgque_send error!\n"); 
				return retval;
			}
            return retval;
        }
    }
err:	
	retval = sendmsg(c_app, "NOACCEPT");
	if (retval < 0) {
		printf("----Linuxpos ERROR ----<app_circle_event> sys_msgque_send error!\n"); 
		return retval;
	}
    
    return retval;
}


/*处理磁条卡事件*/
static void app_mag_event(DRV_OUT *pMagDrvOut)
{
    OSEVENTMAGCARD *pMagEvent = &(OSEvent.pIn->Mag);
    memcpy(pMagEvent->aucData,pMagDrvOut,256);
    app_event_circle(OSEVENTID_MAGCARD,&OSEvent);
}

static int app_icc_event(DRV_OUT *pIccDrvOut)
{
    int processed;
    unsigned char line;

    OSEVENTICC *pIccEvent = &(OSEvent.pIn->Icc);
	memcpy(pIccEvent->aucAtr,pIccDrvOut->xxdata,sizeof(pIccDrvOut->xxdata));
    processed = app_event_circle(OSEVENTID_ICC,&OSEvent);
    if (processed == NOK) {
		/*以下为当没有应用处理该卡时需要在屏幕上显示的错误提示信息*/
#ifdef USEOLD		//delete by xl 20130124
        Os__clr_display(255);

        unsigned char GB2312_str_1[] = "Disable ICC";
        unsigned char GB2312_str_2[] = "Please Remove ICC";
		
		line = 1;
        Os__GB2312_disp(line,GB2312_str_1);
        line = 2;
        Os__GB2312_disp(line,GB2312_str_2);

        sleep(1);
#endif        
        OSICC_Remove();
    }
    return 0;
}

static int app_key_event(DRV_OUT *pKeyDrvOut)
{
    unsigned char ucKey;
#if 0
    int k = 0;
	/*1??2???Ǳ?ʾ???úͲ??ԣ??????????????⴦?���*/
	if(atoi((char *)applist_tmp[g_app_cnt].id) == 1)
		k++;
	if(atoi((char *)applist_tmp[g_app_cnt+1].id) == 2)
		k++;
#endif   
    ucKey = pKeyDrvOut->xxdata[1];
  
    switch (ucKey) {

    //case KEY_SYS_SET:
    //case KEY_SYS_TEST:    
    case '1'...'9':
    	app_event_select(ucKey,&OSEvent);
        if ( (ucKey-0x31) < (g_app_cnt - ucMenuPage*9))
        {
		    
		    main_state = MAIN_DISP_MENU;
        }            
        else
        {
        	main_state = MAIN_DEV_STATUS;
        }
            
        break;
    default:
        main_state = MAIN_DEV_STATUS;
        break;
    }//switch (uckey)

    return main_state;
}

#ifndef QC430_v01 
static int app_mifare_event(DRV_OUT *pPCIDrvOut)
{
    int processed;

    OSEVENTICC *pIccEvent = &(OSEvent.pIn->Icc);
	memcpy(pIccEvent->aucAtr,pPCIDrvOut->xxdata,sizeof(pPCIDrvOut->xxdata));
    processed = app_event_circle(OSEVENTID_MFCCRD,&OSEvent);

    if (processed == NOK)
    {
#ifdef USEOLD    
        Os__clr_display(255);
        char GB2312_str_1[] = "Disable MIFARE CARD";
        char GB2312_str_2[] = "Please Remove MIFARE CARD";

        Os__GB2312_disp(1,GB2312_str_1);
        Os__GB2312_disp(2,GB2312_str_2);
#endif        
        Os__MIFARE_Remove();
    }
    return 0;
}
#endif
int g_lcd_language;

void Os__pm_enable_gprs(void)
{
    int fd, result;
#define KEEP_GPRSPW 10

    fd = fopen("/dev/lattice", "wr");
    if (fd == -1)
    {
        Uart_Printf("open device /dev/lattice error! %s:%d\n",__func__, __LINE__);
        return;
    }

    result = ioctl(fd, KEEP_GPRSPW);

    close(fd);
}

/*通过该函数向qlcddaemon发送消息来让其显示qt的主界面*/
void app_menu_display(unsigned char ucMenuCurrPage)
{
    unsigned char ucI;
    unsigned char ucMenuCount;
    unsigned char ucMenuCurrCount;
    unsigned char ucMenuPage;
    unsigned char ucMenuPageCount;
    unsigned char ucMenuSelect;
    unsigned char aucID[50];
    int shm_id;
    int *shm_test;
    
    //char English_welcom[] = "WELCOME";
    unsigned char ucBuf[50];


    if (( shm_id=shmget( CURRENT_APP_SHM_ID,8096,IPC_CREAT|0666 ))<0 ){
        perror("### shmget failed.");
		handle_error("Can NOT reach qlcddaemon! Please reboot!");
		return;
    }
    shm_test = shmat(shm_id,NULL,0);

    //shmdt(shm_addr);//2012.8.14 by wyl
    shmdt(shm_test);

	if(g_app_cnt < 1) {
		lcd_display(1,3,"NO APP ! ...");
	}else { //xl add
	    ucMenuCount = g_app_cnt;
	    ucMenuPage = (ucMenuCount - 1) / 3 + 1;
	    ucMenuCurrPage = ucMenuCurrPage % ucMenuPage;
	
	    ucMenuSelect = 0;
	
	    //OSMMI_ClrDisplay(ASCIIFONT57, 255);
	    //Os__display_qt(0x30, 0, 0, English_welcom, WIDGET_LABEL, 0, 0); //xl del
	
	    ucMenuCurrCount = ucMenuCurrPage * 3;
	    
	    for(ucI = ucMenuCurrCount, ucMenuSelect = 0, ucMenuPageCount = 0;
	        (ucI < ucMenuCount) && (ucMenuSelect < 40);
	        ucI++, ucMenuSelect++, ucMenuPageCount++)
	    {
	        memset(ucBuf,0,sizeof(ucBuf));
	        sprintf(ucBuf,"%s",g_app_list[ucI].app_lable); 
	        memset(aucID, 0, sizeof(aucID));
	        sprintf((char *)aucID,"%d.%s", ucMenuSelect + 1, ucBuf);
	        Os__display_qt(0x30, 1 + ucMenuSelect, 0, aucID, WIDGET_ICONBUTTON, ucMenuSelect + 1, atoi(applist_tmp[ucI].id));
	    }
	}
}

/*通过按键来调用应用*/
static void app_event_select(unsigned char ucKey,OSEVENT_B *pEvent)
{
    OSEVENTOUT *pOut;
    int app_num;/* 2009-06-17 */
	
#ifdef TEST
	//_debug("\n----Linuxpos Debug---- app_event_select ucKey=%x\n", ucKey);
	if(ucKey == KEY_SYS_SET)//||(ucKey == KEY_SYS_TEST))//设置测试
	{
	
		pEvent->Type = OSEVENTTYPE_SEL;
        pEvent->Index = 0xFF;
        pOut = pEvent->pOut;
        if(atoi(applist_tmp[g_app_cnt].id) == 1)//表示设置下载已进去
		{
			/* 
			 * check its status before single
			 * add by xiangliu 2013.1.25 
			 */
			if (system("ps| grep 00001|grep -v grep") == 256) { // no such ps
				printf("\n----Linuxpos ERROR---- The app SETTING was dead!\n");
				Os__display(0, 0, "The app SETTING was dead!");
				return;
			}
//	        chdir(applist_tmp[g_app_cnt].path);
   	        app_event_single(g_app_cnt,pEvent);
       	    while(1)
           	{
                pEvent->Type = OSEVENTTYPE_SEL;
                pEvent->Index = ucKey;
                pOut = pEvent->pOut;
                app_event_single(g_app_cnt,pEvent);
                break;
           	}
        }
	}
	else if(ucKey == KEY_SYS_TEST)//||(ucKey == KEY_SYS_TEST))//设置测试
	{
		pEvent->Type = OSEVENTTYPE_SEL;
        pEvent->Index = 0xFF;
        pOut = pEvent->pOut;
        if(atoi(applist_tmp[g_app_cnt+1].id) == 2)//表示测试下载已进去
        {
			/* 
			 * check its status before single
			 * add by xiangliu 2013.1.25 
			 */
			if (system("ps| grep 00002|grep -v grep") == 256) { // no such ps
				printf("\n----Linuxpos ERROR---- The app TEST was dead!\n");
				Os__display(0, 0, "The app TEST was dead!");
				return;
			}
//        	chdir(applist_tmp[g_app_cnt+1].path);
           	app_event_single(g_app_cnt+1,pEvent);
           	while(1)
           	{
                pEvent->Type = OSEVENTTYPE_SEL;
                pEvent->Index = ucKey;
                pOut = pEvent->pOut;
                app_event_single(g_app_cnt+1,pEvent);
                break;
            }
        }
	}
	else
#endif

	if((ucKey >= '1')&&(ucKey <= '9'))
    {
        pEvent->Type = OSEVENTTYPE_SEL;
        pEvent->Index = 0xFF;
        pOut = pEvent->pOut;
        ucKey = ucKey - '1';
//        if( ucKey < g_app_cnt )

        if( ucKey < g_app_cnt - 9*ucMenuPage)
        {
            //app_num = ucKey; /* 2009-06-17 */
            app_num = ucKey + 9*ucMenuPage;//修改翻页之后无法正确进入应用
            if (current_app == 1) {
                current_app_id = app_num;
//                chdir(applist_tmp[app_num].path);
				MAIN_FORM = false;
            	if (app_event_single(app_num,pEvent) < 0) {
					char errMsg[100];
					sprintf(errMsg, "The app with id[%s] was dead!\n Please reboot.", applist_tmp[app_num].id);
					lcd_display(0, 0, errMsg);
				}
				MAIN_FORM = true;
        	}
#if 0 //del by xl 20130408 for no use
			 else { //add by xl - Do not single the key event to all app if current_app=1.
	            while(1)
	            {
	                ucKey = ucKey + '1';
	
	                if( !ucKey )
	                    return;//(OSEVENT_PROCESSED);
	                if(  (ucKey >= '1')&&(ucKey <= '9'))
	                {
	                    pEvent->Type = OSEVENTTYPE_SEL;
	                    pEvent->Index = ucKey;
	                    pOut = pEvent->pOut;
	                    app_event_single(app_num,pEvent);
	                    
	                    break;
	                }
	                if(  (ucKey == KEY_UP )||(ucKey == KEY_DOWN ))
	                {
	                    pEvent->Type = OSEVENTTYPE_SEL;
	                    pEvent->Index = ucKey;
	                    pOut = pEvent->pOut;
	                    app_event_single(app_num,pEvent);
	                    continue;
	                } else
	                    break;
	            }
	        }
#endif
        }
    }
    /*else
		_debug("");*/
    ucOSCurrAppID = ucOSMasterAppID;
    //return(OSEVENT_PROCESSED);
}

/*
* ucFlag =0 ,return when timeout or key pressed  
* ucFlag =1, return when key pressed 
*/
unsigned char app_wait_key(unsigned char ucFlag,unsigned int uiTimeout)
{
    return(Os__xget_key());
}

static int app_event_single(int num,OSEVENT_B *pEvent)
{
/*	int k = 0;

	if(atoi((char *)applist_tmp[g_app_cnt].id) == 1)
		k++;
	if(atoi((char *)applist_tmp[g_app_cnt+1].id) == 2)
		k++;
*/
	/* 
	 * check its status before single
	 * add by xiangliu 2013.1.25 
	 */
#if 0
	char buftmp[30]; 
	sprintf(buftmp, "ps| grep %s|grep -v grep", applist_tmp[num].id);

	if (system(buftmp) == 256) { // no such ps
#endif
	int i;
	for (i=0; i<sizeof(g_app_dead_list); i++) {
		if (g_app_dead_list[i] == g_app_list[num].child_id) {
			printf("\n----Linuxpos ERROR---- The app with id[%s] was dead!\n", applist_tmp[num].id);
			/*char errMsg[100];
			sprintf(errMsg, "The app with id[%s] was dead!\n Please reboot.", applist_tmp[num].id);
			Os__display(0, 0, errMsg);*/
			return -1;
		}
	}

#if APP_DYNAMIC 
    void (*manager)(struct seven *);
    manager =  (void (*)(struct seven *))g_app_list[num].app_addr;
    set_app_id(g_app_list[num].app_id);
    (*manager)((struct seven *)pEvent);
#else
/*	if(num >= g_app_cnt + k)
		return;*/

	_debug("------------------------------------------------current app: %d[%d]; type: %x+%x\n", num, g_app_list[num].app_id, pEvent->Type, pEvent->Index);
	/*以下就是写消息到应用并且等待应用返回*/
    if (app_write_event(pEvent) < 0) {
		_debug("\n----Linuxpos ERROR---- app_write_event failed!\n");
		return -1;
    }
    if (app_active_process(num) < 0 ) {
		_debug("\n----Linuxpos ERROR---- app_active_process failed!\n");
		return -1;
	}
    if (app_wait_complete() < 0) {
		_debug("\n----Linuxpos ERROR---- app_wait_complete failed!\n");
		return -1;
	}
//    usleep(50000); 
    return app_read_event(pEvent);
#endif
}

#if 0
static int app_event_common(OSEVENTID EventID,OSEVENT_B *pEvent)
{
    unsigned char num;
    int retval=0;

    pEvent->Type = OSEVENTTYPE_COM;
    pEvent->Index = EventID;

    for(num = 0; num < g_app_cnt; num++)
    {
        app_event_single(num, pEvent);
    }

    return(retval);
}
#endif

/*循环发消息*/
static int app_event_circle(OSEVENTID EventID,OSEVENT_B *pEvent)
{
    //unsigned char num;
    int num;
    int retval=NOK; // not processed

    pEvent->Type = OSEVENTTYPE_CIR;
    pEvent->Index = EventID;

    //if(current_app == 1)//在9宫格界面才循环发送到每一个应用
    if (current_app_id == -1)
    {
		if(REGISTER_FLAG)
		{
			memcpy(((struct seven *)pEvent)->pt_eve_in->e_cash_register.ecr_msg, ((struct seven *)pEvent)->pt_eve_out->xxdata, strlen((char *)((struct seven *)pEvent)->pt_eve_out->xxdata));
			
			((struct seven *)pEvent)->event_nr = EVESEL;
			((struct seven *)pEvent)->event_type = EVE_CASH_REGISTER;
			
			app_write_event(pEvent);
			
			app_read_event(pEvent);
			REGISTER_FLAG = FALSE;
		}
        for(num=0;num<g_app_cnt;num++)
        {
            ucOSCurrAppID = g_app_list[num].app_id;
            ucOSMasterAppID = ucOSCurrAppID;
   
            pEvent->pOut->ucStatus = NOK;
            app_event_single(num,pEvent);

            if(pEvent->pOut->ucStatus == OSEVENT_PROCESSED)
            {
            	memset(((struct seven *)pEvent)->pt_eve_in->e_cash_register.ecr_msg, 0, 255);
				memset(((struct seven *)pEvent)->pt_eve_out->xxdata, 0, 255);
				app_write_event(pEvent);
                retval=OSEVENT_PROCESSED;
                break;
            }
         
/* 调试打印 */
        }
    }
    else
    {
        if(current_app_id != -1)
        {
            app_event_single(current_app_id,pEvent);
            if(pEvent->pOut->ucStatus == OSEVENT_PROCESSED)
            {
                retval=OSEVENT_PROCESSED;
            }
        }
    }
    
    ucOSCurrAppID = 0;
    ucOSMasterAppID = 0;
    return retval;
}



/***************************************************************************
*  for linux added 
*****************************************************************************/
static void app_create_ipcs(int app_count,int f_start)
{
    int num;
    int k = 0;

	if(atoi((char *)applist_tmp[g_app_cnt].id) == 1)
		k++;
	if(atoi((char *)applist_tmp[g_app_cnt+1].id) == 2)
		k++;
	
    for(num=0; num<app_count+k; num++)
    {
        g_app_que_id[num] = sys_msgque_create(app_que_key[num]);
    }
    if (f_start == LINUXPOS_FROM_REBOOT){
        g_shm_id     = sys_shm_create(shm_key);
        g_mas_que_id = sys_msgque_create(mas_que_key);
    }
}

static void child_wait (int sig){
    int pid;
    int status;
    while ((pid = waitpid (-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED (status))
            _debug ("--pid=%d--status=%x--rc=%d---\n",pid, status, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            _debug ("--pid=%d--signal=%d--\n", pid, WTERMSIG (status));

		if (g_app_cnt > 0) {
			int i;
			for(i=0; i<g_app_cnt; i++) {
				if(g_app_list[i].child_id == pid)
					break;
			}
			if (i != g_app_cnt) {
				/* put it into g_app_dead_list */
				g_app_dead++;
				if (g_app_dead == 9) {
					g_app_dead = 0;
				}
				g_app_dead_list[g_app_dead] = pid;
				APPLIST_rmove_appid(pid);//xl add for API 20130423
			}
    	}
	}
}

static int app_write_event(OSEVENT_B *pevent)
{
    unsigned char *event_addr;
    unsigned int offset=0;
    int retval=0;

    event_addr = sys_shm_attach(g_shm_id);
    /* Type  */
    memcpy(event_addr + offset, &pevent->Type, 1);
    offset += 1;

    /* Index */
    memcpy(event_addr + offset, &pevent->Index, 1);
    offset += 1;

    /* EventIn */
    memcpy(event_addr + offset, pevent->pIn, sizeof(OSEVENTIN));
    offset += sizeof(OSEVENTIN);

    /* EventOut */
    memcpy(event_addr + offset, pevent->pOut, sizeof(OSEVENTOUT));
    offset += sizeof(OSEVENTOUT);

    retval = sys_shm_detach(event_addr);
	
	//_debug("----Linuxpos Debug---- <app_write_event> --- after sys_shm_detach=%d\n", retval);//xl
    return retval;
}

static int app_read_event(OSEVENT_B *pevent)
{
    unsigned char *event_addr;
    unsigned int offset=0;

    pevent->pOut->pucData = OSEventBuffer;/* 2011/5/11 */

    event_addr = sys_shm_attach(g_shm_id);
//	_debug("----Linuxpos Debug---- <app_read_event>---after sys_shm_attach: %s\n", event_addr);

    /* ucStatus  */
    memcpy(&pevent->pOut->ucStatus, event_addr + offset, 1);
    offset += 1;

    /* ucDownload  */
    memcpy(&pevent->pOut->ucDownload, event_addr + offset, 1);
    offset += 1;

    /*AppStatus */
    memcpy(&pevent->pOut->ucAppStatus, event_addr + offset, 1);
    offset += 1;

    /* pucData  2011/5/9  */
    memcpy(pevent->pOut->pucData, event_addr + offset, sizeof(OSEventBuffer));
    offset += sizeof(OSEventBuffer);
    
    memcpy(((struct seven *)&OSEvent)->pt_eve_out->xxdata, pevent->pOut->pucData, MAXAPPSHAREBUF-1);
    ((struct seven *)&OSEvent)->pt_eve_out->xxdata[MAXAPPSHAREBUF-1] = 0;

    return sys_shm_detach(event_addr);
	
}

static int app_active_process(int num)
{
    APPMSG msg;
    int retval=0;

    msg.m_type = APP_MSG_TYPE;
    strcpy(msg.m_data,"HELLO");

    retval=sys_msgque_send(g_app_que_id[num], &msg, APP_MSG_LEN);
    //_debug("\n----Linuxpos Debug----<app_active_process>--- send %d[msgid=%d] HELLO = %d\n", num, g_app_que_id[num], retval);

    if(retval<0)
    {
        _debug("\n----Linuxpos ERROR----<app_active_process>---sys_msgque_send\n");
        app_remove_process(g_app_cnt);
//        OSSPD_file_delete();
        system_reboot();
    }

    return retval;
}

static int app_exit_process(int num)
{
    APPMSG msg;
    int retval=0;

    msg.m_type = APP_MSG_TYPE;
    strcpy(msg.m_data,"EXIT");

    sys_msgque_send(g_app_que_id[num], &msg, APP_MSG_LEN);

    return retval;
}

static int app_remove_process(int count)
{
    int i;
    for (i=0;i<count;i++){
        app_exit_process(i);
    }
}
/* modified liugm , 2011/5/9 */
static int app_read_request(struct __request *req);
static int app_write_acknowledge(struct __request *req);
static int app_active_request_process(int msgkey);

/**
**  wait application done or request run  
**/
static int app_wait_complete(void)
{
    APPMSG msg;
    int retval=0;
    int num;
    //int i=0;

    do{
        _debug("----Linuxpos Debug----<app_wait_complete>--- before sys_msgque_recv\n");
		//i++;
        retval = sys_msgque_recv(g_mas_que_id, &msg,APP_MSG_LEN,APP_MSG_TYPE);
		if (retval == -1) {
        	printf("----Linuxpos ERROR----<app_wait_complete>--- sys_msgque_recv error!\n");
			return -1;
		}
		/*else if(i==2 && retval == 0) { // no msg after repeat many times
			return -1; //<-FIXME
		}*/
        
        if(strncmp(msg.m_data,"DONE",4)==0){       /* normal app done (app wakeup) */
			_debug("----Linuxpos Debug----<app_wait_complete>--- after sys_msgque_recv\n");
            return retval;
        }
        else if(strncmp(msg.m_data,"ACCEPT", 6) == 0){       /* normal app done */
            return retval;
        }
        else if(strncmp(msg.m_data,"REQUEST", 7) == 0){   /* app to app */
        	//_debug("----Linuxpos Debug----<app_wait_complete>--- Rev REQUEST!\n");
			retval = app_circle_event();
			continue;
		}
        else if(strncmp(msg.m_data,"OSAPP_REQ",9)==0){  /* request from app */
            /* read reguest */
            app_read_request(&g_req_ack);

            /*for (i=0;i<g_req_ack.length;i++){
                _debug("----Linuxpos Debug----<app_wait_complete>--- %02x \n",g_req_ack.data[i]);
            }*/

            /* set event */
            OSEvent.Type  = OSEVENTTYPE_SEL;
            OSEvent.Index = OSEVENTID_CASHREGISTER;
            memcpy(OSEvent.pIn->CashRegister, &g_req_ack.length , sizeof(int)); 				// length
            memcpy(OSEvent.pIn->CashRegister+sizeof(int), g_req_ack.data , g_req_ack.length); 	// data

            /* send event to request some applicaton doing */
            for(num=0; num < g_app_cnt; num++){
                if (g_app_list[num].app_id == g_req_ack.src_id)
                    continue ;
                /* write into share memmory */

                //_debug("[%s:%d] write event to  %d \n",__func__,__LINE__,g_app_list[num].app_id);

                if (app_write_event(&OSEvent) < 0) {
					_debug("\n----Linuxpos ERROR---- app_write_event failed!\n");
					return -1;
				}
                if (app_active_process(num) < 0) {
					_debug("\n----Linuxpos ERROR---- app_active_process failed!\n");
					return -1;
				}
               
                /* wait done */
                do {
					//_debug("\n----Linuxpos Debug----<app_wait_complete>--- Wait... before sys_msgque_recv...\n");
                    sys_msgque_recv(g_mas_que_id, &msg,APP_MSG_LEN,APP_MSG_TYPE);
					//_debug("\n----Linuxpos Debug----<app_wait_complete>--- Wait... after sys_msgque_recv...\n");
                } while (strncmp(msg.m_data,"DONE",4) !=0 );
				
                app_read_event(&OSEvent);
                __printf("[%s:%d] status = %d\n",__func__,__LINE__,OSEvent.pOut->ucStatus);

                if(OSEvent.pOut->ucStatus == OSEVENT_PROCESSED){
                    // dst_id
                    g_req_ack.dst_id = g_app_list[num].app_id;
                    // length
                    memcpy(&g_req_ack.length, OSEvent.pOut->pucData, sizeof(int));
                    // data
                    memcpy(g_req_ack.data  , OSEvent.pOut->pucData + sizeof(int), sizeof(g_req_ack.data));

                    /*******/
                    __printf("\n^^^^^^^app_write_ack ^^^^^^^^^^\n");
                    __printf("g_req_ack.src_id=%d\n",g_req_ack.src_id);
                    __printf("g_req_ack.dst_id=%d\n",g_req_ack.dst_id);
                    __printf("g_req_ack.length=%d\n",g_req_ack.length);
                    /*for (i=0;i<g_req_ack.length;i++){
                        __printf("%02x ",g_req_ack.data[i]);
                    }*/
                    _debug("\n^^^^^^^app_write_ack ^^^^^^^^^^\n");
                    /********/

                    app_write_acknowledge(&g_req_ack);
                    app_active_request_process(g_req_ack.src_msgque_key);
                    break;
                }
            }//endfor
            if (num == g_app_cnt){/* no app response */
                g_req_ack.dst_id = 0;
                g_req_ack.length = 0;
                memset(&g_req_ack.data ,0, sizeof(g_req_ack.data));
                app_write_acknowledge(&g_req_ack);
                app_active_request_process(g_req_ack.src_msgque_key);
                continue;
            }
        }//if (DONE or REQ)
    }while(1);
   
    return retval;
}

int system_reboot(void)
{
	_debug("\n----Linuxpos ERROR <system_reboot>---- \n");//xl
    OSMMI_ClrDisplay(0x30,255);
    OSMMI_DisplayASCII(0x30,2,0,"Rebooting...");
    sleep(5);
//    system("reboot");
}

#if 0
/* 2010-05-04 */
static void app_check_data_dir(void)
{
//_debug("\ncreat app dir failed!\n\n");
return;
    struct _app_list *applist=0;
    int i;
    char app_data_foldname[255];
    char id_str[10];

    make_fold(DATA_PATH);

    for (i=0,applist=g_app_list;i<g_app_cnt;i++,applist++){
        sprintf(id_str, "%d",applist->app_id);
        strcpy(app_data_foldname,DATA_PATH);
        strcat(app_data_foldname,id_str);

        if (make_fold(app_data_foldname)<0){
            Uart_Printf("Can't mkdir %s\n",app_data_foldname);
            exit(-1);
        }
    }
}

static int make_fold(char *foldname)
{
    DIR *foldfd;
    int ret;

    foldfd = opendir(foldname);
    if (foldfd == NULL){
        ret=mkdir(foldname, 0777);
        return ret;
    }

    closedir(foldfd);
    return 0;
}
#endif    

/*2010-05-04 */

#ifdef _PS4000
extern int lcd_message_id;

#if 0
/*???¼??????????????߳??м???״̬??*/
void modem_on()
{

    struct lcd_messaage msg;

    Uart_Printf("%s:%d\n",__FUNCTION__,__LINE__);
    msg.mtype = 50;
    msg.msg_data.api_id = MSG_ID_MODEM;
    msg.msg_data.modem_dial = 1;
    if(msgsnd(lcd_message_id, &msg, 140, 0) < 0 ){
        perror("XXXXX message send");
		handle_error("msgsnd <modem_on>");
    }
}


void modem_off()
{

    struct lcd_messaage msg;

    Uart_Printf("%s:%d\n",__FUNCTION__,__LINE__);
    msg.mtype = 50;
    msg.msg_data.api_id = MSG_ID_MODEM;
    msg.msg_data.modem_dial = 0;
    if(msgsnd(lcd_message_id, &msg, 140, 0) < 0 ){
        perror("XXXXX message send");
		handle_error("msgsnd <modem_off>");
    }
}

void lcd_gprs_on()
{

    struct lcd_messaage msg;

    Uart_Printf("%s:%d\n",__FUNCTION__,__LINE__);
    msg.mtype = 50;
    msg.msg_data.api_id = MSG_ID_GPRS;
    msg.msg_data.gprs_d = 1;
    if(msgsnd(lcd_message_id, &msg, 140, 0) < 0 ){
        perror("XXXXX message send");
		handle_error("msgsnd <lcd_gprs_on>");
    }
}

void lcd_gprs_off()
{

    struct lcd_messaage msg;

    Uart_Printf("%s:%d\n",__FUNCTION__,__LINE__);
    msg.mtype = 50;
    msg.msg_data.api_id = MSG_ID_GPRS;
    msg.msg_data.gprs_d = 0;
    if(msgsnd(lcd_message_id, &msg, 140, 0) < 0 ){
        perror("XXXXX message send");
		handle_error("msgsnd <lcd_gprs_off>");
    }
}

#endif

#ifndef QC430_v01

void security_del(int flg)
{
#if 0
    char buf[128];
    int ret=0;

    OSMMI_ClrDisplay(0x30,255);
    OSMMI_DisplayASCII(0x30, 0, 0,"WARNING");

    memset(buf,0,128);
    sprintf(buf,"rm -rf %s*",SPD_DATA_PATH);
    ret=system(buf);

    if(flg!=1)
    {
        memset(buf,0,128);
        sprintf(buf,"rm -rf %sappinfo.dat",SPD_CONFIG_PATH);
        ret=system(buf);

        memset(buf,0,128);
        sprintf(buf,"rm -rf %s*",SPD_DOWN_PATH);
        ret=system(buf);

        memset(buf,0,128);
        sprintf(buf,"mv %slinuxpos /tmp",SPD_APP_PATH);
        ret=system(buf);
        memset(buf,0,128);
        sprintf(buf,"rm %s*",SPD_APP_PATH);
        ret=system(buf);
        memset(buf,0,128);
        sprintf(buf,"mv /tmp/linuxpos %s",SPD_APP_PATH);
        ret=system(buf);
    }
    sleep(2);
#endif    
}

void security_api(int signo)
{    
    _debug("####in %s state %d\n", __func__,SECURITY_STATE);
    
    if(SECURITY_STATE==1){

        security_del(0);

        system_reboot();
    }
}


/*
returnï¿¡o
?a?Â· 0 ???Â· 1 â²Ã²?aâ²Ã­?Ã³-1 ?Ã¡Ãš?â²Ã­?Ã³-2
*/
int security_setup(void)
{
    int sefd;
    char flag=-2;
    struct sigaction action;
    action.sa_handler = security_api;
    action.sa_flags = 0;
    sigaction(SIGUSR1, &action, NULL);
    SECURITY_STATE=1;
    sefd = open("/dev/sedev",O_RDWR);
    if(sefd<0)
        return -1;
    read(sefd,&flag,1);
    close(sefd);
    return flag;
}


#endif

#endif

int app_power_off_count(void)
{
    int cnt = POWER_OFF_CNT;

    if (g_cfg_pow.light_off_tm)
        cnt = (g_cfg_pow.power_off_tm * 3)/g_cfg_pow.light_off_tm ;
    return cnt;
}


/* 2011/5/9 */
static int app_read_request(struct __request *req_ack)
{
    unsigned char *addr;

    addr = sys_shm_attach(g_shm_id);

    memcpy(req_ack, addr, sizeof(*req_ack));

    sys_shm_detach(addr);

    return 0;
}

static int app_write_acknowledge(struct __request *req_ack)
{
    unsigned char *addr;

    addr = sys_shm_attach(g_shm_id);

    memcpy(addr, req_ack, sizeof(*req_ack));

    sys_shm_detach(addr);

    return 0;
}

static int app_active_request_process(int msgkey)
{
    APPMSG msg;
    int msgid = msgget(msgkey,777);

    msg.m_type = APP_MSG_TYPE;
    strcpy(msg.m_data,"OSAPP_ACK");
    sys_msgque_send(msgid, &msg, APP_MSG_LEN);
}


static void app_mifare_abort(void)
{
#ifndef QC430_v01
    if (g_cfg_basic.b_rfc_exist){

        OSDRV_Abort(DRVID_MIFARE);


    }
#endif
}

/* for mifare realy not exit,2011/6/23 */
static int app_mifare_set_exist(void)
{
    if (g_cfg_basic.b_rfc_exist){
        int fd = open("/dev/rc531", O_RDONLY);
        if (fd < 0){
            __printf("open mifare device fialed \n");
            g_cfg_basic.b_rfc_exist = 0;

            return -1;
        }
        close(fd);

        return 0;
    }
}

#if 0 //del by xl
unsigned char usbdisk_detect(void)
{
    int fd;

    fd = open("/dev/sda1", O_RDONLY);

    if (fd < 0)
        return 0;

    close(fd);
    return 1;
}

static void qt_status_printer_state(int s)
{
	struct lcd_messaage msg;
	if(s == 0)
		return;
	
    msg.mtype = 50;
    msg.msg_data.api_id = MSG_ID_PRINTER;
    msg.msg_data.gprs_d = s;
    if(msgsnd(lcd_message_id, &msg, 140, 0) < 0 ){
        perror("XXXXX message send");
		handle_error("msgsnd <qt_status_printer_state>");
    }
}

static int sdl_status_charge_connect(void)
{
    struct lcd_messaage msg;
    static int last_charge_state = -1;
    int charge_state ;

//    charge_state = charge_state_detect();
/*	charge_state = Os__get_device_signal(DEV_CHAG_BAR);
  

    if (charge_state == last_charge_state)
        return -1;

    last_charge_state = charge_state;
*/

    msg.mtype = 50;
    msg.msg_data.api_id = ID_CHRG_DETECT;
    msg.msg_data.gprs_d = charge_state;

    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
    {
        perror("message send");
		handle_error("msgsnd <sdl_status_charge_connect>");
    }
    return msg.msg_data.gprs_d;
}

static void sdl_status_charge_bar(int bar)
{
    struct lcd_messaage msg;

    msg.mtype = 50;
    msg.msg_data.api_id = ID_CHAG_BAR;
    msg.msg_data.gprs_d = bar;

    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
    {
        perror("message send");
		handle_error("msgsnd <sdl_status_charge_bar>");
    }
}

static void sdl_status_battery(void)
{
    struct lcd_messaage msg;

    msg.mtype = 50;
    msg.msg_data.api_id = ID_BATT_VALUE;

    msg.msg_data.gprs_d = Os__get_device_signal(DEV_BATT_VALUE);
    //_debug("\nXXXXXXXXXXXX DEV_BATT_VALUE: %d\n", msg.msg_data.gprs_d);

    if (msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 )
    {
        perror("message send");
		handle_error("msgsnd <sdl_status_battery>");
    }
}
#endif //old

/*
 * Send message to qlcd.
 * @id: api_id
 * @value: gprs_d
 * Add by xiangliu for using the same api to send msg to qlcd. 
 */
static void send_msg_qlcd(int id, int value)
{
	struct lcd_messaage msg;

    msg.mtype = 50;
	msg.msg_data.api_id = id;
    msg.msg_data.gprs_d = value;
    if(msgsnd(lcd_message_id, &msg, nbytes, 0) < 0 ){
        perror("XXXXX message send");
		handle_error("msgsnd <send_msg_qlcd>");
    }
}

static void sdl_status_sd(void)
{
    static int last_tf_state = -1;
    int tf_state ;

//    tf_state = tf_detect();
    tf_state = Os__get_device_signal(DEV_TF_DETECT);
    //_debug("\nXXXXXXXXXXXX DEV_TF_DETECT: %d\n", tf_state);

    if (tf_state == last_tf_state)
        return;

    last_tf_state = tf_state;

    send_msg_qlcd(ID_TF_DETECT, tf_state);
}

static void sdl_status_usbdisk(void)
{
    static int last_state = -1;
    int state ;

    state = Os__get_device_signal(DEV_USB_DETECT);

    if (state == last_state)
        return;

    last_state = state;

    send_msg_qlcd(ID_USBDISK_DETECT, state);
}

/* thread to refresh status bar for qlcd */
static	void qt_status_bar(void)
{
    int bar = 0;
    int ret = 0;
	int v_gps = 0;
	int v_gprs = 0;
	int v_battery = 0;
	int v_wifi = 0;
	int v_printer = 0;

	SYSGLOBAL_sys_poweron();
	
    do
    {
		/****************************************** 
		 *GPRS
		 * DEV_GPRS_DIAL : 0表示未拨上号,1表示拨号成功
		 * DEV_GPRS_SIGNALL:0~5,表示信号强度,其中0表示无卡，1表示无信号，2-5表示信号 9表示拨号进程没开 
		 */
    	ret = Os__get_device_signal(DEV_GPRS_DIAL);
		//_debug("\nXXXXXXXXXX DEV_GPRS_DIAL ret = %d\n", ret);
		
		if (ret == 0) { // dial failed.
			ret = Os__get_device_signal(DEV_GPRS_SIGNALL);
			//_debug("\nLLLLLLLLLLL DEV_GPRS_SIGNALL ret = %d\n", ret);
			if (ret == 0) { // no sim card.
				ret = -1;
			}else if (ret == 1 || ret == 9) {
				ret = 0;
			}
		} else {
    		ret = 10 + Os__get_device_signal(DEV_GPRS_SIGNALL); // signal value
			//_debug("\nLLLLLLLLLLL DEV_GPRS_SIGNALL ret = %d\n", ret);
		}    

		if (ret != v_gprs) {
			v_gprs = ret;
	    	send_msg_qlcd(MSG_ID_GPRS, ret);// GPRS 
    	}
	
		/******************************************
		 * GPS
		 * Add by xiangliu 2013.1.11
		 */
		ret = Os__get_device_signal(DEV_GPS); // get GPS signal
		//_debug("\nXXXXXXXXXX DEV_GPS = %d\n", ret);
		
		if (ret != v_gps) {
			v_gps = ret;
			
    		send_msg_qlcd(MSG_ID_GPS, ret);// GPS 
		}
		
        /******************************************
         * SD state 
         */
        sdl_status_sd();

        /******************************************
         * USB disk 
         */
        sdl_status_usbdisk(); // 12/15/2010 12:38p

		/******************************************
         * Printer state
         * 0表示正常,1表示缺纸,2表示打印机过热,3表示缺纸加过热
         */
		if (battery_times%2 == 0) {
	        ret = Os__get_device_signal(DEV_PRINTER_STATE);
    		//_debug("\nXXXXXXXXXXXX DEV_PRINTER_STATE: %d\n", ret);
   		    if (ret != v_printer) {
       		    v_printer = ret;

           		send_msg_qlcd(MSG_ID_PRINTER, ret);
        	}
		}

        /****************************************** 
         * charge and battery state 
		 * 3表示只有电池,2表示充电完成,1表示电池充电中,0表示只接入电源适配器
         * Modified by xiangliu 2013.1.24
         */
		int ret_dev = Os__get_device_signal(DEV_CHAG_BAR);
      	//_debug("\nXXXXXXXXXXXX DEV_CHAG_BAR: %d\n", ret_dev);

        if(ret_dev == 1) {
    		send_msg_qlcd(ID_CHAG_BAR, bar);
            bar++;
            if (bar > 4) bar = 0;
        } 
		else if (ret_dev == 3) {
            bar = 0;
			
			double battery_v = devSignal_battery_voltage(CHANNEL_LI_BATT);
			if (battery_v < 6.7 && Os__check_printBuf() != 0) { // low battery
				send_msg_qlcd(MSG_ID_LOW_BATTERY, CHANNEL_LI_BATT);
			}else {
   				int v = Os__get_device_signal(DEV_BATT_VALUE); // get battery value
     			//_debug("\nXXXXXXXXXXXX DEV_BATT_VALUE: %d\n", v);
				if (v != v_battery) {
					v_battery = v;
    				send_msg_qlcd(ID_BATT_VALUE, Os__get_device_signal(DEV_BATT_VALUE)); // get battery value
				}
   		    }
		}
	   	else if (ret_dev == 0) //xl
   		{
   			send_msg_qlcd(ID_CHAG_BAR, 0);
	    }
   		else if (ret_dev == 2) //xl
	    {
   			send_msg_qlcd(ID_CHAG_BAR, 4);
	    }		

		/******************************************
		 * Get button battery voltage for show MessageBox when low voltage.
		 * Add by xiangliu 2013.2.25
		 */
		battery_times++;
   		if (battery_times >= 3600) { // check low voltage one hour
            double battery_v = devSignal_battery_voltage(CHANNEL_CELL_BATT);
    		    //_debug("\n----Linuxpos Debug---- battery_voltage = %f\n", battery_v);
            if (battery_v < 2.2) { // low battery
   		        send_msg_qlcd(MSG_ID_LOW_BATTERY, CHANNEL_CELL_BATT);
           		if (battery_times == 0xfffffffe)
                    battery_times = 3600;
       	    } else {
          		battery_times = 0;
	        }
      	}

		/******************************************
         * Get wifi signal for qt statusbar
         * Add by xiangliu 2013.2.26
         */
        ret_dev = 0;
        ret_dev = Os__get_device_signal(DEV_WIFI);
        //_debug("\n----Linuxpos Debug---- DEV_WIFI = %d\n", ret_dev);
        
        if (ret_dev != v_wifi) {
			v_wifi = ret_dev;
			
        	send_msg_qlcd(MSG_ID_WIFI, ret_dev);
        }
#if 0
		/* for sleep bug */
		if (battery_times%5 == 0) { //<- FIXME
			send_msg_qlcd(111, 0);
		}
#endif
        usleep(1000000);
    }
    while (1);
}

/*
 * Get self start app
 */
static int get_start_app()
{
	SYS_SETTING setting;
	memset(&setting, 0, sizeof(setting));
	strcpy(setting.section, "MODULE_SETTING");
	strcpy(setting.key, "SELFSTART_APP_NO");
	if(PARAM_setting_get(&setting) < 0) {
		printf("----Linuxpos ERROR---- PARAM_setting_get SELFSTART_APP_NO error!\n");
		return -1;
	}
	if (setting.value) 
		return atoi(setting.value);
	else
		return 0;
}

/*调用sys_execl执行应用*/
static void sand_execl_app(void)//(int app_count)
{
	pid_t pid;;
    int num,retval,ret=0;
    APP_PARAM param;
    struct sigaction siga;
    char app_path[25];
	int app_count = g_app_cnt;
#ifdef TEST
	int k = 0;

	if(atoi((char *)applist_tmp[g_app_cnt].id) == 1)
		k++;
	if(atoi((char *)applist_tmp[g_app_cnt+1].id) == 2)
		k++;
#endif

    memset(&OSEvent,0,sizeof(OSEVENT_B));
    memset(&OSEventIn,0,sizeof(OSEVENTIN));
    memset(&OSEventOut,0,sizeof(OSEVENTOUT));
    OSEvent.pIn = &OSEventIn;
    OSEvent.pOut = &OSEventOut;
	OSEventOut.pucData = OSEventBuffer;
   
    siga.sa_handler = child_wait;
    siga.sa_flags  = 0;
    memset (&siga.sa_mask, 0, sizeof(sigset_t));
    sigaction (SIGCHLD, &siga, NULL);

    for(num=0; num<app_count; num++){//   这里的2步应该加
        strcpy( param.file,    g_app_list[num].app_name);          /* execute file */
        sprintf(param.shm_key,     "%d", shm_key);                 /* key of share memory */
        sprintf(param.mas_que_key, "%d", mas_que_key);             /* key of master massge queue  */
        sprintf(param.app_que_key, "%d", app_que_key[num]);        /* key of app massge queue  */
        
        sprintf(param.app_id,      "%d", g_app_list[num].app_id);  /* app_id  */
		
		memset(app_path, 0, 25);
		strcat(app_path, "/app/");
		strcat(app_path, applist_tmp[num].id);

        pid = vfork();
		if (pid == 0) { // child process
          	chdir(app_path);
            ret=sys_execl(param.file,param.shm_key,param.mas_que_key,param.app_que_key,param.app_id);            
        }
		else if (pid > 0) {
			g_app_list[num].child_id = pid;
			if (APPLIST_add_appid(atoi((char *)applist_tmp[num].id), pid) < 0) {
                _debug("----Linuxpos ERROR---- APPLIST_add_appid error!\n");
            }
		}
		else {
			perror("vfork error: ");
		}

        if (ret < 0) {
			printf("----Linuxpos ERROR---- Can not run the app with id %s\n", param.app_id);
            app_remove_process(g_app_cnt);
            system_reboot();
        }

#if 0 //del by xl 20130503
        (&OSEvent)->Type = OSEVENTTYPE_COM;
        (&OSEvent)->Index = OSEVENTID_POWERON;
   
        if (app_event_single(num, &OSEvent) < 0) {
			lcd_display(0, 0, "APP error! Please reboot system!");
			/* put it into g_app_dead_list */
			g_app_dead++;
			if (g_app_dead == 9) {
				g_app_dead = 0;
			}
			g_app_dead_list[g_app_dead] = pid;
			continue;
        }

        (&OSEvent)->Type = OSEVENTTYPE_CIR;
        (&OSEvent)->Index = OSEVENTID_CUSTOMER;
        ucOSCurrAppID = g_app_list[num].app_id;
        ucOSMasterAppID = ucOSCurrAppID;
       
        //app_event_single(num,&OSEvent);
		if (app_event_single(num, &OSEvent) < 0) {
			lcd_display(0, 0, "APP error! Please reboot system!");
			/* put it into g_app_dead_list */
			g_app_dead++;
			if (g_app_dead == 9) {
				g_app_dead = 0;
			}
			g_app_dead_list[g_app_dead] = pid;
			continue;
		}

        if((&OSEvent)->pOut->ucStatus == OSEVENT_PROCESSED){
            retval=OSEVENT_PROCESSED;
            break;
        }

        (&OSEvent)->Type = OSEVENTTYPE_CIR;
        (&OSEvent)->Index = OSEVENTID_MASTERAPPLI;
        ucOSCurrAppID = g_app_list[num].app_id;
        ucOSMasterAppID = ucOSCurrAppID;
        app_event_single(num,&OSEvent);

        if((&OSEvent)->pOut->ucStatus == OSEVENT_PROCESSED){
            retval=OSEVENT_PROCESSED;
            break;
        }
#endif

        ucOSCurrAppID = 0;
        ucOSMasterAppID = 0;
    } // end for
//    return ret;
}

/*
 * Start app if any self start app
 */
static void start_selfstart_app()
{	
	int sapp = get_start_app();
	if (sapp > 0) {
		int i;
		unsigned char OSEventBuffer[MAXAPPSHAREBUF];
		OSEVENT_B OSEvent;
		OSEVENTIN   OSEventIn;
		OSEVENTOUT  OSEventOut;
		OSEvent.pIn = &OSEventIn;
	    OSEvent.pOut = &OSEventOut;
		OSEventOut.pucData = OSEventBuffer;
       	(&OSEvent)->Type = OSEVENTTYPE_SEL;
        (&OSEvent)->Index = 0xFF;
		for(i=0; i<g_app_cnt; i++) {
			if (g_app_list[i].app_id == sapp)
				break;
		}
		if (i<g_app_cnt) {
			send_msg_qlcd(MSG_ID_APP, 1);
        	ucOSCurrAppID = sapp;
	        ucOSMasterAppID = ucOSCurrAppID;
			current_app_id = i;
	       	app_event_single(i, &OSEvent);
		}
	}
}

/*
 * Get SLEEP_TIME from setting.ini to sleep when timeout.
 * Return sleep timeout time(ms).
 * Add by xiangliu 2013-1-6.
 */
static int get_sleep_time()
{
	return 90000;
    unsigned int etcFile;
    unsigned int etcSection1;
    char v[10];
    int sleep_time;

	memset(v, 0, 10);

    etcFile = Os__param_loadFullPath("/daemon/setting.ini");
    etcSection1 = Os__param_findSection(etcFile,"SCREEN_SETTING", 0);
    Os__param_get(etcSection1, "SLEEP_TIME", v, 10);
	Os__param_unload(etcFile);

    sleep_time = atoi(v) * 1000;
#if 0
    Os__param_get(etcSection1, "SCREENSAVER_TIME", v, 10);
    SCREENSAVER_TIME = atoi(v) * 1000;

    if (SCREENSAVER_TIME <= 0) {
        perror("get_sleep_time error\n");
        SCREENSAVER_TIME = 10000000;
    }
#endif

    if (sleep_time < 10000) {
        perror("get_sleep_time error\n");
        sleep_time = 10000; //10s
    }
	return sleep_time;

#if 0
	return (SLEEP_TIME < SCREENSAVER_TIME) ? SLEEP_TIME : SCREENSAVER_TIME;	
	if (SLEEP_TIME > SCREENSAVER_TIME) {
		sleep_time = SLEEP_TIME - SCREENSAVER_TIME;
	}
#endif
}

int main(int argc, char *argv[])
{
//	int SLEEP = 0;
    int ret = 0;
    volatile unsigned int uiTimeout;
    int f_ipcs;
    static int fstLoadApp = 0;
    pthread_t tid;
    NEW_DRV_TYPE  new_drv;
	bool Wait_Event = true;
	bool RESTART = true;

	g_app_dead = 0;
	
    if( (argc == 2) && !strncmp(argv[1] , "dbg=",4) && (argv[1][4] == '0' || argv[1][4] == '1') ) {
    	DEBUG = argv[1][4] - 0x30;
    } else {
		DEBUG = 0;
	}

    signal(SECURITY_SIG, SIG_IGN);
    timer_init();
	lcd_init();

    int shm_id;
    int *shm_addr;
    
	int sleep_timeout = get_sleep_time();//xl   
    app_sigaction(); // install signal handle

    main_state = MAIN_INIT;
    f_ipcs = LINUXPOS_FROM_REBOOT;

	/*
	 * Add by xiangliu 20130409
	 */
	if (Os__get_device_signal(DEV_DEFAULT_DATE) == 1) {
		send_msg_qlcd(MSG_ID_SETDATE, 1);
		while (Os__get_device_signal(DEV_DEFAULT_DATE));
	}

    if (pthread_create(&tid,NULL,qt_status_bar,NULL)!=0)//״̬??qt_status_bar
    {
        printf("----Linuxpos ERROR---- Create thread error!\n");
    }

    while (1) {
        switch (main_state) {
        case MAIN_INIT:
            g_app_cnt = 0;

            if(check_MFI_AUTO()==1)
                app_mifare_set_exist();
            main_state = MAIN_LIST_APPS;

#ifndef QC430_v01
            ret=security_setup();

            if(ret==0)
            {

                security_del(1);
                OSMMI_DisplayASCII(0x30, 1, 0,"ILLEGAL OPERATE");
                while(1){}
            }
#endif           
            break;

        case MAIN_LIST_APPS:
		{				
				app_listting();//从配置文件中获得app信息
				
		        if(fstLoadApp == 0)
		        {
		            //lcd_clear(255);
		            //lcd_display(0,7,"WELCOME"); //xl del
		            lcd_display(1,3,"APP LOADING...");
		        }
		        
//		        app_check_data_dir();// create app's data path
	            nice(+10);
	            main_state = MAIN_LOAD_APPS;
#if 0        
		        if (!g_app_cnt) {
		            lcd_clear(255);

		            lcd_display(2,0,"NO APPLICATIONS");
		            lcd_display(3,0,"PLS DOWNLOAD");

		            sleep(1);
		            main_state = MAIN_CONFIG_NOAPP;
		        } else {
		            app_check_data_dir();// create app's data path
		            nice(+10);
		            main_state = MAIN_LOAD_APPS;
		        }
#endif		        
            }	
            break;
            
        case MAIN_LOAD_APPS:
        {
        	app_create_ipcs(g_app_cnt,f_ipcs);
	    	if (g_app_cnt)
	            sand_execl_app();//启动应用程序

            ucMenuPage = 0;
            uiTimeout = 0;
            ucCount = 0;
            timer_Internal_start(&uiTimeout,NULL);
            main_state = MAIN_DISP_MENU;
        }
            break;

        case MAIN_DISP_MENU:
//       		chdir("/daemon");
            app_menu_display(ucMenuPage);//通过发消息给qlcddaemon来显示主界面
			if (RESTART) { //start_selfstart_app only once
				start_selfstart_app();//xl add 20130620
				RESTART = false;
			}
            main_state = MAIN_DEV_STATUS;
            break;

        case MAIN_DEV_STATUS:     
        	
//            chdir("/daemon");//??????chdir?????ĵط???Ҫ????4?ı?λ?ã???ΪӦ?õ?λ?ú?linuxpos?ȷŵ?λ?ò?һ??
#ifdef xl			
        	if(&OSEvent != NULL && current_app_id == -1 && ((struct seven *)&OSEvent)->pt_eve_out && ((struct seven *)&OSEvent)->pt_eve_out->xxdata && strlen((char *)((struct seven *)&OSEvent)->pt_eve_out->xxdata) > 1)//??4????Ӧ???д??ݳ?4????Ϣ??????Ӧ??
        	{        	
        		REGISTER_FLAG = TRUE;
        		app_event_circle(EVE_CASH_REGISTER,&OSEvent);
				memset(((struct seven *)&OSEvent)->pt_eve_in->e_cash_register.ecr_msg, 0, strlen((char *)((struct seven *)&OSEvent)->pt_eve_in->e_cash_register.ecr_msg));
				memset(((struct seven *)&OSEvent)->pt_eve_out->xxdata, 0, strlen((char *)((struct seven *)&OSEvent)->pt_eve_out->xxdata));

        		main_state = MAIN_DISP_MENU;
        		break;
        	}
        	else     	
#endif				
        	{
				memset(&new_drv, 0, sizeof(NEW_DRV_TYPE));
				if (Wait_Event) {
					
					/*
					 * Send msg to qlcd: exit app (can sleep)
					 */
					send_msg_qlcd(MSG_ID_APP, 0);
					
        			_debug("----Linuxpos Debug---- before Os_Wait_Event_NoKey all event\n");
        			//最后一个参数为时间参数，1000为一秒，用来接收底层传递上来的各种信息，如果在没有底层消息的时候会停在该处
		            ret = Os_Wait_Event_NoKey(KEY_DRV | MAG_DRV | ICC_DRV| MFR_DRV, &new_drv, 300000);//5minutes //sleep_timeout);
				}
				else {
        			_debug("----Linuxpos Debug---- before Os_Wait_Event_NoKey no event\n");
		            ret = Os_Wait_Event_NoKey(KEY_DRV, &new_drv, 9999999);
				}
	        }
			if (ret != DRV_EVENT_SUCCESS) { // Os_Wait_Event_NoKey error. xl add 03.2.4
				switch(ret) {
					case KEY_DRV_ERROR:
						_debug("----Linuxpos get ERROR after Os_Wait_Event_NoKey: KEY_DRV_ERROR\n");
						break;
					case MAG_DRV_ERROR:
						_debug("----Linuxpos get ERROR after Os_Wait_Event_NoKey: MAG_DRV_ERROR\n");
						break;
					case MFR_DRV_ERROR:
						_debug("----Linuxpos get ERROR after Os_Wait_Event_NoKey: MFR_DRV_ERROR\n");
						break;
					case ICC_DRV_ERROR:
						_debug("----Linuxpos get ERROR after Os_Wait_Event_NoKey: ICC_DRV_ERROR\n");
						break;
					default:
						_debug("----Linuxpos get ERROR after Os_Wait_Event_NoKey: %x\n", ret);
				}
				continue;
			}

			_debug("----Linuxpos Debug---- after Os_Wait_Event_NoKey\n");

			/*以下是打印收到哪种消息*/
			if(new_drv.drv_type == KEY_DRV)
			{
				unsigned char key = new_drv.drv_data.xxdata[1];
				_debug("----Linuxpos Debug---- KEY_DRV. key=%x\n", key);

                if(key == KEY_SYS_TEST || key == KEY_SYS_SET)
				{
					Wait_Event = false;
					MAIN_FORM = false;
                	main_state = MAIN_DEV_STATUS;
					break;
				}
                else if(key == KEY_SYSWIN_EXIT) // when TEST/SET exit
				{
					Wait_Event = true;
					MAIN_FORM = true;
                	main_state = MAIN_DEV_STATUS;
					break;
				}
                else if(key == KEY_UP)
                {                 
                    if(ucMenuPage > 0)
                        ucMenuPage--;
                	main_state = MAIN_DEV_STATUS;
					break;
                }
                else if(key == KEY_DOWN)
                {
                 	if((ucMenuPage + 1)*9 < g_app_cnt)
                        ucMenuPage++;
                	main_state = MAIN_DEV_STATUS;
					break;
                }
			}
			else if(new_drv.drv_type == MAG_DRV)
			{
			_debug("----Linuxpos Debug---- MAG_DRV\n");
			} 
			else if(new_drv.drv_type == MFR_DRV)
			{
			_debug("----Linuxpos Debug---- MFR_DRV\n");
			} 
			else if(new_drv.drv_type == ICC_DRV)
			{
			_debug("----Linuxpos Debug---- ICC_DRV\n");
			} 
			/*else if(new_drv.drv_type == APP_MSG) //xl test for APP_MSG
			{
				_debug("\n----LINUXPOS DEBUG---- get SIG [APP_MSG]\n");
			}*/
			else if(new_drv.drv_type == DRV_TIMEOUT)
			{
				_debug("----Linuxpos Debug---- DRV_TIMEOUT\n");
 			}

            uiTimeout = LIGHT_OFF_TM*7;

			
            if (( shm_id=shmget( CURRENT_APP_SHM_ID,8096,IPC_CREAT|0666 ))<0 ){
                perror("### shmget failed.");
				handle_error("shmget");
            	main_state = MAIN_DISP_MENU;
				break;
            } 
            shm_addr = shmat(shm_id,NULL,0);
   	        memcpy(&current_app, shm_addr,sizeof(int));
           
       	    if(current_app == 1)/*判断是否在主界面下*/
           	    current_app_id = -1;
			else if (current_app != 1 && MAIN_FORM) { //<-FIXME
				current_app = 1;
           	    current_app_id = -1;
			}

	        shmdt(shm_addr);

			
			/*
			 * Send msg to qlcd: enter app (can not sleep)
			 */
			send_msg_qlcd(MSG_ID_APP, 1);

			/* Handle event... */

            if (new_drv.drv_type == KEY_DRV) 
			{
                app_mifare_abort();
                main_state = MAIN_KEY_EVENT;
            }
			else if (new_drv.drv_type == DRV_TIMEOUT) 
			{
#if 0 
				if (current_app == 1) {
					// sleep
					SYSGLOBAL_sys_suspend();
				}
#endif
                //xl app_mifare_abort();
            	main_state = MAIN_DEV_STATUS;
			}
            /*以下是判断底层的各种消息对应的处理*/
            else if (new_drv.drv_type == MAG_DRV) 
			{
                main_state = MAIN_MAG_EVENT;
            }
            else if(new_drv.drv_type == ICC_DRV)
            {
                    app_mifare_abort();
                    main_state = MAIN_ICC_EVENT;
            }
				/*if(new_drv.drv_type == APP_MSG) //xl test for APP_MSG
				{
					_debug("\n----LINUXPOS DEBUG---- handle [APP_MSG]\n");
					if (app_circle_event() < 0) {
						_debug("\n----LINUXPOS ERROR---- app_circle_event error\n");
					}
					break;
				}*/
                //if (g_cfg_basic.b_rfc_exist){
            else if (new_drv.drv_type == MFR_DRV)
			{
                main_state = MAIN_MIF_EVENT;
            }
            else {
                OSDRV_Abort(DRVID_MIFARE);
                Os__MIFARE_PowerRF(0);

                usleep(50000);
            }

            break;

        case MAIN_MAG_EVENT:
			if (!g_app_cnt) {
				main_state = MAIN_DEV_STATUS;
				break;
			}

            app_mag_event(&new_drv.drv_data);
            main_state = MAIN_DISP_MENU;
            break;

        case MAIN_ICC_EVENT:
			if (!g_app_cnt) {
				main_state = MAIN_DEV_STATUS;
				break;
			}
            app_icc_event(&new_drv.drv_data);
            main_state = MAIN_DISP_MENU;

            break;

        case MAIN_MIF_EVENT:
			if (!g_app_cnt) {
				main_state = MAIN_DEV_STATUS;
				break;
			}
            app_mifare_event(&new_drv.drv_data);
            main_state = MAIN_DISP_MENU;

            break;

        case MAIN_KEY_EVENT:
            /*if((current_app) < 0)
            {
                main_state = MAIN_DISP_MENU;
                break;
            }*/
            main_state = app_key_event(&new_drv.drv_data);
            break;

        case MAIN_TIMEOUT:
            main_state = MAIN_DISP_MENU;
            break;

        case MAIN_CONFIG:
            app_remove_process(g_app_cnt);
            f_ipcs = LINUXPOS_FROM_CONFIG;
            main_state = MAIN_INIT;
            break;

        case MAIN_CONFIG_NOAPP:
            main_state = MAIN_INIT;
            break;

        default:
            break;
        }//switch(state)
        
    }//while(1)
    return 0;
}
/////////////////////  END ////////////////////////////////
