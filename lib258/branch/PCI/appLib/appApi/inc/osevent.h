/*
	Operating System
--------------------------------------------------------------------------
	FILE  osevent.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-03		Xiaoxi Jiang
    Last modified :	2002-10-03		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __OSEVENT_H
#define __OSEVENT_H

#include <oslsr.h>

#ifdef SLAVE_TERMINAL
#define LGPUPITRE 145
#else
#define LGPUPITRE 99
#endif

#define MAXAPPSHAREBUF 512 //512 20130523 for app

/* --- GLOBALES STRUCTURES TYPES --- */

struct new_date_time
{
    uchar old_date_time[5];
    uchar new_date_time[5];
};

struct samount {                        /* amount type : */
        uchar   sign;                   /* sign '+' or '-' */
        uchar   rcv_amount[9];          /* amount on 8 digits ended with 0 */
};

struct seve_mag_card {                  /* structure for magnetic card event : */
         struct samount amount;         /* amount type */
         uchar  mag_card[187];          /* magnetic card data structure */
         uchar  further_acq;            /* another amount acquisition (1) */
         uchar  message_len;            /* cash register message length */
         uchar  rcv_message[LGPUPITRE]; /* cash register message */
         uchar  flag_currency;          /* additionnal info for currency (flag:0xFF=available) */
         uchar  version;                /* number of version of the structure */
         uchar  *pt_rcv_message;        /* pointer on cash register reception message */
         CURRENCY currency;             /* amount currency info */
         uchar  authorisation_request;  /* the application must do an authorisation request */
};

struct seve_icc {                       /* structure for Smart card event : */
         struct samount amount;         /* amount type */
         uchar  icc_ATR[35];            /* Answer To Reset from the ICC */
         uchar  rfu[187-35];
         uchar  further_acq;            /* another amount acquisition (1) */
         uchar  message_len;            /* cash register message length */
         uchar  rcv_message[LGPUPITRE]; /* cash register message */
         uchar  flag_currency;          /* additionnal info for currency (flag:0xFF=available) */
         uchar  version;                /* number of version of the structure */
         uchar  *pt_rcv_message;        /* pointer on cash register reception message */
         CURRENCY currency;             /* amount currency info */
         uchar  authorisation_request;  /* the application must do an authorisation request */
};

struct seve_icc_emv2 {                  /* structure for EMV level2 smart card event : */
         struct samount amount;         /* amount type */
         uchar  icc_ATR[35];            /* Answer To Reset from the ICC */
         uchar  *pt_emv2_info;           /* Pointer on the smart card data transmitted by the EMV level 2 application selection module */
         uchar  rfu[187-35-sizeof(uchar*)];
         uchar  further_acq;            /* another amount acquisition (1) */
         uchar  message_len;            /* cash register message length */
         uchar  rcv_message[LGPUPITRE]; /* cash register message */
         uchar  flag_currency;          /* additionnal info for currency (flag:0xFF=available) */
         uchar  version;                /* number of version of the structure */
         uchar  *pt_rcv_message;        /* pointer on cash register reception message */
         CURRENCY currency;             /* amount currency info */
         uchar  authorisation_request;  /* the application must do an authorisation request */
};

struct seve_man {                       /* structure for Manual card acquisition event : */
         struct samount amount;         /* amount type */
         uchar  mag_card[187];          /* ISO 2 track reconstituted */
         uchar  further_acq;            /* another amount acquisition (1) */
         uchar  message_len;            /* cash register message length */
         uchar  rcv_message[LGPUPITRE]; /* cash register message */
         uchar  flag_currency;          /* additionnal info for currency (flag:0xFF=available) */
         uchar  version;                /* number of version of the structure */
         uchar  *pt_rcv_message;        /* pointer on cash register reception message */
         CURRENCY currency;             /* amount currency info */
         uchar  authorisation_request;  /* the application must do an authorisation request */
};

struct sdate    {                       /* date type */
         uchar date[10];                /* DDMMYYHHMM (DayMonthYearHourMinute) */
};

struct seve_cash_register{              /* cash register type */
         uchar ecr_msg[MAXAPPSHAREBUF];             /* received buffer message */
};

struct seve_portable{                   /* structure for PORTABLE terminal */
         uchar ter_nr;                  /* terminal number */
         uchar app_nr;                  /* Application number : '0'= BANCAIRE */
         uchar infos_len;               /* Received message length */
         uchar infos[256];              /* Received message */
};

struct seve_ct
{                                       /* struct eve message CT2000 */
    uchar lg;                           /* longueur du message re噓 */
    uchar msg[128];                     /* buffer message recu */
};

struct stransac_info{
         struct samount amount;         /* amount type */
         uchar  payment_type;           /* type of payement */
         uchar  card_number[20];        /* iso2 track terminated with \0 OR (if application_status==EMV2_RESELECT) AID to be removed for the EMV2 selection */
         uchar  expiry_date[5];         /* validity date terminated with \0*/
         uchar  service_code[4];        /* service code terminated with \0*/
         uchar  k;                      /* K value */
         uchar  autor_nr[7];            /* autorisation number and \0*/
         uchar  certif[9];              /* certificat number terminated with \0*/
};

struct stransac_info_2{
         struct samount amount;         /* amount type */
         uchar  payment_type;           /* type of payement */
         uchar  card_number[20];        /* iso2 track terminated with \0*/
         uchar  expiry_date[5];         /* validity date terminated with \0*/
         uchar  service_code[4];        /* service code terminated with \0*/
         uchar  k;                      /* K value */
         uchar  autor_nr[7];            /* autorisation number and \0*/
         uchar  certif[9];              /* certificat number terminated with \0*/
         uchar  RFU[10];                /* additionnal buffer for mpev5 */
         uchar  flag;                   /* flag : 0xFE if ADDITIONNAL INFO ARE PRESENT */
         uchar  stat;                   /* champ STAT ( cf message caisse ) */
         uchar  mode;                   /* champ MODE ( cf message caisse ) */
         uchar  PRIV[10];               /* champ PRIV ( cf message caisse ) */
};

#define P_DEBIT_CARD    '0'             /* debit card */
#define P_CREDIT_CARD   '1'             /* credit card */
#define P_CHEQUE        '2'             /* cheque */

/* Warning never put this declaration in the union structure, Turbo C
   does not understand it ! */
struct smag_card { char mag_card[60];} ;

struct seve_cheque {                    /* structure for Cheque reader event : */
         struct samount amount;         /* amount type */
         uchar  cheque[187];            /* I2C driver message from the cheque reader */
         uchar  further_acq;            /* another amount acquisition (1) */
         uchar  message_len;            /* cash register message length */
         uchar  rcv_message[LGPUPITRE]; /* cash register message */
         uchar  flag_currency;          /* additionnal info for currency (flag:0xFF=available) */
         uchar  version;                /* number of version of the structure */
         uchar  *pt_rcv_message;        /* pointer on cash register reception message */
         CURRENCY currency;             /* amount currency info */
         uchar  authorisation_request;  /* the application must do an authorisation request */
};

struct seve_lsr {                       /* structure for LSR specific events : */
         uchar  lsr_event_nr;           /* specific lsr event number */
         uchar  lsr_event_id;           /* complementary lsr event id */
         uchar  xxdata[MAXAPPSHAREBUF];            /* transmitted data */
};
union seve_in {                         /* data structure received by the application from the MONOS on event activation */
                struct sdate                e_awake;
                struct sdate                e_moddate;
                struct new_date_time        e_new_date_time;
                struct smag_card            e_merchant_nr;
                struct seve_mag_card        e_mag_card;
                struct seve_icc             e_icc;
                struct seve_icc_emv2        e_icc_emv2;
                struct seve_mag_card        e_customer;
                struct seve_man             e_man;
                struct seve_cash_register   e_cash_register;
                struct seve_portable        e_infrared;
                struct seve_cheque          e_cheque;
                struct seve_lsr             e_lsr;
                struct seve_ct              e_ct;
#ifdef SLAVE_TERMINAL
                CURRENCY                    e_tot_currency;
#endif
};

struct seve_out {                       /* returning data structure from the application to the MONOS */
        uchar   treatment_status;       /* TREATED or NOT_TREATED */
        uchar   download;               /* downloading request (1) */
        uchar   application_status;     /* execution report */
        uchar   xxdata[MAXAPPSHAREBUF]; /* returned data */
};
#define TREATED             0           /* application has treated the event */
#define NOT_TREATED         1           /* application has not treated the event */
#define END_APPLI           255         /* there is no more application */
#define DOWNLOAD            1           /* application request for a downloading */
#define NO_DOWNLOAD         0           /* application doesn't request for a downloading */

enum    sevent_type {                   /* constantes defining the event types */
        EVECIR,
        EVECOM,
        EVESEL,
        EVEGRP
};

enum    sevent_nr {                     /* constantes defining the event numbers */
#if 0	
        EVE_POWER_ON,
        EVE_AWAKE,
        EVE_MODDATE,
        EVE_CLRTOTALS,
        EVE_MERCHANT_NR,
        EVE_MASTER_APPLI,
        EVE_MAG_CARD,
        EVE_ICC,
        EVE_CUSTOMER,
        EVE_RECORD,
        EVE_MAN,
        EVE_TOTALS,
        EVE_CASH_REGISTER,
        EVE_PORTABLE_TERMINAL,
#ifdef SLAVE_TERMINAL
        EVE_MES_CT,
#endif
        EVE_CHEQUE,
#ifdef SLAVE_TERMINAL
        EVE_MSG_EXT,
#endif
        EVE_INCOMING_CALL,
        EVE_ASK_MAG_CARD,
        EVE_ASK_ICC,
        EVE_ASK_CHEQUE,
        EVE_LSR,
        EVE_FAST_POWER_ON,
        EVE_NEW_DATE_TIME,
        EVE_ASK_MAN,
        EVE_PSS_EXCHANGE,
#ifdef SLAVE_TERMINAL
        EVE_TOT_CURRENCY,
        EVE_MASTER2,
#else
        EVE_PSS_RECEPT,
#endif
        EVE_TREAT_PCSC,
        EVE_DISCARD_PCSC,
        EVE_CHECK_CARD,
        EVE_ASK_EMV_PARAMETER,
        EVE_NEW_EMV_PARAMETER,
        EVE_TRANSACTION,
        EVE_ICC_EMV2,
        EVE_NEW_MESSAGES,
        EVE_GSM_INCOMING_CALL,
        EVE_GSM_INCOMING_SMS,
		EVE_EMV2_RESELECT,
		EVE_RESELECT_LOGON
#else
	OSEVENTID_SHOW,	   // 0xff
	OSEVENTID_MAGCARD, //磁条卡
    OSEVENTID_ICC,     //IC卡
    OSEVENTID_MFCCRD,  //非接卡
    OSEVENTID_CIRCLE,  //应用跨接
    OSEVENTID_AWAKE,   //定时器
    OSEVENTID_REF01,   //以下预留
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
#endif   
};

struct  seven   {                       /* event structure : */
        enum sevent_type event_type;    /* event type eve CIRCULARE, COMMON, SELECTIVE or GROUP */
        enum sevent_nr   event_nr;      /* event number */
        union seve_in    pt_eve_in;    /* data sent to the application */
        struct seve_out  pt_eve_out;   /* data returned from the application */
};

typedef enum sevent_nr OSEVENTID;
typedef struct seven  OSEVENT;

struct  setat {
        uchar    app_nr;
        uchar    zprog;
        uchar    zdata;
        uchar    pagepro;
};

struct  ctl_portable {
        struct setat etatapp[7];
        uchar        app_val;
        uchar        date_a_decharger[7];
        long         consom;
        ushort       crc;
};

/* --- CONSTANTES DECLARATIONS --- */

/* code page header addresses */
#define IN_APP      0x8000                 /* entry point of the application */
#define CRCAPP      0x8003                 /* CRC address */
#define APP_END     0x8005                 /* end application address */
#define CRC_START   0x8005                 /* starting address for CRC compute */
#define APPNAME     0x8007                 /* application name */
#define APPNR       0x800C                 /* application number */
#define VERSION     0x800D                 /* application version */
#define MENU_APP    0x800E                 /* starting address for application menu */

/*  states for asynchronous smart card */
/*  ---------------------------------- */
#define ASY_OK          0x30      /* treatment ok */
#define ICC_REMOVED     0x31      /* smart card removed during the treatment */
#define VCC_ERROR       0x32      /* Vcc default */
#define VPP_ERROR       0x33      /* Vpp default */
#define ICC_DIALOG_ERR  0x34      /* error of dialog with the smart card */
#define ICC_ASY         0x35      /* asynchronous smart card detected */
#define ICC_ERR         0x36      /* driver unable to read the asynchronous smart card */
#define ICC_SYN         0x37      /* synchronous smart card detected */
#define ICC_MUTE        0x38      /* smart card dumb */
#define ICC_RUNNING     0xFF      /* treatment running */

#define SAME_ICC        0x35       /* identical smart card after another switching on */
#define ICC_EXCHANGED   0x36       /* smart card exchanged */

/*  synchronous smart card */
/*  ---------------------- */
#define SYN_GPM416      0x00      /* smart card type GPM416 */
#define SYN_TS1200      0x01      /* smart card type TS1200 */

/*  states for synchronous smart card */
/*  --------------------------------- */
#define SYN_OK          0x30      /* execution OK */
#define SYN_WRITE_ERR   0x35      /* error writing */
#define SYN_CMP_ERR     0x35      /* comparison not OK */
#define ICC_REMOVED     0x31      /* smart card removed during treatment */

#endif

