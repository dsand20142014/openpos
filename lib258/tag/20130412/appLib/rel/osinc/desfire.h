/*
	Hardware Abstraction Layer
--------------------------------------------------------------------------
	FILE  desfire.h
--------------------------------------------------------------------------
*/

#ifndef __DESFIRE_H__
#define __DESFIRE_H__

#ifndef DESFIREDATA
#define DESFIREDATA	extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

DESFIREDATA unsigned char aucErrCode[][2];

#define DESFIRE_SEND_MAX_LEN	128
#define DESFIRE_REV_MAX_LEN		128

#define DESFIRE_ERR_SUCCESS			0x00
#define DESFIRE_ERR_NOCHANGE		0x01
#define DESFIRE_ERR_OUTOFMEM		0x02
#define DESFIRE_ERR_CMDCODE			0x03
#define DESFIRE_ERR_CRCMAC			0x04
#define DESFIRE_ERR_NOSUCHKEY		0x05
#define DESFIRE_ERR_CMDLENGH		0x06
#define DESFIRE_ERR_DENIED			0x07
#define DESFIRE_ERR_PARAMETER		0x08
#define DESFIRE_ERR_APPNOTFOUND		0x09
#define DESFIRE_ERR_APPINTEGRITY	0x0A
#define DESFIRE_ERR_AUTHENTICATION	0x0B
#define DESFIRE_ERR_NEEDFRAME		0x0C
#define DESFIRE_ERR_BOUNDARY		0x0D
#define DESFIRE_ERR_PICCINTEGRITY	0x0E
#define DESFIRE_ERR_CMDABORTED		0x0F
#define DESFIRE_ERR_PICCDISABLE		0x10
#define DESFIRE_ERR_APPCOUNT		0x11
#define DESFIRE_ERR_DUPLICATE		0x12
#define DESFIRE_ERR_EEPROM			0x13
#define DESFIRE_ERR_FILENOTFOUND	0x14
#define DESFIRE_ERR_FILEINTEGRITY	0x15
#define DESFIRE_ERR_UNKNOWN			0x16

unsigned char DESFIRE_Process( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder );

#ifdef __cplusplus
}
#endif

#endif
