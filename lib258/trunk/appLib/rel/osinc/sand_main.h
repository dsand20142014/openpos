#ifndef __SAND_MAIN_H
#define __SAND_MAIN_H 

#ifdef __cplusplus
extern "C" {
#endif
#include <osevent.h>

#define APP_MSG_TYPE        100
#define APP_MSG_LEN         40
#define APP_SHM_SIZE        8096
#define STATUS_ACCEPT		5 //xl add 03.2.4 
#define OSEVENTID_CIRCLE 	0xf0 //add by xl for circle event


typedef enum
{
    OSEVENTTYPE_CIR,
    OSEVENTTYPE_COM,
    OSEVENTTYPE_SEL,
    OSEVENTTYPE_GRP
}OSEVENTTYPE;

typedef enum
{
    OSEVENTID_POWERON,
    OSEVENTID_AWAKE,
    OSEVENTID_MODDATE,
    OSEVENTID_CLRTOTALS,
    OSEVENTID_MERCHANTNR,
    OSEVENTID_MASTERAPPLI,
    OSEVENTID_MAGCARD,
    OSEVENTID_ICC,
    OSEVENTID_CUSTOMER,
    OSEVENTID_RECORD,
    OSEVENTID_MAN,
    OSEVENTID_TOTALS,
    OSEVENTID_CASHREGISTER,
    OSEVENTID_REF01,
    OSEVENTID_REF02,
    OSEVENTID_REF03,
    OSEVENTID_REF04,
    OSEVENTID_REF05,
    OSEVENTID_REF06,
    OSEVENTID_REF07,		//20
    OSEVENTID_REF08,
    OSEVENTID_REF09,
    OSEVENTID_REF10,
    OSEVENTID_REF11,
    OSEVENTID_REF12,
    OSEVENTID_REF13,
    OSEVENTID_REF14,
    OSEVENTID_REF15,
    OSEVENTID_REF16,
    OSEVENTID_REF17,		//30
    OSEVENTID_REF18,
    OSEVENTID_REF19,
    OSEVENTID_REF20,
    OSEVENTID_REF21,
    OSEVENTID_REF22,
    OSEVENTID_REF23,
    OSEVENTID_LOGON,		
    OSEVENTID_MFCCRD,		//38
    OSEVENTID_PAMLOAD		//39
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
    OSEVENTTYPE     Type;
    OSEVENTID       Index;
    OSEVENTIN       *pIn;
    OSEVENTOUT      *pOut;
}OSEVENT_B;

typedef struct {
	int  m_type;
	char m_data[APP_MSG_LEN];
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
		
		//initialize memory
        memset(((struct seven *)sysEvent)->pt_eve_out->xxdata, 0, 256);
        memcpy(((struct seven *)sysEvent)->pt_eve_out->xxdata, "SendInfo:1234567890", 19);

        sysEvent = OS_Event_Run(sysEvent);
        if(sysEvent < 0) {
            qDebug("OS_Event_Run ERROR!");
            break;
        }
        qDebug("Rev msg: %s", ((struct seven *)sysEvent)->pt_eve_in->e_cash_register.ecr_msg); //get msg
 ***/
int OS_Event_Run(struct seven *sevent_in, struct seven *sevent_out);

/***
 * Write msg ACCEPT to linuxpos when get signal OSEVENTTYPE_CIR + OSEVENTID_CIRCLE
 * @sevent: struct seven that will send back to client app.
 * Return: 0 on success or -1 when error. 
 * Add by xiangliu 2013.2.4
 * eg.
	case OSEVENTTYPE_CIR + OSEVENTID_CIRCLE: //get signal
    {        
        // handle your process here...
		qDebug("Get signal OSEVENTTYPE_CIR + 0x10"); 
        this->show();

		// write data to memory here
        memset(((struct seven *)sysEvent)->pt_eve_out->xxdata, 0, 256);
        memcpy(((struct seven *)sysEvent)->pt_eve_out->xxdata, "RevdInfo:0123456789", 19);

        if(OS_Event_Accept(sysEvent) < 0) {
            qDebug("OS_Event_Accept ERROR!");
            break;
        }
	}
	break;
 ***/
int OS_Event_Accept(struct seven *sevent);

#ifdef __cplusplus
}
#endif

#endif //__SAND_MAIN_H

