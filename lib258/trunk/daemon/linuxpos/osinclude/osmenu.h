#ifndef __OSMENU_H__
#define __OSMENU_H__

#define OSMENUERR_SUCCESS		0x00
#define OSMENUERR_ABORT			0x01

#define OSMENU_PAGEMENUITEM		6

typedef struct
{
	unsigned char *pucText;
	unsigned char (*pfnProc)(void);
}OSMENU;

unsigned char OSMENU_Display(unsigned char *pucTitle,OSMENU *pMENU);

#endif
