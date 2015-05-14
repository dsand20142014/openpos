#ifndef __EVENT_H
#define __EVENT_H

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
    OSEVENTID_REF07,		//19
    OSEVENTID_REF08,
    OSEVENTID_REF09,
    OSEVENTID_REF10,
    OSEVENTID_REF11,
    OSEVENTID_REF12,
    OSEVENTID_REF13,
    OSEVENTID_REF14,
    OSEVENTID_REF15,
    OSEVENTID_REF16,
    OSEVENTID_REF17,		//29
    OSEVENTID_REF18,
    OSEVENTID_REF19,
    OSEVENTID_REF20,
    OSEVENTID_REF21,
    OSEVENTID_REF22,
    OSEVENTID_REF23,
    OSEVENTID_LOGON,		//36
    OSEVENTID_MFCCRD,		//37
    OSEVENTID_PRGLOAD,	//38
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
    unsigned char   aucData[256];
}OSEVENTMAGCARD;

typedef struct
{
    OSEVENTAMOUNT   Amount;
    unsigned char   aucAtr[35];
}OSEVENTICC;

typedef struct
{
    OSEVENTAMOUNT   Amount;
    unsigned char   aucData[256];
}OSEVENTMAN;

typedef union
{
    OSEVENTMAGCARD  Mag;
    OSEVENTICC      Icc;
    OSEVENTMAN      Man;
	unsigned char   CashRegister[256];
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
}OSEVENT;

#endif
