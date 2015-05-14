#include <include.h>
#include <xdata.h>
#include <global.h>

#define HALIREXTPRNADDR				0x70

#define EXTPRINTCMD_FEED			0x17
#define EXTPRINTCMD_REVERSE			0x18
#define EXTPRINTCMD_PRINTGRAPH		0x39
#define EXTPRINTCMD_PRINTGB2312		0x3A
#define EXTPRINTCMD_PRINTASCII		0x3C

#define EXTPRINT_MAX_WAITIME		4000

#if PS300
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
	if( ucResult==SUCCESS )
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
	if( ucResult==SUCCESS )
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
	if( ucResult==SUCCESS )
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
	if( ucResult==SUCCESS )
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
	if( ucResult==SUCCESS )
		ucResult=aucRevBuf[0];		
	return ucResult;    
}

unsigned char ext__GB2312_xprint(unsigned char *pucPtr, unsigned char ucGBFont)
{
    return(EXTPRINT_ext_xprintGB2312(0x00,ucGBFont,pucPtr));
}

unsigned char ext__xprint(unsigned char *pucPtr)
{
    return(EXTPRINT_ext_xprintASCII(0x00,pucPtr));
}

unsigned char ext__graph_xprint(unsigned char *pucInData,unsigned char ucInLen)
{
    return(EXTPRINT_ext_xprintGraph(pucInData,ucInLen));
}

unsigned char ext__xlinefeed(unsigned char ucLineNB)
{
    return(EXTPRINT_XFeed(ucLineNB));
}

unsigned char ext__xlineReverse(unsigned char ucLineNB)
{
    return(EXTPRINT_XReverseFeed(ucLineNB));
}

unsigned char ext__CheckBase(void)
{
	if( Os__CheckBase() )
	{
		Os__light_on();
		//Os__clr_display(255);
		//Os__GB2312_display(1,0,(uchar *)"请放回座机！");
	}else
	{
		return SUCCESS;
	}
	
	while(1)
	{
		if( !Os__CheckBase() )
		{
			if(!DataSave0.ConstantParamData.BackLightFlag)
				Os__light_off();
			break;
		}
	}
	//Os__clr_display(255);
	//Os__GB2312_display(1,0,(uchar *)"    正在打印");
	//Os__GB2312_display(2,0,(uchar *)"    请稍等...");
	return SUCCESS;
}

unsigned char ext__GB2312_xprint_big(unsigned char *pucPtr, unsigned char ucGBFont)//打印大字体
{
if(ext__CheckBase()==SUCCESS)

    return(EXTPRINT_ext_xprintGB2312(0x31,ucGBFont,pucPtr));
    return SUCCESS;
}
#endif

