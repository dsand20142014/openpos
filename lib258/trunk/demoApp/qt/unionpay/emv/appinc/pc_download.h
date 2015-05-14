/*
|   Application System
|-----------------------------------------------------------------
|   SAND China						(Copyright SAND 2001)      
|------------------------------------------------------------------
|   INTRODUCTION          
|   ============    
|                                                                
|   Project :        EMV for China union Pay 
|                                                                
|   File name :     pc_download.h                                            
|                   
|------------------------------------------------------------------
|   Creation date :	2005-09-01		Fangbo
|   Last modified  :	2005-09-23		Fangbo
|------------------------------------------------------------------
*/

#define TAGMAXLEN			64	
#define TAGMAXNUM			64
#define MASTERKEYMAXNUM	3
#define TOPC_COM1 			1
#define TOPC_COM2 			2
#define COMM_SPEED 			9600  //115200 or 9600
#define COMMBUFMAXLEN 		1024
#define POSINIT				0x01
#define POSCLNTRANS			0x02
#define POSUPDATAPARAM	0x03

typedef struct
{
	UCHAR ucLen;
	UCHAR ucVal[3];
	
}TAGDEFINE;

typedef struct
{
	UCHAR 	ucTag[2];
	USHORT  usTagLen;
	UCHAR 	ucTagData[TAGMAXLEN];
	UCHAR 	ucExistFlag;
	
}RECVTAG;

unsigned short usTagNum;
RECVTAG TagData[TAGMAXNUM];	

typedef struct
{
	unsigned char ucCommand;
	unsigned short ucRecvLen;
	unsigned char ucRecvData[COMMBUFMAXLEN];
	
}PCDOWNLOAD_RECVDATA;

PCDOWNLOAD_RECVDATA    PCDownload_RecvData;


unsigned char PCDown_UpdataParam(void);
unsigned char PCDown_UnPackProcess(void);
unsigned char PCDown_POSInit(void);
unsigned char PCDown_POSClnTrans(void);
unsigned char PCDown_POSUpdataParam(void);
unsigned char PCDown_CommInit(unsigned char ucFlag);
unsigned char PCDown_SelectCOM(void);
unsigned char PCDown_rxcar(unsigned short usOutTime );
unsigned char PCDown_txcar(unsigned char ucSendChar);
unsigned char PCDown_RecvFromPC(unsigned char WaitTime);
unsigned char PCDown_ResponeToPC(unsigned char *aucInBuf,unsigned short uiLen);
unsigned char   PCDown_DispSucess(void);
unsigned char PCDown_GetTag(unsigned char * pucTag ,unsigned char * pucOutBuf ,unsigned short * pusOutLen);
unsigned char PCDown_SwitchTag(unsigned char * pucTag , unsigned short usTagLen);
unsigned char PCDown_SaveTagVal(void);
void PCDown_UpdataMasterKey(void);
unsigned char  PCDown_SaveMasterKey(unsigned char ucIndex , unsigned char * pucInBuf , unsigned short usInLen);

