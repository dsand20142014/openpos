
#ifndef			_QERROR_H
#define			_QERROR_H

#include		<QTypeDef.h>

#define			QERROR_SUCCESS				0
#define			QERROR_FUNCNOTSUPPORT		1
#define			QERROR_CARDBLOCKED			2
#define			QERROR_PSENOTFIND			3
#define			QERROR_PSEBLOCKED			4
#define         QERROR_ICCCOMAND			5
#define			QERROR_ICCDATA				6
#define			QERROR_PARAM					7
#define			QERROR_MISSDATA				8
#define			QERROR_RESELECT				9
#define			QERROR_DATA					10
#define			QERROR_DATAAUTH				11
#define			QERROR_CAPKNOTFOUND			12
#define			QERROR_CERTIFICATIONMISS   13
#define			QERROR_CERTIFICATIONLEN	14
#define			QERROR_HASHVALUE			15
#define			QERROR_CAPKINDEXNOTFOUND   16
#define			QERROR_CANCEL				17
#define			QERROR_NEEDARQC				18
#define			QERROR_EXTERNALAUTH			19
#define			QERROR_ISSUREDATA			20
#define			QERROR_OPENCOM				21
#define			QERROR_SENDDATA				22
#define			QERROR_RECEIVEDATA			23
#define			QERROR_TAGNOTFIND			24
#define			QERROR_NOSUPPORTAPP			25
#define			QERROR_IPKREVOKE			26
#define			QERROR_PARAMUPDATE			27
#define			QERROR_VELOCITYCHECK		28
#define			QERROR_VERIFY				29
#define			QERROR_SAVEFILE				30
#define			QERROR_READFILE				31
#define			QERROR_WRITE					32
#define			QERROR_INVALIDSERVICE		33
#define			QERROR_SCRIPTLIMIT			34
#define			QERROR_CAPKINVALID			35
#define			QERROR_OVERFLOW				36
#define			QERROR_CID					37
#define			QERROR_DATE					38
#define			QERROR_RESPFORMAT			39
#define			QERROR_BYPASS				40
#define			QERROR_FALLBACK				41
#define			QERROR_AIDBLOCKED			42
#define			QERROR_COLLISION			43
#define			QERROR_TRANSOVER			44
#define			QERROR_EXPDATE				55
#define			QERROR_SDA					56
#define			QERROR_AIDMISMATCH		58

/*********** VISA CONTACTLESS ERROR ************/
#define			QERROR_CHANGEINTERFACE		45


typedef		struct
{
	UCHAR		ucMsgIndex;
	UCHAR		aucErrMsg[20];
}QERRMSG;

void 	QERROR_DisplayMsg(UCHAR	ucMsgId);

#endif
