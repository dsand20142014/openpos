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

#ifdef __cplusplus
extern "C" {
#endif

//#include <type_uns.h>           /* unsigned uchar, ushort and ulong types */
#include <osdriver.h>           /* driver types */
#include <osmodem.h>            /* communications types (modem and rs232) */
//#include <osmenu.h>             /* MENU and HEADER_APP types */
#include <osevent.h>            /* structures for events management */
#include <appdef.h>
#if 0
#include <osv42.h>              /* modem V42 communication */
#include <osswap.h>             /* data transmission between CODE pages */
#endif

/* File API definement */
#define O_READ              0x01
#define O_WRITE             0x02
#define O_CREATE            0x04
#define O_TRUNC             0x08

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


/* SAND API */
unsigned char APISAND_Check(void);
/* OS Interface */
unsigned char OSMENU_MainForApp(void);
/* General drivers */
void OSDRV_Call(DRIVER *pDrv);
void OSDRV_Abort(unsigned char ucDrvID);
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
unsigned char OSMMI_NonStdDisplay(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *pucPtr);
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
/* Printer */
unsigned char OSPRN_BeginPrint(unsigned char *pucParam);
unsigned char OSPRN_EndPrint(void);
unsigned char OSPRN_CheckPaper(unsigned char ucFlag);
DRV_OUT * OSPRN_Feed(unsigned char ucLineNB);
unsigned char OSPRN_XFeed(unsigned char ucLineNB);
DRV_OUT * OSPRN_PrintASCII(unsigned char ucFont,unsigned char *pucPtr);
unsigned char OSPRN_XPrintASCII(unsigned char ucFont,unsigned char *pucPtr);
unsigned char OSPRN_XPrintGB2312(unsigned char ucASCFont,unsigned char ucGBFont,unsigned char *pucPtr);
unsigned char OSPRN_XPrintGraph(unsigned char *pucInData,unsigned char ucInLen);
unsigned char OSPRN_XPrintNonStd(unsigned char ucFont,unsigned char *pucPtr);
/*Extern  Printer */
unsigned char OSPRN_ExtXFeed(unsigned char ucLineNB);

/* Magnetic cards reader */
DRV_OUT * OSMAG_Read(void);
DRV_OUT * OSMAG_ReadGeneral(unsigned char ucTrack1Method,
                unsigned char ucTrack2Method,
                unsigned char ucTrack3Method);
/* Smart cards and SIMs reader-writer */
unsigned char OSICC_Detect(unsigned char ucReader);
DRV_OUT *OSICC_Insert(void);
DRV_OUT *OSICC_OrderAsync(unsigned char *pucOrder,unsigned short uiLen);
DRV_OUT *OSICC_Order(DRV_IN  *pDrvIn);
void OSICC_Remove(void);
void OSICC_PowerOff(void);
/* Modem and 1st RS232 */
unsigned char OSUART_Init1(COM_PARAM *pComParam);
unsigned char OSMODEM_Dial(DIAL_PARAM *pDialParam);
unsigned char OSMODEM_BackDial(DIAL_PARAM *pDialParam);
unsigned char OSMODEM_CheckBackDial(unsigned short uiTimeout);
void OSMODEM_ReadDialParam(DIAL_PARAM *pDialParam);
unsigned char OSMODEM_SetParam(unsigned char ucParamNb,unsigned char ucValue);
unsigned char OSUART_TxChar1(unsigned char ucCh);
unsigned short OSUART_RxChar1(unsigned short uiTimeout);
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
unsigned char	OSMODEMPPP_Check(void);

/* GSM */
void OSGSM_GprsControlInit(unsigned char ucCtrl);
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
unsigned char TCP_Connect(int iTCPHandle,unsigned char* ulPeerAddr,unsigned short uiPeerPort);
unsigned char TCP_Send(int iTCPHandle,unsigned char pucInBuf,unsigned short uiInLen);
unsigned char TCP_Recv(int iTCPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);
unsigned char TCP_Close(int iTCPHandle);
/* UDP Function */
unsigned char UDP_Open(int *piUDPHandle);
unsigned char UDP_Config(int iUDPHandle,unsigned long ulPeerAddr,unsigned short uiPeerPort);
unsigned char UDP_Send(int iUDPHandle,unsigned char *pucInBuf,unsigned short uiInLen);
unsigned char UDP_Recv(int iUDPHandle,unsigned char *pucInBuf,unsigned short *puiInLen,unsigned short uiTimeout);
unsigned char UDP_Close(int iUDPHandle);

/*DNS Function*/
unsigned char	DNS_StdQuery(unsigned char* pQName,UINT *pauiHostIP,UCHAR* pucHostIPNum);
/*Infrared*/
unsigned char OSIR_SendBuf(unsigned char ucModuleID,unsigned int ucWaitTime,
			unsigned char *pSendBuf,unsigned int uiSendLen );
unsigned char OSIR_Receive(unsigned char ucModuleID,unsigned int ucWaitTime,
			unsigned char *pRevBuf,unsigned int *puiRevLen );
unsigned char OSIR_SendReceive(unsigned char ucModuleID,unsigned int ucWaitTime,
			unsigned char *pSendBuf,unsigned int uiSendLen,
			unsigned char *pRevBuf,unsigned int *puiRevLen );
/*Mobile*/
unsigned char OSMOBILE_CheckBase( void );
void OSMOBILE_PowerOff( void );
void OSMOBILE_GetBatterLevel( unsigned int *puiValue );

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
/* oscfg.h */
CONFIG_PARAM OSCFG_ReadParam(void);
unsigned char OSCFG_SetETHConfig(unsigned char *paucAddr,unsigned char *paucMask,unsigned char *paucGatewayAddr);



/* INGENICO API */

/* --- Displays and Keyboards --- */
void    Os__clr_display(uchar line);
void    Os__clr_display_pp(uchar line);
//void    Os__beep(void);
void    Os__beep_pp(void);
DRV_OUT * Os__get_key(void);
DRV_OUT * Os__get_key_pp(void);
uchar   Os__xget_key(void);
uchar   Os__xget_key_pp(void);
DRV_OUT * Os__get_amount(uchar line, uchar col, uchar *text);
uchar   * Os__xget_amount(uchar line, uchar col, uchar *text);
DRV_OUT * Os__get_varkey(uchar line, uchar col, uchar *text, uchar n)reentrant;
DRV_OUT * Os__get_varkey_pp(uchar line, uchar col, uchar *text, uchar n)reentrant;
uchar   * Os__xget_varkey(uchar line, uchar col, uchar *text, uchar n)reentrant;
uchar   * Os__xget_varkey_pp(uchar line, uchar col, uchar *text, uchar n)reentrant;
uchar   * Os__xget_minkey(uchar line, uchar col, uchar *text, uchar n,uchar m)reentrant;
DRV_OUT * Os__get_fixkey(uchar line, uchar col, uchar *text, uchar n)reentrant;
DRV_OUT * Os__get_fixkey_pp(uchar line, uchar col, uchar *text, uchar n)reentrant;
uchar   * Os__xget_fixkey(uchar line, uchar col, uchar *text, uchar n)reentrant;
uchar   * Os__xget_fixkey_pp(uchar line, uchar col, uchar *text, uchar n)reentrant;
DRV_OUT * Os__get_pin(uchar line, uchar col, uchar *text, uchar n)reentrant;
DRV_OUT * Os__get_pin_pp(uchar line, uchar col, uchar *text, uchar n)reentrant;
uchar   * Os__xget_pin(uchar line, uchar col, uchar *text, uchar n)reentrant;
void    Os__crypt(uchar *crypting_key);
void    Os__crypt_pp(uchar *crypting_key);
void    Os__display(uchar line,uchar col,uchar *text);
void    Os__display_pp(uchar line,uchar col,uchar *text);
uchar   * Os__xget_insert(uchar line, uchar col, uchar *text, uchar n,uchar charac,uchar step)reentrant;
uchar   * Os__xget_scankey(uchar line, uchar col, uchar *text, uchar n,uchar mode)reentrant;

/* --- EEPROM --- */
uchar   Os__read_eeprom(ushort start,uchar length,uchar *pt)reentrant;

/* --- SMART card ---*/
DRV_OUT * Os__ICC_insert(void);
void    Os__ICC_remove(void);
void    Os__ICC_off(void);
uchar Os__ICC_detect(uchar ReaderNb);
/* For Asynchronous smart card only */
DRV_OUT * Os__ICC_Async_order(uchar *ISO7816_order,ushort length);
/* For every kind of smart card */
DRV_OUT * Os__ICC_order(DRV_IN *pt_drv_in);

/* --- magnetic card ---*/
DRV_OUT * Os__mag_read(void);

/* --- PRINTER ---*/
DRV_OUT * Os__print(uchar *buffer);
DRV_OUT * Os__linefeed(uchar nbline);
uchar   Os__xprint(uchar *buffer);
uchar   Os__xlinefeed(uchar nbline);

/* --- Time and Date --- */
void    Os__read_date_time(uchar *pt);
void    Os__read_date(uchar *pt);
void    Os__read_time(uchar *pt);
uchar   Os__write_time(uchar *pt);
uchar   Os__write_date(uchar *pt);
void    Os__read_date_format(uchar *pt_out);
void    Os__read_time_format(uchar *pt_out);

/* --- DELAY ---*/
uchar   Os__timer_start(uint *);
void    Os__timer_stop(uint *);
uchar   Os__xdelay(uint);

/* --- SYSTEM ---*/
uchar   Os__saved_swap(uchar);
void    Os__saved_copy(uchar *,uchar *,ushort)large;
void    Os__saved_set(uchar *,uchar,ushort)large;
void    Os__code_copy(uchar *,uchar *,ushort)large;
ushort  Os__crc16(uchar *pt,ushort length)large;
void    Os__read_eft_id(uchar *pt);
uchar   Os__read_paper_type(void);
DRV_OUT * Os__get_appli_id(void);

/* --- GENERAL DRIVER ---*/
void    Os__call_drv(DRIVER *);
void    Os__abort_drv(uchar);
uchar   Os__wait_drv(DRV_OUT *);

/* --- MODEM ---*/
uchar   Os__init_com(COM_PARAM *pt);
void    Os__hang_up(void);
uchar   Os__txcar(uchar car);
void    Os__com_flush1(ushort recmax);
void    Os__com_flush2(void);
uchar   Os__dial(DIAL_PARAM *pt);
uchar   Os__x28(X28_PARAM *pt);
ushort  Os__rxcar(ushort timeout);
void    Os__read_dial_param(DIAL_PARAM *pt);
uchar   Os__V23_reverse_carrier(void);

/* --- COM 3 --- */
uchar   Os__init_com3(ushort r_6_7 , ushort r_4_5 , uchar r_3);
ushort  Os__rxcar3(ushort to_attente);
void    Os__txcar3(uchar car);
void    Os__end_com3(void);

/* --- "FRAME PROTOCOL" (FRENCH CASH REGISTER PROTOCOL) --- */
uchar   Os__frame_init(COM_PARAM *pt);
void    Os__frame_conf_abort(uchar);
DRV_OUT * Os__frame_send(uchar *message);
DRV_OUT * Os__frame_rcv(uchar *buffer);

/* --- I2C communication (for cheque reader) --- */
uchar   Os__cheque_init(COM_PARAM *pt);
DRV_OUT * Os__cheque_rcv(uchar *buffer);
DRV_OUT * Os__cheque_send(uchar *buffer);

/* Mifare card function */
DRV_OUT *Os__MIFARE_Request(unsigned char ucCardType,unsigned char ucReqMode);
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


/* --- Code page swapping : call a function in another page --- */
#if 0
ushort Os__get_calling_param(CALLING_PARAM *pt,void *pt_out)reentrant;
void   Os__return_calling_page(CALLING_PARAM *pt,void *pt_in,ushort length,uchar status)reentrant;
uchar  Os__call_page(CALLING_PARAM *pt,void *pt_in,ushort lg_in,void *pt_out)reentrant;
#endif

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
uchar   Os__backgnd_dial(DIAL_PARAM *pt);      /* start background dialing */
uchar   Os__check_backgnd_dial(ushort time_out); /* check background dialing result */

/* --- INFRA-RED COMMUNICATION ---*/
/* Remark : Only for "old" version of portable terminal M3 and M4 */
#if 0
uchar   Os__infrared_init(COM_PARAM *pt);
DRV_OUT * Os__infrared_send( struct seve_portable *message);
DRV_OUT * Os__infrared_rcv( struct seve_portable *buffer);
#endif

void activeDev(int iDev);


#ifdef __cplusplus
}
#endif
#endif

