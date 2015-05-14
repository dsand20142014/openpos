
#ifndef		_GLOBAL_H
#define		_GLOBAL_H

#ifndef		GLOBAL_DATA
#define		GLOBAL_DATA		extern
#endif

#include	<QPBOC.h>
#include	<include.h>
#include <semaphore.h>
#ifdef	__cplusplus
extern "C"
{
#endif

GLOBAL_DATA  QTERMCONFIG	 	QConfig;
GLOBAL_DATA	 UCHAR		 	ucQTermCAPKNum;
GLOBAL_DATA	 QCAPK		 	QTermCAPK[QMAXTERMCAPKNUMS];
GLOBAL_DATA	 UCHAR		 	ucQTermAIDNum;
GLOBAL_DATA  QTERMSUPPORTAPP	QTermAID[QMAXTERMAIDNUMS];
GLOBAL_DATA	 UCHAR			ucQIPKRevokeNum;
GLOBAL_DATA	 QIPKREVOKE		QIPKRevoke[QMAXIPKREVOKENUMS];
GLOBAL_DATA	 UCHAR			ucQExceptFileNum;
GLOBAL_DATA	 QEXCEPTPAN		QExceptFile[QMAXEXCEPTFILENUMS];
GLOBAL_DATA	 QEMVCONSTPARAM		QConstParam;
GLOBAL_DATA	 QTRANSREQINFO QTransReqInfo;

GLOBAL_DATA char myString[8][256];

GLOBAL_DATA sem_t binSem1;
GLOBAL_DATA sem_t binSem2;

GLOBAL_DATA void emitSignal();

GLOBAL_DATA bool ThreadCloseFlag ;

GLOBAL_DATA  void Os__clr_display11(unsigned char line );
GLOBAL_DATA void Os__clr_display_pp11(uchar line);

GLOBAL_DATA void Os__display_pp11(uchar line,uchar col,uchar *text);
GLOBAL_DATA void Os__display11(uchar line,uchar col,uchar *text);
GLOBAL_DATA void OSMMI_DisplayASCII11 (unsigned char ucFont,unsigned char ucRow,unsigned char ucCol, unsigned char * text);
GLOBAL_DATA void OSMMI_GB2312Display11(unsigned char ucFont,unsigned char ucRow, unsigned char ucCol, unsigned char *text);
GLOBAL_DATA void Os__GB2312_display11(unsigned char line, unsigned char Column, unsigned char *text);
GLOBAL_DATA unsigned char Os__GB2312_display_pp11 (unsigned char line, unsigned char   Column, unsigned char * text);

#ifdef	__cplusplus
}
#endif
#endif
