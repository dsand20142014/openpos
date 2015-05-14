/*
	Operating System
--------------------------------------------------------------------------
	FILE  osmenu.h
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2002-10-03		Xiaoxi Jiang
    Last modified :	2002-10-03		Xiaoxi Jiang
    Module :
    Purpose :
        Header file.

    List of routines in file :

    File history :
*/

#ifndef __OSMENU_H__
#define __OSMENU_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OSMENUERR_SUCCESS		0x00

#define OSMENU_PAGEMENUITEM		6

typedef struct
{
	unsigned char *pucText;
	unsigned char (*pfnProc)(void);
}OSMENU;

unsigned char OSMENU_Main(void);
unsigned char OSMENU_Display(unsigned char *pucTitle,OSMENU *pMENU);

/* osdown.c */
unsigned char OSDOWN_Menu(void);

/* ostest.c */
unsigned char OSTEST_Menu(void);







/* OLD */
#define LJMP    02                      /* instruction code LJMP in 8051 */
struct  spage_header 					/* application header structure */
{
	unsigned char   jmp;                    /* long jump (LJMP) */
	unsigned short  addr_jmp;               /* application entry point address */
	unsigned short  crc;                    /* application CRC */
    unsigned short  addr_app_end;           /* application end address */
    unsigned char   app_name[5];            /* application name */
    unsigned char   app_nr;                 /* application number */
    unsigned char   app_version;            /* application version */
};
typedef struct spage_header PAGE_HEADER;

/*--- structure of a menu item ---*/
struct  smenu {
        unsigned char           message[17];    /* message to display */
        unsigned char           nr_of_func;     /* number of functions */
        unsigned char           func_nr;        /* function number */
};
typedef struct smenu MENU;


#ifdef __cplusplus
}
#endif

#endif
