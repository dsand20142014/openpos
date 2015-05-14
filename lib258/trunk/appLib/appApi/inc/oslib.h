/*
	Operating System
--------------------------------------------------------------------------
	FILE  oslib.h
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

#ifndef __OSLIB_INC
#define __OSLIB_INC


#include <type_uns.h>           /* unsigned unsigned char, unsigned short and ulong types */
#include <osdriver.h>           /* driver types */
#include <osmodem.h>            /* communications types (modem and rs232) */
//#include <osmenu.h>             /* MENU and HEADER_APP types */
#include <osevent.h>            /* structures for events management */
#include <appdef.h>
#include <oswifi.h>
#include <osdrv.h>
#include <toolslib.h>
#include <py_input.h>

#include "Os__barscanner_9600.h"


/* File API definement */
#ifndef O_READ
#define O_READ              0x01
#endif
#ifndef O_WRITE
#define O_WRITE             0x02
#endif
#ifndef O_CREATE
#define O_CREATE            0x04
#endif
#ifndef O_TRUNC
#define O_TRUNC             0x08
#endif
#ifndef SEEK_SET
#define SEEK_SET	        0x00
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	        0x01
#endif
#ifndef SEEK_END
#define SEEK_END	        0x02
#endif

#define	FILE_INVMODE		0x01
#define	FILE_NOEXIST		0x02
#define	FILE_EXIST          0x03
#define	FILE_FULL			0x04
#define	FILE_NOSPACE		0x05
#define	FILE_MUCHHALDLE		0x06

/*power manage*/
int Os__pm_enable (void);	//add by zy
void Os__pm_enable_vcc(void);
void Os__pm_disable_vcc(void);
void Os__sys_poweroff(void);



/*********** PS4000 2.4G ***************/
int Os__24g_voucher_connect(unsigned char *pout, int  *out_len);
int Os__24g_voucher_disconnect(unsigned char *pout, int  *out_len);
int Os__24g_voucher_init(unsigned char *termID, unsigned char *pout, int  *out_len);
int Os__24g_voucher_read(unsigned char p1, unsigned char p2, unsigned char vid[16],
                         unsigned char *pout, int  *out_len);
int Os__24g_voucher_readover(unsigned char *vid, unsigned char *pout, int  *
out_len);

int Os__24g_mag_connect(unsigned char *pout, int  *out_len);
int Os__24g_mag_init(unsigned char *data, unsigned char *pout, int  *out_len);
int Os__24g_mag_readtrack(unsigned char termid[6], unsigned char track, unsigned char *pout, int  *out_len);
int Os__24g_mag_readphoto_init(unsigned char *pout, int  *out_len);
int Os__24g_mag_readphoto(unsigned char xx, unsigned char yy,  unsigned char data_type,
                          unsigned char *pout, int  *out_len);
int Os__24g_mag_consumenote(unsigned char salerID[15], unsigned char termID[4], 
	unsigned char cardid_len, unsigned char cardid[10], unsigned char amount[6],
	unsigned char date[3], unsigned char time[3], unsigned char *pout, int  *out_len);
	
int Os__24g_mag_disconnect( unsigned char *pout, int  *out_len);                          



int Os__24g_ep_connect(unsigned char *pout, int  *out_len);
int Os__24g_ep_disconnect(unsigned char *pout, int  *out_len);
int Os__24g_ep_trade(unsigned char p1, unsigned char salerID[15], unsigned char termID[4], 
	unsigned char cardid_len, unsigned char cardid[10], unsigned char amount[6],
	unsigned char date[3], unsigned char time[3],
	unsigned char *pout, int  *out_len);




int SEIMMA_DownloadPIC();


/*zmodem download*/
int simple_rz(char argc, char *argv[]);

/* SAND API */
unsigned char APISAND_Check(void);
/* OS Interface */
unsigned char OSMENU_MainForApp(void);
/* General drivers */
void OSDRV_Call(DRIVER *pDrv);
void OSDRV_Abort(unsigned char ucDrvID);
void OSDRV_Abort1(unsigned char ucDrvID);
unsigned char OSDRV_Wait(DRV_OUT *pDrvOut);
unsigned char OSDRV_Ioctl(unsigned char ucDrvID,unsigned char ucType,unsigned char *pucData);
DRV_OUT *OSDRV_CallAsync(DRIVER *pDrv);
/* MMI */
void OSMMI_ClrDisplay(unsigned char ucFont,unsigned char ucRow);
void OSMMI_Beep(void);
DRV_OUT *OSMMI_GetKey(void);
unsigned char OSMMI_XGetKey(void);
DRV_OUT *OSMMI_GetKeys(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
unsigned char *OSMMI_XGetKeys(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
DRV_OUT *OSMMI_GetFixKeys(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
unsigned char *OSMMI_XGetFixKeys(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
DRV_OUT *OSMMI_GetPin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB);
unsigned char *OSMMI_XGetPin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB);
void OSMMI_Crypt(unsigned char *pucCryptKey);
void OSMMI_DisplayASCII(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr);
unsigned char OSMMI_GB2312Display(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr);
unsigned char OSMMI_GraphDisplay(unsigned char ucType,unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr,
                                 unsigned char ucLen);
unsigned char OSMMI_BackLightCtl(unsigned char ucType);
unsigned char OSPS2_ScanPs2Key(unsigned int uiDelayTime);
unsigned char OSMMI_NonStdDisplay(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr);
unsigned char OSMMI_ArabicDisplay(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr);
unsigned char OSMMI_DisplayIcon(unsigned char ucIconType,unsigned char *pucIconData);
unsigned char OSMMI_ClearIcon(unsigned char ucIconType);
unsigned char OSMMI_DisplayNewDate(unsigned char *pucDate);
/* PINPAD */
void OSPAD_ClrDisplay(unsigned char ucFont,unsigned char ucRow);
void OSPAD_Beep(void);
DRV_OUT *OSPAD_GetKey(void);
unsigned char OSPAD_XGetKey(void);
DRV_OUT *OSPAD_GetKeys(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
unsigned char *OSPAD_XGetKeys(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
DRV_OUT *OSPAD_GetFixKeys(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
unsigned char *OSPAD_XGetFixKeys(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol,unsigned char *pucPtr, unsigned char ucCharNB);
DRV_OUT *OSPAD_GetPin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB);
unsigned char *OSPAD_XGetPin(unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr, unsigned char ucCharNB);
void OSPAD_Crypt(unsigned char *pucCryptKey);
void OSPAD_DisplayASCII(unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char *pucPtr);
unsigned char OSPAD_GB2312Display(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr);
unsigned char OSPAD_GraphDisplay(unsigned char ucType,unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr,
                                 unsigned char ucLen);
unsigned char OSPAD_BackLightCtl(unsigned char ucType);
unsigned char OSPAD_ArabicDisplay(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr);
/* Printer */
unsigned char OSPRN_BeginPrint(unsigned char *pucParam);
unsigned char OSPRN_EndPrint(void);
unsigned char OSPRN_CheckPaper(unsigned char ucFlag);
DRV_OUT * OSPRN_Feed(unsigned char ucLineNB);
unsigned char OSPRN_XFeed(unsigned char ucLineNB);
DRVOUT *OSPRN_PrintASCII(unsigned char *ucFont, unsigned char *Text);
unsigned char OSPRN_XPrintASCII(unsigned char ucFont,unsigned char *pucPtr);
int OSPRN_XPrintGB2312(unsigned char ucASCIIFont, unsigned char ucGBFont, unsigned char * Text);
unsigned char OSPRN_XPrintGraph(unsigned char *pucInData,unsigned char ucInLen);
unsigned char OSPRN_XPrintNonStd(unsigned char ucFont,unsigned char *pucPtr);
/* Magnetic cards reader */
DRV_OUT * OSMAG_Read(void);
DRV_OUT * OSMAG_ReadGeneral(unsigned char ucTrack1Method,
                            unsigned char ucTrack2Method,
                            unsigned char ucTrack3Method);
/* Modem and 1st RS232 */
unsigned char OSUART_Init1(COM_PARAM *pComParam);
unsigned char OSMODEM_DialInit(COM_PARAM *pComParam);
unsigned char OSMODEM_Dial(DIAL_PARAM *pDialParam);
unsigned char OSMODEM_BackDial(DIAL_PARAM *pDialParam);
unsigned char OSMODEM_CheckBackDial(unsigned short uiTimeout);
void OSMODEM_ReadDialParam(DIAL_PARAM *pDialParam);
unsigned char OSMODEM_SetParam(unsigned char ucParamNb,unsigned char ucValue);
unsigned char OSMODEM_TxChar(unsigned char ucCh);
unsigned char OSUART_TxChar1(unsigned char ucCh);
unsigned short OSMODEM_RxChar(unsigned short uiTimeout);
unsigned short OSUART_RxChar1(unsigned short uiTimeout);
unsigned char OSMODEM_Hangup(void);
void OSUART_Flush1(unsigned short uiRecvNB);
void OSUART_Close1(void);
/* 2nd RS232 */
unsigned char OSUART_Init2(unsigned short uiParam1,unsigned short uiParam2,unsigned char ucParam3);
void OSUART_TxChar2(unsigned char ucCh);
unsigned short OSUART_RxChar2(unsigned short uiTimeout);
void OSUART_Close2(void);

/*osmodemppp.c*/
unsigned char	OSMODEMPPP_Open(unsigned char* paucUserName,unsigned char *paucPassword);
unsigned char	OSMODEMPPP_Close(void);

/* GSM */
unsigned char OSGSM_GprsDial(void);
unsigned char OSGSM_GprsCheckDial(unsigned short uiTimeout);
unsigned char OSGSM_GprsHangupDial(void);
unsigned char OSGSM_GprsReset(void);
unsigned char OSGSM_GprsSetAPN(unsigned char* pucPtr);
unsigned char OSGSM_GprsGetCCID(unsigned char* pucCCID,unsigned char* pucCCIDLen);
unsigned char OSGSM_GprsSetDialParam(WIRELESS_PARAM wl);

/* Date and Time */
void OSDATE_ReadDate(unsigned char *pucDate);
unsigned char OSDATE_WriteDate(unsigned char *pucDate);
void OSDATE_ReadTime(unsigned char *pucTime);
unsigned char OSDATE_WriteTime(unsigned char *pucTime);
void OSDATE_ReadTimeSec(unsigned char *pucTime);
void OSDATE_ReadDateTime(unsigned char *pucDateTime);
void OSDATE_ReadDateFormat(unsigned char *pucDate);
void OSDATE_ReadTimeFormat(unsigned char *pucTime);
/* Timers */
unsigned char OSTIMER_Start(unsigned int *puiTimeout);
void OSTIMER_Stop(unsigned int *puiTimeout);
unsigned char OSTIMER_Delay(unsigned int uiTimeout);
/* Protected Memory */
unsigned char OSSAVE_Swap(unsigned char ucPageNb);
void OSSAVE_Copy(unsigned char *pucSrc, unsigned char *pucDest, unsigned short uiLen);
void OSSAVE_Set(unsigned char *pucDest, unsigned char ucCh, unsigned short uiLen);
unsigned char OSSAVE_ReadEEPROM(unsigned short uiStartAddr, unsigned char ucLen, unsigned char *pucPtr);
/* TCP Function */
unsigned char TCP_Open(int *piTCPHandle);
unsigned char TCP_Connect(int iTCPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort);
unsigned char TCP_Send(int iTCPHandle,unsigned char *pucInBuf,unsigned short uiInLen);
unsigned char TCP_Recv(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);
unsigned char TCP_Close(int iTCPHandle);


/* UDP Function */
unsigned char UDP_Open(int *piUDPHandle);
unsigned char UDP_Config(int iUDPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort);
unsigned char UDP_Send(int iUDPHandle,unsigned char *pucInBuf,unsigned short uiInLen);
unsigned char UDP_Recv(int iUDPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);
unsigned char UDP_Close(int iUDPHandle);

/*DNS Function*/
unsigned char	DNS_StdQuery(unsigned char* pQName,unsigned int *pauiHostIP,unsigned char* pucHostIPNum);
/*osbarcode.c*/
unsigned char OSBARCODE_EANPrint(unsigned char *pucData);

/* Debug */
void Uart_Printf(char *fmt,...);

/* File Function */
int OSAPP_OpenFile(char *pcFileName,unsigned char ucMode);
int OSAPP_OpenFileEx(char *pcFileName,unsigned char ucMode,unsigned char *pucAttr);
int OSAPP_FileRead(int iHandle,unsigned char *pucOutData,unsigned int uiOutLen);
int OSAPP_FileWrite(int iHandle,unsigned char *pucInData,unsigned int uiInLen);
int OSAPP_FileClose(int iHandle);
int OSAPP_FileSeek(int iHandle,long lOffset,unsigned char ucLocation);
long OSAPP_FileSize(char *pcFileName);
int OSAPP_FileDelete(char *pcFileName);
int OSAPP_FileTrunc(int iHandle,unsigned int uiLen);
unsigned char OSAPP_FileGetLastError(void);
void OSAPP_ChangeName(unsigned char *pucName);

/*usb*/
unsigned char OSUSB_Wait_InsertDisk(unsigned int uiTimeOut);
unsigned char OSUSB_DirOrFile_List(void);
unsigned char OSUSB_OpenFile(unsigned char *pucFileName);
unsigned char OSUSB_CreateDir(unsigned char *pFilename);
unsigned char OSUSB_ReadFile(unsigned int uiOffset,unsigned char *pucData,unsigned int *piLen);
unsigned char OSUSB_WriteFile(unsigned int uiOffset,unsigned char *pucData,unsigned int uiLen);
unsigned char OSUSB_AppendFile(unsigned char *pucData,unsigned int uiLen);
unsigned char OSUSB_FileSize(unsigned char *pFilename,unsigned int *piSize);
unsigned char OSUSB_EraseFile(unsigned char *pFilename);
unsigned char OSUSB_CloseFile(void);
/* oscfg.h */
CONFIG_PARAM OSCFG_ReadParam(void);
unsigned char OSCFG_SetETHConfig(unsigned char *paucAddr,unsigned char *paucMask,unsigned char *paucGatewayAddr);
int OSAPP_FreeSpace(void);

/* INGENICO API */

/* --- Displays and Keyboards --- */
void    Os__clr_display(unsigned char line);
void    Os__clr_display_pp(unsigned char line);
void    Os__beep(void);
void    Os__beep_pp(void);
DRV_OUT * Os__get_key(void);

unsigned char  Os__get_cam_key();

DRV_OUT * Os__get_key_pp(void);
unsigned char   Os__xget_key_pp(void);

unsigned char   Os__xget_key(void);

DRV_OUT * Os__get_amount(unsigned char line, unsigned char col, unsigned char *text);
unsigned char   * Os__xget_amount(unsigned char line, unsigned char col, unsigned char *text);
DRV_OUT * Os__get_varkey(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
DRV_OUT * Os__get_varkey_pp(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
unsigned char   * Os__xget_varkey(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
unsigned char   * Os__xget_varkey_pp(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
unsigned char   * Os__xget_minkey(unsigned char line, unsigned char col, unsigned char *text, unsigned char n,unsigned char m)reentrant;
DRV_OUT * Os__get_fixkey(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
DRV_OUT * Os__get_fixkey_pp(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
unsigned char   * Os__xget_fixkey(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
unsigned char   * Os__xget_fixkey_pp(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
DRV_OUT * Os__get_pin(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
DRV_OUT * Os__get_pin_pp(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
unsigned char   * Os__xget_pin(unsigned char line, unsigned char col, unsigned char *text, unsigned char n)reentrant;
void    Os__crypt(unsigned char *crypting_key);
void    Os__crypt_pp(unsigned char *crypting_key);
void    Os__display(unsigned char line,unsigned char col,unsigned char *text);
void    Os__display_pp(unsigned char line,unsigned char col,unsigned char *text);
unsigned char   * Os__xget_insert(unsigned char line, unsigned char col, unsigned char *text, unsigned char n,unsigned char charac,unsigned char step)reentrant;
unsigned char   * Os__xget_scankey(unsigned char line, unsigned char col, unsigned char *text, unsigned char n,unsigned char mode)reentrant;

/* --- EEPROM --- */
unsigned char   Os__read_eeprom(unsigned short start,unsigned char length,unsigned char *pt)reentrant;

/* --- SMART card ---*/
DRV_OUT * Os__ICC_insert(void);
void    Os__ICC_remove(void);
void    Os__ICC_off(void);
/* For Asynchronous smart card only */
DRV_OUT * Os__ICC_Async_order(unsigned char *ISO7816_order,unsigned short length);
/* For every kind of smart card */
DRV_OUT * Os__ICC_order(DRV_IN *pt_drv_in);

/* --- magnetic card ---*/
int Os_open_mag_fd(); //new add by zhangy
DRV_OUT * Os__mag_read(void);
//int Os__get_mag_status(DRV_OUT *magdata);
int Os__get_mag_status(void);




/* --- PRINTER ---*/
/*
*new add, for print sign bmp file.
*/
int Os__sign_xprint(unsigned char *ucPath);
DRV_OUT * Os__print(unsigned char *buffer);
int Os__linefeed(unsigned int);
int Os__xlinefeed(unsigned int);
unsigned char   Os__xprint(unsigned char *buffer);

/* --- DELAY ---*/
unsigned char   Os__timer_start(unsigned int *);
void    Os__timer_stop(unsigned int *);
unsigned char   Os__xdelay(unsigned int);

/* --- SYSTEM ---*/
unsigned char   Os__saved_swap(unsigned char);
void    Os__saved_copy(unsigned char *,unsigned char *,unsigned short)large;
void    Os__saved_set(unsigned char *,unsigned char,unsigned short)large;
void    Os__code_copy(unsigned char *,unsigned char *,unsigned short)large;
unsigned short  Os__crc16(unsigned char *pt,unsigned short length)large;
void    Os__read_eft_id(unsigned char *pt);
unsigned char   Os__read_paper_type(void);
DRV_OUT * Os__get_appli_id(void);

/* --- GENERAL DRIVER ---*/
void    Os__call_drv(DRIVER *);
void    Os__abort_drv(unsigned char);
unsigned char   Os__wait_drv(DRV_OUT *);

/* --- MODEM ---*/
unsigned char   Os__init_com(COM_PARAM *pt);
void    Os__hang_up(void);
unsigned char   Os__txcar(unsigned char car);
void    Os__com_flush1(unsigned short recmax);
void    Os__com_flush2(void);
unsigned char   Os__dial(DIAL_PARAM *pt);
unsigned char   Os__x28(X28_PARAM *pt);
unsigned short  Os__rxcar(unsigned short timeout);
void    Os__read_dial_param(DIAL_PARAM *pt);
unsigned char   Os__V23_reverse_carrier(void);

/* --- COM 3 --- */
unsigned char   Os__init_com3(unsigned short r_6_7 , unsigned short r_4_5 , unsigned char r_3);
unsigned short  Os__rxcar3(unsigned short to_attente);
void    Os__txcar3(unsigned char car);
void    Os__end_com3(void);

/* --- "FRAME PROTOCOL" (FRENCH CASH REGISTER PROTOCOL) --- */
unsigned char   Os__frame_init(COM_PARAM *pt);
void    Os__frame_conf_abort(unsigned char);
DRV_OUT * Os__frame_send(unsigned char *message);
DRV_OUT * Os__frame_rcv(unsigned char *buffer);

/* --- I2C communication (for cheque reader) --- */
unsigned char   Os__cheque_init(COM_PARAM *pt);
DRV_OUT * Os__cheque_rcv(unsigned char *buffer);
DRV_OUT * Os__cheque_send(unsigned char *buffer);

/* --- Os__read_eft_id possible modes --------*/
#define EFT_M1              0x01        /* M1 */
#define EFT_M2              0x02        /* M2 */
#define EFT_PORT_M1         0x03        /* portable */
#define EFT_VENDING_MACHINE 0x05        /* tpe automate */
#define EFT_M8              0x08        /* M8 Elite Diffusion */
#define EFT_MPLUS           0x0D        /* M Plus */
#define EFT_PORT_MPX        0x0F        /* portable MPX */
#define EFT_MPX             0x11        /* MPX */

/* --- Background dialing --- */
unsigned char   Os__backgnd_dial(DIAL_PARAM *pt);      /* start background dialing */
unsigned char   Os__check_backgnd_dial(unsigned short time_out); /* check background dialing result */



//int Os__bar_scanner( unsigned char *coder, int readlen, char exitcode);
//int Os__barscanner( unsigned char *coder, int readlen, char exitcode);
/*
*  WIFI 2010-04-19
*/
unsigned char OSWIFI_Init(POSWIFICFG pOSWIFICfg);
unsigned char OSWIFI_ParamConfig(PWIFIPARAMCFG	pWIFIParamCfg);
unsigned char OSWIFI_NetConfig(PWIFINETCFG  pWIFIConfig);
//unsigned char OSWIFI_Connect(ULONG ulPeerAddr,USHORT uiPeerPort,UINT uiTimeOut);
//unsigned char OSWIFI_Send(UCHAR *pucInBuf,USHORT uiInLen);
//unsigned char OSWIFI_Recv(UCHAR *pucBuf,USHORT* puiInLen,UINT uiTimeout);
//unsigned char OSWIFI_Close(void);
unsigned char OSWIFI_Stop(void);

unsigned char OSWIFI_Connect(int* piTCPHandle,
                             unsigned long ulPeerAddr,
                             unsigned short uiPeerPort,
                             unsigned int uiTimeout);
unsigned char OSWIFI_Send(int  iTCPHandle,
                          unsigned char *pucInBuf,
                          unsigned short uiInLen);

unsigned char OSWIFI_Recv(int  iTCPHandle,
                          unsigned char *pucInBuf,
                          unsigned short *puiInLen,
                          unsigned int uiTimeout);                        

unsigned char OSWIFI_Close(int  iTCPHandle);

unsigned char OSWIFI_Open(int *iTCPHandle);
/*
* AES DES 3DES
*/
void des(UCHAR *binput, UCHAR *boutput, UCHAR *bkey);
void desm1(UCHAR *binput, UCHAR *boutput, UCHAR *bkey);

/* TripleDES_Encrypt(),TripleDES_Decrypt()，加解密的数据和结果为8byte，密钥长度为16byte*/
int  TripleDES_Encrypt(UCHAR *binput, UCHAR *boutput, UCHAR *bkey);
int  TripleDES_Decrypt(UCHAR *binput, UCHAR *boutput, UCHAR *bkey);

/* SHA1_Hardware_Compute()为SHA1算法, uiInLen: 计算数据的长度，不超过8k byte, 结果为20byte*/
int SHA1_Hardware_Compute(UCHAR *pucInData,UINT uiInLen,UCHAR *pucDigest);

/*  SHA256_Hardware_Compute()为SHA256算法，uiInLen: 计算数据的长度，不超过8k byte,  结果为32byte */
int SHA256_Hardware_Compute(unsigned char *pucInData,unsigned int uiInLen,unsigned char *pucDigest);

/*TripleDES_Hardware() 为3DES加解密接口，pucIndata传入被加解密的数据，uiInputLen:*/
/*  被加解密数据的长度，不大于2048byte，uiMode: bit0置0为加密，bit0置1解密，*/
/* bit1置0为ECB模式，bit1置1为CBC模式，pucInitVector: CBC模式时为初始向量*/
/*ECB模式时为0，  pucKey:加解密的密钥；uiKeylen: 密钥长度，如果是16， */
/* 则认为K1=K3，也可以是24byte长度,   pucOutdata: 加解密后的数据 */
int TripleDES_Hardware(unsigned char *pucIndata, unsigned int uiInputLen,unsigned char uiMode, 
	unsigned char *pucInitVector,unsigned char *pucKey,unsigned int uiKeylen,unsigned char *pucOutdata);

/*Get_Random()获取随机数功能，一次获取最多128byte，传入的uiLen不能大于128*/
int Get_Random(unsigned char uiLen,unsigned char *pucRandom); 
int Get_Security_Status(unsigned int *puiStatus);
int Set_Security_Initial_Status(void);

void codage_ede( UCHAR * in , UCHAR * kk , UCHAR * out);
void codage_ded( UCHAR * in , UCHAR * kk , UCHAR * out);
void aesInit( unsigned char * tempbuf );
void aesDecrypt(unsigned char *buffer, unsigned char *chainBlock);
void aesEncrypt(unsigned char * buffer, unsigned char * chainBlock );
unsigned char aesBlockDecrypt(unsigned char Direct,unsigned char *ChiperDataBuf,unsigned char DataLen);


/*
*mifare
*/
int  CLASSIC_CMD_PollCard(unsigned char *pucCardType, unsigned char *pucUid,
                          unsigned char *pucUidLen, unsigned char *pucAtq, unsigned char *pucAtqLen );
                          
                          
int CLASSIC_CMD_AuthBlock( unsigned char ucAuthType, unsigned char ucSectorIndex,
                           unsigned char ucKeyType, unsigned char *pucKey );
                           
int CLASSIC_CMD_ReadBlock(unsigned char ucBlockIndex, unsigned char *pucBlockData );
int CLASSIC_CMD_WriteBlock(unsigned char ucBlockIndex, unsigned char *pucBlockData );
int CLASSIC_CMD_Reset(int delay);
int CLASSIC_CMD_Wupa(unsigned char *pucRevData, unsigned char *pucRevLen );
int CLASSIC_CMD_Select(unsigned char *pucUid,
                       unsigned char ucUidLen, unsigned char *pucSak );
int CLASSIC_CMD_Open(void);
int CLASSIC_CMD_Close(void);

/*usb*/
int Os__check_usb_ready(void);

/*tf*/
int Os__check_tf_ready(void);

/* Mifare card function */
unsigned char Os__MIFARE_Anti( unsigned char ucAntiEn,unsigned char *pucCardId,unsigned char *pucCardIdLen,unsigned char *pucCardClass );
unsigned char Os__MIFARE_Select( unsigned char *pucCardID,unsigned char ucCardIDLen,unsigned char *pucCardClass );
unsigned char Os__MIFARE_Auth( unsigned char ucAuthType,unsigned char ucSectorIndex,unsigned char ucKeyType,
				unsigned char *pucKey );
unsigned char Os__MIFARE_SendData( unsigned char ucCheckMode,unsigned char *pucSendData,unsigned int uiSendLen );
unsigned char Os__MIFARE_SendRevFrame( unsigned char *pucSendFrame,unsigned int uiSendLen,
								unsigned char *pucRevFrame,unsigned int *puiRevLen,unsigned int uiTimeDelay );
unsigned char Os__MIFARE_DeSelect( void );
unsigned char Os__MIFARE_Halt( void );
unsigned char Os__MIFARE_Remove( void );
unsigned char Os__MIFARE_PowerRF( unsigned char ucOnOff );
DRV_OUT *Os__MIFARE_Polling(void);
unsigned char Os__MIFARE_Active(void);


#endif
