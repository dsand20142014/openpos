#ifndef __SAND_MAIN_H
#define __SAND_MAIN_H 

#ifdef __cplusplus
extern "C" {
#endif
#include <osevent.h>

#define APP_MSG_TYPE        100
#define APP_MSG_LEN         (sizeof(APPMSG))
#define APP_SHM_SIZE        8096
#define STATUS_ACCEPT		5 //xl add 03.2.4 

#if 0
typedef enum
{
    OSEVENTTYPE_CIR,
    OSEVENTTYPE_COM,
    OSEVENTTYPE_SEL,
    OSEVENTTYPE_GRP
}OSEVENTTYPE;

typedef enum
{    
    OSEVENTID_MAGCARD,
    OSEVENTID_ICC,
    OSEVENTID_MFCCRD,
    OSEVENTID_CIRCLE,
    OSEVENTID_AWAKE,
    OSEVENTID_REF01,
    OSEVENTID_REF02,
    OSEVENTID_REF03,
    OSEVENTID_REF04,
    OSEVENTID_REF05,
    OSEVENTID_REF06,
    OSEVENTID_REF07,		
    OSEVENTID_REF08,
    OSEVENTID_REF09,
    OSEVENTID_REF10,
    OSEVENTID_REF11,
    OSEVENTID_REF12,
    OSEVENTID_REF13,
    OSEVENTID_REF14,
    OSEVENTID_REF15,
    OSEVENTID_REF16,
    OSEVENTID_REF17,		
    OSEVENTID_REF18,
    OSEVENTID_REF19,
    OSEVENTID_REF20,
    OSEVENTID_REF21,
    OSEVENTID_REF22,
    OSEVENTID_REF23,	
}OSEVENTID;

typedef struct
{
    unsigned char   ucSign;
    unsigned char   aucAmount[9];
}OSEVENTAMOUNT;

typedef struct
{
    OSEVENTAMOUNT   Amount;
    unsigned char   aucData[MAXAPPSHAREBUF];
}OSEVENTMAGCARD;

typedef struct
{
    OSEVENTAMOUNT   Amount;
    unsigned char   aucAtr[MAXAPPSHAREBUF];
}OSEVENTICC;

typedef struct
{
    OSEVENTAMOUNT   Amount;
    unsigned char   aucData[MAXAPPSHAREBUF];
}OSEVENTMAN;


typedef union
{
    OSEVENTMAGCARD  Mag;
    OSEVENTICC      Icc;
    OSEVENTMAN      Man;
	unsigned char   CashRegister[MAXAPPSHAREBUF];
}OSEVENTIN;

typedef struct
{
    unsigned char   ucStatus;
    unsigned char   ucDownload;
    unsigned char   ucAppStatus;
    unsigned char   *pucData;
}OSEVENTOUT;

typedef struct
{
    OSEVENTAMOUNT   Amount;
    unsigned char   aucData[MAXAPPSHAREBUF];
}OSEVENTDATA;

typedef struct
{
    OSEVENTTYPE     Type;
    OSEVENTID       Index;
    OSEVENTDATA     Data;
}OSEVENT_B;
#endif

typedef struct {
	int  m_type;
	struct seven m_data;
}APPMSG;

int main_get_app_id(void);
int main_entry(int agrc,char *argv[]);
int main_entry_gui(int agrc,char *argv[]);
void qt_main_entry_init(int agrc,char *argv[]) ;
int qt_main_entry_readmsg();
void qt_main_entry_writemsg();

/***
 * Write message to linuxpos and wait for back message that is from other responsed APP.
 * @sevent_in: struct seven that will circle to other app.
 * @sevent_out: the memory addr back to the app.
 * Return: 0 on success or -1 when error.
 * Add by xiangliu 2013.2.4
 * -- Mod 3.6
 * eg.
	int ret = 0;
	OSEVENT event_in, event_out;
	
	 //initialize memory
        memset(&event_out, 0, sizeof(OSEVENT));
        memset(&event_in, 0, sizeof(OSEVENT));
        memcpy(&event_in.pt_eve_out.xxdata, "SendInfo:1234567890", 19);

        ret = OS_Event_Run(&event_in, &event_out);
        if(ret < 0) {
            printf("OS_Event_Run: no one response.");
            break;
        }
        printf("Rev msg: %s", event_out.pt_eve_in.e_cash_register.ecr_msg); //get msg
 ***/
int OS_Event_Run(struct seven *sevent_in, struct seven * sevent_out);


/***
 * Write msg ACCEPT to linuxpos when get signal OSEVENTTYPE_CIR + OSEVENTID_CIRCLE
 * @sevent: struct seven that will send back to client app.
 * Return: 0 on success or -1 when error. 
 * Add by xiangliu 2013.2.4
 * eg.
	case OSEVENTID_CIRCLE: //get signal
    {        
        // handle your process here...
        OSEVENT event;
        
        this->show();

	 // get data here
	 printf("Get OSEVENTID_CIRCLE msg: %s",((struct seven *)sysEvent)->pt_eve_in.e_cash_register.ecr_msg);

	 // write data if necessary
        memset(&event, 0, sizeof(OSEVENT));
        memcpy(&event.pt_eve_out.xxdata, "RevdInfo:0123456789", 19);

	// return your data if necessary
        if(OS_Event_Accept(&event) < 0) {
            qDebug("OS_Event_Accept ERROR!");
            break;
        }
	}
	break;
 ***/
int OS_Event_Accept(struct seven *sevent);


/*
 * write status 'OK' to tell OS that you handled the event.
 * OS_Event_OK take the place of treatment_status = OK.
 * Add by xiangliu 2013.5.13
 */
void OS_Event_OK();

#ifdef __cplusplus
}
#endif

#endif //__SAND_MAIN_H

