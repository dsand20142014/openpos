
#ifndef		_GLOBAL_H
#define		_GLOBAL_H

#ifndef		GLOBAL_DATA
#define		GLOBAL_DATA		extern
#endif

#include	<EMV41.h>
#include	<test/include.h>
#include <semaphore.h>

#ifdef	__cplusplus
extern "C"
{
#endif

GLOBAL_DATA  TERMCONFIG	 	EMVConfig;
GLOBAL_DATA	 UCHAR		 	ucTermCAPKNum;
GLOBAL_DATA	 CAPK		 	TermCAPK[MAXTERMCAPKNUMS];
GLOBAL_DATA	 UCHAR		 	ucTermAIDNum;
GLOBAL_DATA  TERMSUPPORTAPP	TermAID[MAXTERMAIDNUMS];
GLOBAL_DATA	 UCHAR			ucIPKRevokeNum;
GLOBAL_DATA	 IPKREVOKE		IPKRevoke[MAXIPKREVOKENUMS];
GLOBAL_DATA	 UCHAR			ucExceptFileNum;
GLOBAL_DATA	 EXCEPTPAN		ExceptFile[MAXEXCEPTFILENUMS];
GLOBAL_DATA	 EMVCONSTPARAM		ConstParam;
GLOBAL_DATA	 TRANSREQINFO TransReqInfo;
UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut);

//UCHAR 	MagTrack2[40];
unsigned char OnEve_power_on(void);
GLOBAL_DATA unsigned char Os__xget_key11();

GLOBAL_DATA char myString[8][256];

GLOBAL_DATA sem_t binSem1;
GLOBAL_DATA sem_t binSem2;

GLOBAL_DATA int emitSignal();

GLOBAL_DATA bool ThreadCloseFlag ;

GLOBAL_DATA  void Os__clr_display11(unsigned char line );
GLOBAL_DATA void Os__clr_display_pp11(uchar line);

GLOBAL_DATA void Os__display_pp11(uchar line,uchar col,uchar *text);
GLOBAL_DATA void Os__display11(uchar line,uchar col,uchar *text);
GLOBAL_DATA void OSMMI_DisplayASCII11 (unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char * text);
GLOBAL_DATA void OSMMI_GB2312Display11(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *text);
GLOBAL_DATA void Os__GB2312_display11(unsigned char line, unsigned char Column, unsigned char *text);
GLOBAL_DATA unsigned char Os__GB2312_display_pp (unsigned char line, unsigned char   Column, unsigned char * text);




#ifdef	__cplusplus
}
#endif
#endif
