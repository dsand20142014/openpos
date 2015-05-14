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
#include	<QPBOC.h>
#include	<include.h>

extern UCHAR	QICC(QICCIN *pQICCIn ,QICCOUT *pQICCOut);
extern const QTERMSUPPORTAPP TermSupportAppDefault[];
extern const QCAPK	 TermSupportCAPKDefault[];
extern const QTERMCONFIG	 TermConfigDefault;
extern const	QEMVCONSTPARAM	ConstParamDefault[];
extern const	QREADERPARAM	ConstReaderDefault;
#ifdef		__cplusplus
}
#endif

#endif
