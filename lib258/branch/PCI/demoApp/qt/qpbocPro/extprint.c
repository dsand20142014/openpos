#include <include.h>
#include <global.h>

#define HALIREXTPRNADDR				0x70

#define EXTPRINTCMD_FEED			0x17
#define EXTPRINTCMD_REVERSE			0x18
#define EXTPRINTCMD_PRINTGRAPH		0x39
#define EXTPRINTCMD_PRINTGB2312		0x3A
#define EXTPRINTCMD_PRINTASCII		0x3C
#define EXTPRINTCMD_PRINTBEGIN      0x3E
#define EXTPRINTCMD_PRINTEND        0x3F

#define EXTPRINT_MAX_WAITIME		4000

//#define PS100						

unsigned char ext__CheckBase(void);

#ifdef PS100
#else
unsigned char EXTPRINT_ext_xprintASCII(unsigned char ucFont,unsigned char *pucPtr)
{
	unsigned char aucSendBuf[96],aucRevBuf[16];
	unsigned char ucResult,ucI;
	unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
	aucSendBuf[ucI++]=EXTPRINTCMD_PRINTASCII;
	aucSendBuf[ucI++]=ucFont;
	uiSendLen=strlen((char *)pucPtr);
	memcpy( &aucSendBuf[ucI],pucPtr,uiSendLen );
	uiSendLen+=2;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==0 )
		ucResult=aucRevBuf[0];		
	return ucResult;
}

unsigned char EXTPRINT_XFeed(unsigned char ucLineNB)
{
unsigned char aucSendBuf[96],aucRevBuf[16];
unsigned char ucResult,ucI;
unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
    aucSendBuf[ucI++]=EXTPRINTCMD_FEED;
    aucSendBuf[ucI++]=0x30+ucLineNB;
    uiSendLen=2;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,
			aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==0 )
		ucResult=aucRevBuf[0];		
	return ucResult;
}

unsigned char EXTPRINT_XReverseFeed(unsigned char ucLineNB)
{
unsigned char aucSendBuf[96],aucRevBuf[16];
unsigned char ucResult,ucI;
unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
    aucSendBuf[ucI++]=EXTPRINTCMD_REVERSE;
    aucSendBuf[ucI++]=0x30+ucLineNB;
    uiSendLen=2;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,
			aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==0 )
		ucResult=aucRevBuf[0];		
	return ucResult;
}

unsigned char EXTPRINT_XBEGINPRINT(void)
{
    unsigned char aucSendBuf[96],aucRevBuf[16];
    unsigned char ucResult,ucI;
    unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
    aucSendBuf[ucI++] = EXTPRINTCMD_PRINTBEGIN;
    uiSendLen=1;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,
			aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==0 )
		ucResult=aucRevBuf[0];		
	return ucResult;
}

unsigned char EXTPRINT_XENDPRINT(void)
{
    unsigned char aucSendBuf[96],aucRevBuf[16];
    unsigned char ucResult,ucI;
    unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
    aucSendBuf[ucI++] = EXTPRINTCMD_PRINTEND;
    uiSendLen=1;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,
			aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==0 )
		ucResult=aucRevBuf[0];		
	return ucResult;
}

unsigned char EXTPRINT_ext_xprintGB2312(unsigned char ucASCFont,unsigned char ucGBFont,unsigned char *pucPtr)
{
	unsigned char aucSendBuf[96],aucRevBuf[16];
	unsigned char ucResult,ucI;
	unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
	aucSendBuf[ucI++]=EXTPRINTCMD_PRINTGB2312;
	aucSendBuf[ucI++]=ucASCFont;
	aucSendBuf[ucI++]=ucGBFont;
	uiSendLen=strlen((char *)pucPtr);
	memcpy( &aucSendBuf[ucI],pucPtr,uiSendLen );
	uiSendLen+=3;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,
			aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==0 )
		ucResult=aucRevBuf[0];		
	return ucResult;
}

unsigned char EXTPRINT_ext_xprintGraph(unsigned char *pucInData,unsigned char ucInLen)
{
unsigned char aucSendBuf[96],aucRevBuf[16];
unsigned char ucResult,ucI;
unsigned int uiSendLen,uiRevLen;

	ucI=0;
	uiRevLen=sizeof(aucRevBuf);
	memset( aucSendBuf,0,sizeof(aucSendBuf) );
	memset( aucRevBuf,0,sizeof(aucRevBuf) );
    aucSendBuf[ucI++]=EXTPRINTCMD_PRINTGRAPH;
    memcpy( &aucSendBuf[ucI],pucInData,ucInLen );
    uiSendLen+=ucI+ucInLen;
	ucResult=OSIR_SendReceive( HALIREXTPRNADDR,EXTPRINT_MAX_WAITIME,
			aucSendBuf,uiSendLen,aucRevBuf,&uiRevLen );
	if( ucResult==0 )
		ucResult=aucRevBuf[0];		
	return ucResult;    
}
#endif

unsigned char ext__GB2312_xprint(unsigned char *pucPtr, unsigned char ucGBFont)
{
	#ifdef PS100
		return(Os__GB2312_xprint(pucPtr,ucGBFont));
	#else
		if(ext__CheckBase()==0)
	    	return(EXTPRINT_ext_xprintGB2312(0x00,ucGBFont,pucPtr));
	#endif
}

unsigned char ext__xprint(unsigned char *pucPtr)
{
	#ifdef PS100
		return(Os__xprint(pucPtr));
	#else
		if(ext__CheckBase()==0)
	    return(EXTPRINT_ext_xprintASCII(0x00,pucPtr));
	#endif
}

unsigned char ext__graph_xprint(unsigned char *pucInData,unsigned char ucInLen)
{
	#ifdef PS100
		return(Os__graph_xprint(pucInData,ucInLen));
	#else
		if(ext__CheckBase()==0)
	    return(EXTPRINT_ext_xprintGraph(pucInData,ucInLen));
	#endif
}

unsigned char ext__xlinefeed(unsigned char ucLineNB)
{
	#ifdef PS100
		return(Os__xlinefeed(ucLineNB));
	#else
		if(ext__CheckBase()==0)
	    return(EXTPRINT_XFeed(ucLineNB));
	#endif
}	

unsigned char ext__xlineReverse(unsigned char ucLineNB)
{
	#ifdef PS100
		return(OSPRN_XReverseFeed(ucLineNB));
	#else
		if(ext__CheckBase()==0)
	    return(EXTPRINT_XReverseFeed(ucLineNB));
	#endif
}

unsigned char ext_xBeginPrint(void)
{
	#ifdef PS100
		return(0);
	#else
		if(ext__CheckBase()==0)
		  return(EXTPRINT_XBEGINPRINT());
	#endif
}

unsigned char ext_xEndPrint(void)
{
	#ifdef PS100
		return(0);
	#else
		if(ext__CheckBase()==0)
			return(EXTPRINT_XENDPRINT());
	#endif
}


unsigned char ext__CheckBase(void)
{
#ifdef PS100
    return 0;
#else

	unsigned char ucResult = 0;
	if( Os__CheckBase() )
	{
		Os__light_on();
		 memset(myString,0,sizeof(myString));
         memcpy(myString[2],"ËØ∑ÊîæÂõûÂ∫ßÊú∫ÔºÅ",sizeof("ËØ∑ÊîæÂõûÂ∫ßÊú∫ÔºÅ"));
		Os__clr_display(255);
         sem_post(&binSem1);
         sem_wait(&binSem2);
		Os__GB2312_display(1,0,(uchar *)"«Î∑≈ªÿ◊˘ª˙£°");
	}else
	{
		return 0;
	}
	
	while(1)
	{
		if( !Os__CheckBase() )
		{
			break;
		}
	}	
	return 0;
#endif

}




unsigned char ext__GB2312_xprint_big(unsigned char *pucPtr, unsigned char ucGBFont)//”°
{
#ifdef PS100
    return 0;
#else
	if(ext__CheckBase()==0)
        return(EXTPRINT_ext_xprintGB2312(0x31,ucGBFont,pucPtr));
    return 0;
#endif
}


