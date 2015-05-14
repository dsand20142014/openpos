/*******************************************
 File:			GuiTools.c
 Creator:		Zeng Shu
 Maintainer:	Zeng Shu
 CopyRight:		Shanghai Sand ITS Co., Ltd
********************************************/

#include "iconv.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>



static int guiCodeConvert(char *from_charset,char *to_charset, \
						  char *inbuf,int inlen,char *outbuf,int outlen)
{
    iconv_t cd;
    int rc;
    const char **pin = (const char **)(&inbuf);
    char **pout = &outbuf;

    cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,outlen);
    if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
    iconv_close(cd);
    return 0;
}

/*gb18030 to utf-8*/
int guiGb18030ToUtf8(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return guiCodeConvert("gb18030", "utf-8", inbuf,inlen,outbuf,outlen);
}

/*utf-8 to gb18030*/
int guiUtf8ToGb18030(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return guiCodeConvert("utf-8","gb18030",inbuf,inlen,outbuf,outlen);
}

/*utf-8 to gb2312*/
int guiUtf8ToGb2312(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return guiCodeConvert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

/*utf-8 to big5*/
int guiUtf8ToBig5(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return guiCodeConvert("utf-8","big5",inbuf,inlen,outbuf,outlen);
}

/*GBK to utf-8*/ 
int guiGbkToUtf8(char *inbuf,int inlen, char *outbuf,int outlen)
{
	return guiCodeConvert("GBK", "utf-8", inbuf,inlen,outbuf,outlen);
}

#ifdef PROGRAM_CROSS
char* rindex(char* __s, int __c)
{
	return strrchr(__s,__c);
}
#endif
