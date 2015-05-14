/*
	Operating System
--------------------------------------------------------------------------
	FILE  osapp.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2005-03-10		Xiaoxi Jiang
    Last modified :	2005-03-10		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

//#include <osevent.h>

#ifndef __OSAPP_H
#define __OSAPP_H

#ifndef OSAPPDATA
#define OSAPPDATA extern
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#define OSAPPERR_SUCCESS        0x00
#define OSAPPERR_TOOLARGE       0x01
#define OSAPPERR_LOAD           0x02

#define OSAPPID_OS              0x00

#define OSAPPTAG_FILE           0x40
#define OSAPPTAG_TYPE           0x41
#define OSAPPTAG_ID             0x42
#define OSAPPTAG_NAME           0x43
#define OSAPPTAG_LABEL          0x44
#define OSAPPTAG_CREATETIME     0x48
#define OSAPPTAG_ORIGINALSIZE   0x4D
#define OSAPPTAG_CRC            0x4E
#define OSAPPTAG_DATA           0x4F

#define OSAPP_MAXAPPNB          10
#define OSAPP_FILENAMELEN       21
#define OSAPP_LABELLEN          40
#define OSAPP_MAXAPPSIZE        0x40000

#define OSAPP_SDRAMBLOCKSIZE    0x1000
#define OSAPP_SDRAMRUNADDR      0xC000000
#define OSAPP_SDRAMRUNSIZE      0x200000
#define OSAPP_SDRAMLOADADDR     0xC300000
#define OSAPP_SDRAMLOADSIZE     0x100000

#define OSDOWN_APPFILEID        "%DWN-1.0-SAND%"

    typedef struct
    {
        unsigned char ucID;
        char acFileName[OSAPP_FILENAMELEN];
        unsigned char aucLabel[OSAPP_LABELLEN];
        unsigned int uiAddr;
        unsigned int uiLen;
    }OSAPP;


    unsigned char OSAPP_Main(void);


    unsigned char OSAPP_Maintence(void);
    unsigned char OSAPP_Download(void);
    unsigned char OSAPP_Delete(void);
    unsigned char OSAPP_Check(unsigned char *pucFile,unsigned int uiFileLen);
    unsigned char OSAPP_DeleteDataFile(unsigned char ucAppID);

    unsigned char OSAPP_Load(void);
    unsigned char OSAPP_List(void);

    OSAPPDATA unsigned short ucOSCurrAppID;
    OSAPPDATA unsigned short ucOSMasterAppID;
    OSAPPDATA unsigned short ucOSAppNB;
    OSAPPDATA OSAPP OSApp[OSAPP_MAXAPPNB];

#ifdef __cplusplus
}
#endif

#undef OSAPPDATA

#endif
