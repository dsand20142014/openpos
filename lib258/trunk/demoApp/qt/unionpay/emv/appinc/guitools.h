/*******************************************
 File:			GuiTools.h
 Creator:		Zeng Shu
 Maintainer:	Zeng Shu
 CopyRight:		Shanghai Sand ITS Co., Ltd
********************************************/
#ifndef __GUITOOLS_H__
#define __GUITOOLS_H__

#ifdef __cplusplus
extern "C" {
#endif

int guiGb18030ToUtf8(char *inbuf,int inlen,char *outbuf,int outlen);
int guiUtf8ToGb18030(char *inbuf,int inlen,char *outbuf,int outlen);
int guiUtf8ToGb2312(char *inbuf,int inlen,char *outbuf,int outlen);
int guiUtf8ToBig5(char *inbuf,int inlen,char *outbuf,int outlen);
int guiGbkToUtf8(char *inbuf,int inlen, char *outbuf,int outlen);

#ifdef __cplusplus
}
#endif

#endif //__GUITOOLS_H__
