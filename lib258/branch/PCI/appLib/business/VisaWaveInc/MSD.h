
#ifndef		_MSD_H
#define		_MSD_H

#ifndef		MSD_DATA
#define		MSD_DATA	extern
#endif

#include	<QTypeDef.h>
#include	<QDef.h>
#include  <QTransTag.h>
#include	<QError.h>
#ifdef	__cplusplus
extren "C"
{
#endif

#define  TTQ_MSDSUPPORT		QRightRotate(7) 
#define  TTQ_CSVSDCSUPPORT	QRightRotate(6) 
#define  TTQ_QVSDCSUPPORT		QRightRotate(5)
#define  TTQ_CVSDCSUPPORT		QRightRotate(4)
#define  TTQ_OFFLINEONLY		QRightRotate(3)
#define  TTQ_ONLINEPINSUPPORT	QRightRotate(2)
#define  TTQ_SIGNATURESUPPORT	QRightRotate(1)

#define  TTQ_ONLINECRYPTREQ	QRightRotate(7)
#define  TTQ_CVMREQ			QRightRotate(6)

#define			MSDTRANSACTION				0x91
#define			QVSDCTRANSACTION			0x07

#define			TRACK1MAXLEN				79
#define			TRACK2MAXLEN				37
#define			TRACK3MAXLEN				105


typedef enum {ENTRYMODE_MSD,ENTRYMODE_CSVSDC,ENTRYMODE_QVSDC} ENTRYMODE; 



typedef struct
{
	BOOL		bTrack1;
	UCHAR		ucTrack1Len;
	UCHAR		aucTrack1[QMAXTRACK1LEN];
	UCHAR		ucDynTrack1Len;
	UCHAR		aucDynTrack1[QMAXTRACK1LEN];
	
	UCHAR		ucTrack2Len;
	UCHAR		aucTrack2[QMAXTRACK2LEN];
	UCHAR		ucDynTrack2Len;
	UCHAR		aucDynTrack2[QMAXTRACK2LEN];
	
}MAGSTRIPEINFO,*PMAGSTRIPEINFO;

UCHAR	MSD_TrackInfo(void);
UCHAR	MSD_Track2Info(void);
UCHAR	MSD_Track1Info(void);
UCHAR	MSD_DigitNums(PUCHAR pStr);

MSD_DATA MAGSTRIPEINFO	MagStripeInfo;

#ifdef	__cplusplus
}
#endif
#endif
