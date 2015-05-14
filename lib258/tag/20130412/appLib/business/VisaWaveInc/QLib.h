#ifndef			_QLIB_H
#define			_QLIB_H

typedef struct
{
	UCHAR			aucTerminalID[QTERMINALIDLEN];
	UCHAR			aucMerchantID[QMERCHANTIDLEN];
	UCHAR			aucMerchantName[QMERCHANTNAMELEN];
	QTRANSTYPE		enTransType;
	UCHAR			aucTransDate[QTRANSDATELEN]; //YYYYMMDD
	UCHAR			aucTransTime[QTRANSTIMELEN]; //hhmmss
	UINT			uiRandNum;
	UINT			uiAmount;
	UINT			uiOtherAmount;
	UINT			uiLogAmount;
	UCHAR			ucEntryMode;
	BOOL			bForceOnline;
	UINT			uiTraceNumber;
}QTRANSREQINFO,*PQTRANSREQINFO;



//UCHAR	QINTIAPP_GPO(void);

#endif

