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
#include	<test/include.h>

extern UCHAR	EMVICC(EMVICCIN *pEMVICCIn ,EMVICCOUT *pEMVICCOut);
extern const TERMSUPPORTAPP TermSupportAppDefault[];
extern const CAPK	 TermSupportCAPKDefault[];
extern const TERMCONFIG	 TermConfigDefault;
extern const	EMVCONSTPARAM	ConstParamDefault[];

#ifdef		__cplusplus
}
#endif

#endif
