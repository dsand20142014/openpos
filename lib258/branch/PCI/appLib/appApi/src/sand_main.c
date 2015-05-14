/******************************************************************
*  This main file would be added for application file which started at manager
******************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include <sand_main.h>
#include <osdrv.h>
#include <osevent.h>
#include <sand_timer.h>
#include <sand_lcd.h>
#include <oscfg_param.h>
#include <sand_debug.h>
//#include <sand_fileops.h>
#include <sand_print.h>

#ifdef TMS
#include "apptms.h"
#endif

#define SHM_TEST	 0
/*
 * For apps communication. Add by xiangliu 03.2.4
 */
#define ACCEPT 0
#define REQUEST 1

/* functions */
extern void manager(struct seven* );

static int main_read_msg(int qid, uchar mbuf);
#ifdef NOPCI
static int main_read_event(key_t shm_key , seven *pevent);
static int main_write_event(key_t shm_key , seven *pevent);
#endif
static int main_complete(int qid, uchar mbuf);
static int qt_main_entry_readmsg_app(unsigned short r);
static int qt_main_entry_writemsg_app(unsigned short r, struct seven *sevent);
static void main_printer_idle(void);


/* variables */
OSEVENT  event;
#ifdef NOPCI
seve_in  event_in; 
seve_out event_out;
#endif

//unsigned char OSEventOut_Buffer[MAXAPPSHAREBUF];

/* ipc key */
int shm_key;
int mas_que_key;
int app_que_key;

/*ipc id */
int mas_que_id;
int app_que_id;

static unsigned char EVENT_ACCEPT = 0;
static int g_app_id;
extern unsigned short ucOSCurrAppID;

/*
 * Add msg to strncmp when read, msg maybe 'REQUEST' or 'ACCEPT'
 * by xiangliu 03.1.30
 */
static int qt_main_entry_readmsg_app(unsigned short r) 
{
	int ret = 0;

	if ((app_que_id = msgget(app_que_key,777))<0){
		perror("### msgget fail.\n");
		return -1;
	}

	if (r)
		ret = main_read_msg(app_que_id, 'R'); //REQUEST
	else 
		ret = main_read_msg(app_que_id, 'A'); //ACCEPT

#ifdef NOPCI

	if (ret < 0) { // no available msg 
		return -1;
	}
	
	event.pt_eve_in= &event_in;
	event.pt_eve_out= &event_out;
	
	if (main_read_event(shm_key, &event) < 0) {
		return -1;
	}
	
//	set_app_id(g_app_id);
	event.pt_eve_out.xxdata = OSEventOut_Buffer; // prepare for return data to linuxpos
	
	return   &event;
#else
	return ret;
#endif	
}

/*
 * Add msg to write, msg maybe 'REQUEST' or 'ACCEPT'
 * by xiangliu 03.1.30
 */
static int qt_main_entry_writemsg_app(unsigned short r, struct seven *sevent)  
{
	int ret = 0;

	if ((mas_que_id = msgget(mas_que_key,777))<0){
		perror("### msgget fail in <qt_main_entry_writemsg_app>.\n");
		return(-1);
	}
	
#ifdef NOPCI
	if (main_write_event(shm_key, (OSEVENT_B *)sevent) < 0)
		return -1;
#else
	memcpy(&event, sevent, sizeof(event));
	event.pt_eve_out.xxdata[MAXAPPSHAREBUF-1] = 0;
#endif

	if (r)
		ret = main_complete(mas_que_id, 'R');
	else {
		ret = main_complete(mas_que_id, 'A');
		EVENT_ACCEPT = 1;
	}

	return ret;
}

/***
 * Write REQUEST msg to linuxpos and read ACCEPT from other APP.
 * Return (OSEVENT *) the share memory addr for APP, or -1 when error.
 * Add by xiangliu 2013.2.4
 */
int OS_Event_Run(struct seven *sevent, struct seven *sevent_out)
{
	//if (!sevent || !sevent->pt_eve_out || !sevent_out || !(sevent_out->pt_eve_in) || !(sevent_out->pt_eve_in.e_cash_register.ecr_msg)) {
	if (sevent == NULL || sevent_out == NULL) {
		printf("OS_Event_Run: sevent/sevent_out is NULL!");
		return -1;
	}
	//printf("========== addr: %p\n", sevent_out);
	sevent->pt_eve_out.treatment_status = NOK;

	if(qt_main_entry_writemsg_app(REQUEST, sevent) < 0) { // send msg
        return -1;
    }

    int ret = qt_main_entry_readmsg_app(ACCEPT); //rev msg
	if (ret < 0)
		return -1;
	else {
#ifdef NOPCI
		memcpy(sevent_out, (struct seven *)ret, sizeof(sevent_out));
		memcpy(sevent_out->pt_eve_in.e_cash_register.ecr_msg, ((struct seven *)ret)->pt_eve_in.e_cash_register.ecr_msg, MAXAPPSHAREBUF);
#else		
		
		
		//printf("========== addr2: %x\n", sevent_out->pt_eve_in.e_cash_register.ecr_msg);
		if (sevent_out)
			memcpy(sevent_out->pt_eve_in.e_cash_register.ecr_msg, event.pt_eve_in.e_cash_register.ecr_msg, MAXAPPSHAREBUF-1);
		else
			printf("OS_Event_Run ERROR: addr: %x\n", sevent_out);
#endif
		//printf("End OS_Event_Run:\n%s\n---------\n", sevent_out->pt_eve_in.e_cash_register.ecr_msg);
		return 0;
	}
}

/***
 * Write ACCEPT msg and sevent to linuxpos.
 * Return 0 on success or -1 when error.
 * Add by xiangliu 2013.2.4
 */
int OS_Event_Accept(struct seven *sevent)
{
	if (sevent == NULL) {
		printf("OS_Event_Accept: sevent is NOT valid!");
		sevent = &event; //<-FIXME
	}
	sevent->pt_eve_out.treatment_status = STATUS_ACCEPT;

	return qt_main_entry_writemsg_app(ACCEPT, sevent);
}


void qt_main_entry_init(int agrc,char *argv[])  
{
	shm_key     = atoi(argv[1]);
	mas_que_key = atoi(argv[2]);
	app_que_key = atoi(argv[3]);
	g_app_id      = atoi(argv[4]);
	
	//timer_init();
	OSDRV_init();
	lcd_init();
	//OSDRV_install(DRVID_MMI,&gbl_mmi_op);
}

int qt_main_entry_readmsg() 
{

	if ((app_que_id = msgget(app_que_key,777))<0){
		perror("### msgget fail.\n");
		exit(-1);
	}
#ifdef NOPCI	
	event.pt_eve_in= &event_in;
	event.pt_eve_out= &event_out;
#endif
	main_read_msg(app_que_id, 0);
#ifdef NOPCI	
	main_read_event(shm_key, &event);
	event.pt_eve_out.xxdata = OSEventOut_Buffer; // prepare for return data to linuxpos
#endif

	set_app_id(g_app_id);
	
	return   &event;
}

void qt_main_entry_writemsg()  
{
	if (EVENT_ACCEPT) {
		EVENT_ACCEPT = 0;
		return;
	}
	if ((mas_que_id = msgget(mas_que_key,777))<0){
		perror("### msgget fail.\n");
		printf(" %s -------------------error\n", __func__);
		exit(-1);
	}
#ifdef NOPCI
	main_write_event(shm_key, &event);
#endif	
//	memset(((struct seven *)&event)->pt_eve_in->e_cash_register.ecr_msg, 0, strlen(((struct seven *)&event)->pt_eve_in->e_cash_register.ecr_msg));
//	memset(((struct seven *)&event)->pt_eve_out.xxdata, 0, strlen(((struct seven *)&event)->pt_eve_out.xxdata));
	
	main_complete(mas_que_id, 0);
}

/*
 * write status 'OK' to event
 * Add by xiangliu for PCI because no share memory
 */
void OS_Event_OK()
{
	event.pt_eve_out.treatment_status = OK;
}
 
/**
 ** entry of non-gui application
 **/
int main_entry(int agrc,char *argv[])  
{ 
	int app_id;
	int retval=0;
	
	shm_key     = atoi(argv[1]);
	mas_que_key = atoi(argv[2]);
	app_que_key = atoi(argv[3]);
	app_id      = atoi(argv[4]);

	//timer_init();
	OSDRV_init();
	lcd_init();
	//OSDRV_install(DRVID_MMI,&gbl_mmi_op);

	if ((mas_que_id = msgget(mas_que_key,777))<0){
		perror("### msgget fail.\n");
		exit(-1);
	}

	if ((app_que_id = msgget(app_que_key,777))<0){
		perror("### msgget fail.\n");
		exit(-1);
	}
	
#ifdef NOPCI
	event.pt_eve_in= &event_in;
	event.pt_eve_out= &event_out;
#endif
	g_app_id = app_id;
	
//	OSCFG_decide_debug();
	
	
	while(1)
	{
		main_read_msg(app_que_id, NULL);
#ifdef NOPCI		
		main_read_event(shm_key, &event);
		event.pt_eve_out.xxdata = OSEventOut_Buffer; // prepare for return data to linuxpos

#endif		
		set_app_id(app_id);
		
		manager((struct seven *)&event); 
		
#ifdef NOPCI
		main_write_event(shm_key, &event);
#endif
		main_complete(mas_que_id, NULL);
	}
	return retval;
}

/**
 ** entry of MiniGUI application
 **/
int main_entry_gui(int agrc,char *argv[])  
{ 

//OSDRV_install(DRVID_MMI,&gbl_mmi_op);

return 1;
}

/*
 * Add mbuf to strncmp msg and 'HELLO' or 'ACCEPT' means it's apps msg
 * by xiangliu 03.1.30
 */
static int main_read_msg(int qid, uchar mbuf)
{
    int nbytes, ret = -1;
	APPMSG msg;
	uchar d;
	
	while(1) {
		nbytes = msgrcv(qid ,&msg,APP_MSG_LEN,APP_MSG_TYPE,0);
		if (nbytes<0){
			perror("### msgrcv fail.");
			return (-1);
		}
		d = msg.m_data.pt_eve_out.download;		
				
		if (mbuf) {//ACCEPT/REQUEST
			if (mbuf == d)
				ret = 0;
			else if (d == 'N') //NOACCEPT
				return -1;			
		}
		else {
			if (d == 'H')
				ret = 0;
			else
				ret = -1;
		}
		if (ret == 0) //match
			break;

		else if (d == 'E'){ //EXIT
		    /* remove message queue */
		   	if (msgctl(qid,IPC_RMID,NULL)<0){
				Uart_Printf(" EXIT %s msgctl fail.\n",__func__);
				exit(-1);
			} 
			/* process exit */
		    exit(0);
		}
	}

	memcpy(&event, &msg.m_data, sizeof(OSEVENT));//xl add for no sharememory 20130509
	printf("=====%c====\n", d);
	//printf("============out:\n%s\n", event.pt_eve_out.xxdata);
	event.pt_eve_out.treatment_status = NOK;
	return nbytes;
}

#ifdef NOPCI
static int main_read_event(key_t shm_key , OSEVENT_B *pevent)
{
	unsigned char *event_addr;
	unsigned int offset=0;
	int retval=0;
	int shm_id;
	
	
	if (( shm_id=shmget(shm_key,APP_SHM_SIZE,777|IPC_CREAT))<0 ){
		perror("### shmget failed.");
		return (-1);
	}

	event_addr = shmat(shm_id,NULL,0);
	//Uart_Printf( "### event_addr = %d \n",(unsigned int)event_addr);

	/* Type  */
	memcpy(&pevent->Type, event_addr+offset, 1);
	offset += 1;

	/* Index */
	memcpy(&pevent->Index,  event_addr+offset, 1);
	offset += 1;

	/* EventIn */
	memcpy(pevent->pIn, event_addr+offset,sizeof(OSEVENTIN));
	offset += sizeof(OSEVENTIN);

	/* EventOut */
	memcpy(pevent->pOut,event_addr+offset,sizeof(OSEVENTOUT));
	offset += sizeof(OSEVENTOUT);
	
#ifdef TMS	
	if (pevent->Index == OSEVENTID_PAMLOAD)
	{
		gTmsIPC.ucType = *(event_addr + offset);
		offset++;
		gTmsIPC.uiLen = (unsigned short)CONV_AscLong(event_addr + offset, 3);
		offset += 3;
		memset(gaucUpdate, 0, sizeof(gaucUpdate));
		gTmsIPC.pucPtr = gaucUpdate;
		memcpy(gTmsIPC.pucPtr, event_addr + offset, gTmsIPC.uiLen);
		offset += gTmsIPC.uiLen;

#if SHM_TEST
		{
			int i;
			Uart_Printf("#############app read from linuxpos TMSIPC len = %d\n", gTmsIPC.uiLen);
			Uart_Printf("gTmsIPC.ucType = %02x\n", gTmsIPC.ucType);
			Uart_Printf("gTmsIPC.uiLen = %d\n", gTmsIPC.uiLen);
			Uart_Printf("gTmsIPC.pucPtr:\n");
			for (i = 0; i < gTmsIPC.uiLen; i++)
			{
				Uart_Printf("%02x ", gTmsIPC.pucPtr[i]);
			}
			Uart_Printf("\n-----------------------------------------\n");
			Uart_Printf("event_addr + offset:\n");
			for (i = 0; i < gTmsIPC.uiLen + 4; i++)
			{
				Uart_Printf("%02x ", *(event_addr + offset - 4 - gTmsIPC.uiLen + i));
			}
			Uart_Printf("\n-----------------------------------------\n");
		}
#endif
	}
#endif	
	shmdt(event_addr);

	return retval;
}

static int main_write_event(key_t shm_key, OSEVENT_B *pevent)
{
	unsigned char *event_addr;
	unsigned int offset=0;
	int retval=0;
	int shm_id;
	

	if (( shm_id=shmget(shm_key,APP_SHM_SIZE,777|IPC_CREAT))<0 ){
		perror("### shmget failed.");
		return (-1);
	}
	
	event_addr = shmat(shm_id,NULL,0);

	/* ucStatus  */
	memcpy(event_addr + offset, &pevent->pOut->ucStatus, 1);
	offset += 1;

	/* ucDownload  */
	memcpy(event_addr + offset, &pevent->pOut->ucDownload, 1);
	offset += 1;

	/*AppStatus */
	memcpy(event_addr + offset, &pevent->pOut->ucAppStatus, 1);
	offset += 1;

	/* pucData */
	if ( pevent->pt_eve_out.xxdata ){  
		memcpy(event_addr + offset, pevent->pt_eve_out.xxdata, MAXAPPSHAREBUF);
		offset += MAXAPPSHAREBUF;
	}
	
#ifdef TMS
	/* pucData  */
	if (pevent->Index == OSEVENTID_PAMLOAD)
	{
		*(event_addr + offset) = gTmsIPC.ucType;
		offset++;
		short_asc(event_addr + offset, 3, &(gTmsIPC.uiLen));
		offset += 3;
		memcpy(event_addr + offset, gTmsIPC.pucPtr, gTmsIPC.uiLen);
		offset += gTmsIPC.uiLen;

#if SHM_TEST
		{		
			int i;
			Uart_Printf("#############app write to linuxpos TMSIPC len = %d\n", gTmsIPC.uiLen);
			Uart_Printf("event_addr + offset:\n");
			for (i = 0; i < gTmsIPC.uiLen + 4; i++)
			{
				Uart_Printf("%02x ", *(event_addr + offset - 4 - gTmsIPC.uiLen + i));
			}
			Uart_printf("\n-----------------------------------------\n");
			Uart_Printf("gTmsIPC.ucType = %02x\n", gTmsIPC.ucType);
			Uart_Printf("gTmsIPC.uiLen = %d\n", gTmsIPC.uiLen);
			Uart_Printf("gTmsIPC.pucPtr:\n");
			for (i = 0; i < gTmsIPC.uiLen; i++)
			{
				Uart_Printf("%02x ", *(gTmsIPC.pucPtr + i));
			}
			Uart_Printf("\n-----------------------------------------\n");
		}
#endif		
	}
#endif	
	shmdt(event_addr);

	return retval;
}
#endif

static int main_complete(int qid, uchar buf)
{
    APPMSG msg;
	int retval=0;
	
 	msg.m_type = APP_MSG_TYPE;
	memcpy(&msg.m_data, &event, sizeof(msg.m_data));//xl add for no sharememory 20130509
	
	if(buf)
		msg.m_data.pt_eve_out.download = buf;
	else
		msg.m_data.pt_eve_out.download = 'D'; //DONE
	
	retval = msgsnd(qid, &msg, APP_MSG_LEN, 0);
	if (retval<0){
		perror("### msgsnd fail.\n");
		return (-1);
	}
	return retval;
}

int main_get_app_id(void)
{
    return g_app_id;
}

static void main_printer_idle(void)
{
	while(!Os__printer_idle());
}


/******************************************************************************
*   END ADDED
*******************************************************************************
*/

