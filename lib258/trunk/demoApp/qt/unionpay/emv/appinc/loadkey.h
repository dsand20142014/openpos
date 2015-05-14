/*
	SAND China
--------------------------------------------------------------------------
	FILE  LoadKey.c								(Copyright SAND 2009)
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2009-03-02		Lote
    Last modified :	2009-03-02		Lote
    Module :  Shanghai Golden Card Network
    Purpose :
        Head file for LoadKey.

    List of routines in file :

    File history :
*/

#define COM_ONE			1
#define COM_TWO		2
#define ERRName_Len								32

#define ERR_RECVTIMEOUT					0x01
#define ERR_COMMS_DATALEN			0x02
#define ERR_DATA_ETX							0x03
#define ERR_REVDATA_LEN					0x04
#define ERR_COMMS_STX						0x05
#define ERR_COMMS_DATA					0x06
#define ERR_COMMS_DATAETX			0x07
#define ERR_COMMS_SANDLEN			0x08

typedef struct
	{
		unsigned char ucErrorType;
		unsigned char ucErrorNo;		
		unsigned char aucErrorName[ERRName_Len];
	}LoadKEYError;

const	LoadKEYError	COMMErr[]=
	{
/*1*/{'0',0x01,"接收超时"},
/*2*/{'0',0x02,"数据过长"},
/*3*/{'0',0x03,"数据出错"},
/*4*/{'0',0x04,"接收数据过长"},
/*5*/{'0',0x05,"STX错误"},
/*6*/{'0',0x06,"数据接收出错"},
/*7*/{'0',0x07,"数据校验错"},
/*8*/{'0',0x08,"应答数据过长"},
/*8*/{'0',0xFF,"串口操作未知错误"}
/*8*/
	};