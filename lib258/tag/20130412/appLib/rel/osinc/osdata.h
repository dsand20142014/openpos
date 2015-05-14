/*
	Operating System
--------------------------------------------------------------------------
	FILE  osdata.h
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

#ifndef __OSDATA_H_
#define __OSDATA_H_

#include <osmodem.h>

#ifndef __OSDATA
#define __OSDATA extern
#endif

#if 0
/* APPLICATION CONFIGURATION DATA */
__OSDATA  uchar                   app_val;        /* nbr appli valide */
__OSDATA  struct setat *          ptetat;         /* pt sur etat appli */
__OSDATA  struct setat            etatapp[MAX_APPLI+1];/* table etat des appli + systeme */
__OSDATA  uchar                   nb_page_code;   /* Nb Page code */
__OSDATA  uchar                   nb_page_data;   /* Nb Page data */
__OSDATA  uchar                   AppliNum;       /* Physical number of the application */
__OSDATA  uchar                   AppliNr;        /* Application Number */
__OSDATA  uchar                   PAGE_NBR;       /* Current code page */
__OSDATA  uchar                   PAGE_APP;       /* Current application code page */
__OSDATA  uchar                   PAGE_OS_APPLI;  /* CODE PAGE NUMBER OS APPLI */

/* EVENT */
__OSDATA  struct seven            even;           /* Event structure */
__OSDATA  union seve_in           eve_in;         /* Input Event Structure */
__OSDATA  struct seve_out         ret_os;
__OSDATA  MONOS_EVENT_DETECTED    rec_eve;        /* Received Event */
__OSDATA  MONOS_EVENT_TRT_REPORT  cpt_eve;        /* Event treatment report */
#endif

/* TIMERS */
__OSDATA  unsigned int            uiAwakeTimeout; /* To for AWAKE */
__OSDATA  unsigned int            uiLightTimeout;   /* To for light */

/* UART */
__OSDATA COM_PARAM ComParam;
//__OSDATA MODEMDRV *pDrv;

#endif
