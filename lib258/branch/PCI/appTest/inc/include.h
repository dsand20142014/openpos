#ifndef		_INCLUDE_H
#define		_INCLUDE_H

#include	<oslib.h>

#define SUCCESS								0x00
#define ERR_DRIVER							0x04
#define ERR_CHECKSECRET							0x43
#define ERR_UNKNOWNTRANTYPE			       0x07
#define ERR_HOSTCODE						0x0A

#ifndef		NULL
#define		NULL		0
#endif



#define PARAM_PASSWORDLEN				      6
#define PARAM_TERMINALIDLEN			      8
#define PARAM_MERCHANTIDLEN			      15
#define PARAM_MERCHANTNAMELEN                   40


#define ISO8583_MSGIDLEN			2
#define ISO8583_BITMAPLEN			16
#define ISO8583_MAXBITNUM			128
#define ISO8583_MAXBITBUFLEN		1024	
#define ISO8583_MAXCOMMBUFLEN	1024   
#define ISO8583_BITNOTEXISTFLAG 	0x00
#define ISO8583_BITEXISTFLAG		0xff
#define ISO8583_BIT48NUM			8
#define ISO8583_ICTRANSINFOLEN  	0xff

#define ATTR_N					0x01
#define ATTR_AN					0x02
#define ATTR_NS					0x03
#define ATTR_XN					0x04
#define ATTR_ANS				0x05
#define ATTR_LLVARN			0x06
#define ATTR_LLVARNS		0x07
#define ATTR_LLVARAN		0x08
#define ATTR_LLVARANS		0x09
#define ATTR_LLLVARANS	0x0A
#define ATTR_LLVARZ			0x0B
#define ATTR_LLLVARZ			0x0C
#define ATTR_BIN					0x0D
#define ATTR_LLLVARN		0x0E
#define ATTR_LLVARANZS	0x10
#define ATTR_LLLVARANZS 0x11

#define MAXLINECHARNUM			16


#define MSG_MAXERRMSG			100                                     
#define MSG_MAXNB				200                                     
#define MSG_MAXHOSTMSG		24
#define MSG_MAXCHAR				33                                      
#define MSG_MAXTYPE				2  
#define ERR_APP_TIMEOUT					0xFF



#define PRINT_MAXCHAR			50

#define SUCCESS_TRACKDATA					0x00

#define ERR_END								0x01
#define ERR_CANCEL							0x02

#define ERR_ISO83										0x80
#define ERR_ISO8583_INVALIDVALUE			0x81
#define ERR_ISO8583_INVALIDBIT				0x82
#define ERR_ISO8583_INVALIDPACKLEN		0x83
#define ERR_ISO8583_OVERBITBUFLIMIT		0x84
#define ERR_ISO8583_BITNOTEXIST				0x85
#define ERR_ISO8583_INVALIDPARAM		0x86
#define ERR_ISO8583_NODATAPACK			0x87
#define ERR_ISO8583_OVERBUFLIMIT			0x88
#define ERR_ISO8583_INVALIDBITATTR		0x89
#define ERR_ISO8583_UNPACKDATA			0x8A
#define ERR_ISO8583_SETBITLEN					0x8B
#define ERR_ISO8583_COMPARE					0x8C
#define ERR_ISO8583_MACERROR				0x8D
#define ERR_ISO8583_INVALIDLEN				0x8E


#define TRANS_PURCHASE						0x02


#define ERR_COMMS_PROTOCOL				0xD2
#define ERR_COMMS_LRC								0xD5
#define ERR_COMMS_RECVTIMEOUT				0xD4
#define ERR_COMMS_RECVCHAR				0xD1
#define ERR_COMMS_ERRTPDU						0xD6
#define ERR_WRITEFILE					        0xAA

#define ERR_TRANSFILEFULL					0x0E

#define CFG_MAXDISPCHAR					16
#define CFG_MAXAMOUNTLEN				10



#define KEYARRAY_GOLDENCARDSH		0x01

#define KEYINDEX_GOLDENCARDSH_PINKEY			0x07
#define KEYINDEX_GOLDENCARDSH_MACKEY		0x09
#define KEYINDEX_PURSECARD_MASTERKEY			0x00

#define TRANS_LOGONON						0xF0

#define ERR_MAG_TRACKDATA					0xC0
#define ERR_SANDMAGCARD                      		0xD8

#define ERR_CASH_NOTLOGON				0xE4


#define TRANS_DATELEN					4
#define TRANS_TIMELEN						3
#define TRANS_ACCLEN						20


#define CHAR_STX				0x02
#define CHAR_ETX				0x03


#define TRANS_ISO2MINLEN				24//12
#define TRANS_ISO2LEN						40//37
#define TRANS_ISO3LEN						107//104
#define TRANS_PINDATALEN				8

#define TRANS_SAVEMAXNB  200

#define TRANS_AUTHCODELEN		6
#define TRANS_AUTHCODEMIN		2
#define TRANS_BANKIDLEN				6
#define TRANS_REFNUMLEN				12

#define min(a, b)           (((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))


typedef struct
{
	unsigned char	 ucISO2Status;
	unsigned short uiISO2Len;
	unsigned char	aucISO2[TRANS_ISO2LEN];
	unsigned char	ucISO3Status;
	unsigned short uiISO3Len;
	unsigned char	aucISO3[TRANS_ISO3LEN];
	unsigned char	ucInputPINLen;
	unsigned char	aucPINData[TRANS_PINDATALEN];
}EXTRATRANS;


typedef struct
{
	unsigned char	ucExistFlag;
	unsigned short uiOffset;
	unsigned short uiLen;
}ISO8583Bit;

typedef struct
{
	ISO8583Bit Bit[ISO8583_MAXBITNUM];
	unsigned short uiBitBufLen;
	unsigned char	aucMsgID[ISO8583_MSGIDLEN];
	unsigned char	aucBitBuf[ISO8583_MAXBITBUFLEN];
	unsigned short uiCommBufLen;
	unsigned char	aucCommBuf[ISO8583_MAXCOMMBUFLEN];
}ISO8583;

typedef struct
{
	unsigned char	ucType;
	unsigned char	ucErrCode;
	unsigned char	aucErrMsg[MSG_MAXCHAR];
}SINGLEERRMSG;

typedef struct
{
	unsigned char	ucType;
	unsigned char	ucErrCode;
	unsigned char	aucErrMsg[MSG_MAXCHAR];
}HOSTEERRMSG;

typedef struct
{
	 unsigned char	aucMSGType;
	 unsigned char	aucMSGTab[MSG_MAXCHAR];
}MSG;

typedef struct
{
	unsigned char	ucAttr;
	unsigned short uiMaxLen;
}ISO8583BitAttr;


typedef struct
{
	ISO8583Bit		Bit[ISO8583_MAXBITNUM];
	unsigned short	uiBitBufLen;
	unsigned char		aucMsgID[ISO8583_MSGIDLEN];
	unsigned char		aucBitBuf[ISO8583_MAXBITBUFLEN];
}BACKUPISO8583;

typedef struct
{
	BACKUPISO8583	SendISO8583Data;
}TRANS_8583_DATA;


typedef struct
{
	unsigned char	ucTransType;
	unsigned long ulTraceNumber;
	unsigned char	aucISO2[20];
	unsigned char	ucISO2Len;
	unsigned char	ucISO3Len;
	unsigned char ucDispSourceAcc[TRANS_ACCLEN*2];
	unsigned char	ucDispSourceAccLen;
	unsigned char	aucISO3[53];
	unsigned char  aucCardPan[6];
	unsigned char ucSourceAccType;
	unsigned char	ucExpiredDate[TRANS_DATELEN];
	unsigned long ulAmount;
	unsigned char	aucDate[TRANS_DATELEN];
	unsigned char	aucTime[TRANS_TIMELEN];
	unsigned char	aucSourceAcc[TRANS_ACCLEN];
	unsigned char	ucSourceAccLen;
	unsigned char		aucAuthCode[TRANS_AUTHCODELEN+1];
	unsigned char		aucRefNum[TRANS_REFNUMLEN];
	unsigned char   	aucBankID[TRANS_BANKIDLEN*2];
}NORMALTRANS;	

typedef struct
{
	 unsigned short	uiTransIndex;
	 unsigned char	aucTerminalID[PARAM_TERMINALIDLEN+4];
	unsigned char	aucMerchantID[PARAM_MERCHANTIDLEN];
	unsigned char ucErrorExtCode;
}RUNDATA;


typedef struct
{
	unsigned char	  aucTerminalID[PARAM_TERMINALIDLEN];
	unsigned char	  aucMerchantID[PARAM_MERCHANTIDLEN]; 
	unsigned char	  aucMerchantName[PARAM_MERCHANTNAMELEN];
	unsigned char   font;
	unsigned char   printtimes;	
}CONSTANTPARAM;

typedef struct
{
	 unsigned long ulBatchNumber;
	 unsigned long ulTraceNumber;
	 unsigned char ucCashierLogonFlag;
}CHANGEPARAM;


typedef struct
{
	unsigned short	auiTransIndex[TRANS_SAVEMAXNB];
}TRANSINFO;

typedef struct
{
	CONSTANTPARAM	ConstantParamData;
	CHANGEPARAM	ChangeParamData;
	TRANS_8583_DATA Trans_8583Data;
	TRANSINFO		TransInfoData;
}DATASAVEPAGE0;

typedef struct
{
	NORMALTRANS	SaveTrans;
}DATASAVEPAGE1;


void ISO8583_Clear(void);
unsigned char ISO8583_SetMsgID(unsigned short iMsgID);
unsigned char ISO8583_SetBitValue(short iBitNo,unsigned char *pucInData,unsigned short uiInLen);
unsigned char ISO8583_GetBitValue(short iBitNo, unsigned char *pucOutData, short *puiOutDataLen, short iOutBufLen);
unsigned char ISO8583_PackData(unsigned char *pucOutData, unsigned short *puiOutDataLen, short iOutLen);
unsigned char ISO8583_CheckBit(short iBitNo);
unsigned char ISO8583_UnpackData(unsigned char *pucInData, short iInDataLen);
void ISO8583_ClearBit(void);
void ISO8583_ClearBitBuf(void);
unsigned char ISO8583_CheckResponseValid(void);
unsigned char ISO8583_CompareSentBit(short iBitNo);
unsigned char ISO8583_GetMsgID(unsigned short *piMsgID);
void ISO8583_SaveISO8583Data(unsigned char *pucSrc,unsigned char *pucDest);
unsigned char ISO8583_SetBitHexValue(short iBitNo,
				unsigned char *pucInData,unsigned short uiInLen);




void UTIL_IncreaseTraceNumber(void);
unsigned char UTIL_Single_StoreNewKey(void);
unsigned char UTIL_CalcGoldenCardSHMAC_Single(
				unsigned char *pucInData,
				unsigned short uiInLen,
				unsigned char *pucOutMAC);
void UTIL_ClearGlobalData(void);
unsigned char UTIL_Input(unsigned char ucLine,unsigned char ucClrFlag,
				unsigned char ucMin, unsigned char ucMax,
				unsigned char ucType,
				unsigned char *pucBuf,
				unsigned char *pucMask);
unsigned char UTIL_GetKey(unsigned char wait_sec);
unsigned char UTIL_InputTransAmount(void);
unsigned char UTIL_InputAmount(unsigned char ucLine,unsigned long *pulAmount,
				unsigned long ulMinAmount, unsigned long ulMaxAmount);
void UTIL_Form_Montant(unsigned char *Mnt_Fmt,unsigned long Montant,unsigned char Pos_Virgule);
unsigned char UTIL_InputEncryptPIN(void);
unsigned char UTIL_format_time_bcd_str( unsigned char *Ptd, unsigned char *Pts );
unsigned char UTIL_format_date_bcd_str( unsigned char *Ptd, unsigned char *Pts );
void UTIL_Beep(void);
void gz_xor(unsigned char *a, unsigned char *b, char lg);


unsigned char PINPAD_42_LoadSingleKeyUseSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucDecryptKeyIndex,
					unsigned char ucDestKeyIndex,
					unsigned char *pucInData);
unsigned char PINPAD_47_Encrypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData);
unsigned char PINPAD_47_Crypt8ByteSingleKey(
					unsigned char ucKeyArray,
					unsigned char ucKeyIndex,
					unsigned char *pucInData,
					unsigned char *pucOutData);
unsigned char SendReceivePinpad(void);


unsigned char COMMS_GenSendReceive(void);
unsigned char COMMS_RS232_SendReceive(unsigned char *pucInData,unsigned short uiInLen,
					unsigned char *pucOutData,unsigned short *puiOutLen);
unsigned char COMMS_ReceiveByte(unsigned char *pucByte,unsigned short *puiTimeout);

void OnEve_power_on(void);

unsigned char CFG_CheckConstantParamData(void);
unsigned char CFG_CheckChangeParamData(void);
unsigned char CFG_ConstantParamBaseData(void);
unsigned char CFG_CheckSaveTrans(void);
unsigned char CFG_CheckTransInfoData(void);

unsigned char Mag_Menu(unsigned char ucKey);
unsigned char MAG_GetTrackInfo(unsigned char *pucTrack);
unsigned char  MAG_DispCardNo(void);

unsigned char Trans_Process(void);
unsigned char TRANS_Online(void);


unsigned char SAV_TransSave(void);
unsigned char SAV_CheckTransMaxNb(void);

unsigned char XDATA_Write_SaveTrans_File(unsigned short uiIndex);
unsigned char XDATA_Read_Vaild_File(unsigned char ucFileIndex);
unsigned char XDATA_Write_Vaild_File(unsigned char ucFileIndex);
unsigned char XDATA_Read_SaveTrans_File(unsigned short uiIndex);

unsigned char PRINT_Ticket(unsigned char ucFlag);
unsigned char PRINT_Deal(void);

unsigned char MSG_DisplayErrMsg(unsigned char ucErrorCode);
unsigned char MSG_WaitKey(unsigned short uiTimeout);



#endif
