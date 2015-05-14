#ifndef		_EMVPARAM_H
#define		_EMVPARAM_H

#ifndef		EMVPARAM_DATA
#define		EMVPARAM_DATA 		extern
#endif

#ifdef		__cplusplus
extern "C"
{
#endif

//#include	<AppSelect.h>
//#include	<DataAuth.h>
#include	<EMV41.h>
#include	<include.h>
#if SANDREADER
#include <QPBOC.h>
extern const QTERMCONFIG	 QTermConfigDefault;
extern const	QREADERPARAM ConstReaderDefault[];
#endif
extern UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut);
extern const TERMSUPPORTAPP TermSupportAppDefault[];
extern const CAPK	 TermSupportCAPKDefault[];
extern const TERMCONFIG	 TermConfigDefault;

extern const	EMV_CONSTPARAM	ConstParamDefault[];


#ifdef		__cplusplus
}
#endif

#endif
