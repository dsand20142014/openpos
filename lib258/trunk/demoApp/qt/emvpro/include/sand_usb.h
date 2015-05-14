/*
skb25_usb.h
*/
#ifndef __SAND_USB_H__
#define __SAND_USB_H__

#ifdef __cplusplus 
extern "C" { 
#endif


#ifndef __SKB25_USB__
#define USB_EXTERN extern
#else
#define USB_EXTERN
#endif

USB_EXTERN unsigned char OSUSB_Wait_InsertDisk(unsigned int timeout);
USB_EXTERN unsigned char OSUSB_DirOrFile_List();
USB_EXTERN unsigned char OSUSB_OpenFile(unsigned char  *pucFileName);
USB_EXTERN unsigned char OSUSB_CreateDir(unsigned char *pFilename);
USB_EXTERN unsigned char OSUSB_ReadFile(unsigned int uiOffset, unsigned char *pucData,
                                        unsigned int *piLen);
USB_EXTERN unsigned char OSUSB_WriteFile(unsigned int uiOffset,unsigned char *pucData,
        unsigned int uiLen);
USB_EXTERN unsigned char OSUSB_AppendFile(unsigned char *pucData,unsigned int uiLen);
USB_EXTERN unsigned char OSUSB_FileSize(unsigned char *pFilename,unsigned int *piSize);
USB_EXTERN unsigned char OSUSB_CloseFile(void);

#ifdef __cplusplus 
}
#endif

#endif
