

typedef struct
{
	UCHAR			aucTerminalID[TERMINALIDLEN];
	UCHAR			aucMerchantID[MERCHANTIDLEN];
	UCHAR			aucMerchantName[MERCHANTNAMELEN];
	TRANSTYPE		enTransType;
	UCHAR			aucTransDate[TRANSDATELEN]; //YYYYMMDD
	UCHAR			aucTransTime[TRANSTIMELEN]; //hhmmss
	UINT			uiRandNum;
	UINT			uiAmount;
	UINT			uiOtherAmount;
	UINT			uiLogAmount;
	UCHAR			ucEntryMode;
	BOOL			bForceOnline;
	UINT			uiTraceNumber;
}TRANSREQINFO,*PTRANSREQINFO;//



UCHAR	INTIAPP_GPO(void);
