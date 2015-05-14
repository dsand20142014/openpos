/*
	Operating System
--------------------------------------------------------------------------
	FILE  osmem.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2005-03-14		Xiaoxi Jiang
    Last modified :	2005-03-14		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

//#include <os.h>
#include <oserr.h>
#include <string.h>
#include <osdata.h>

#define OSMEM_SRAMPASS		"147369"

unsigned char OSMEM_SetSRAMSize(unsigned char *pucSize)
{
#if 0
    unsigned char ucStatus;
    unsigned char aucSize[10];
	unsigned char aucHexSize[5];
	unsigned long ulSize;
	DRVOUT *pdKey;

    OSMMI_ClrDisplay(ASCIIFONT57,255);
    OSMMI_DisplayASCII(ASCIIFONT57,OSDISP_MSG0ROW,0,OSMSG_SRAMPassword);
	while(1)
	{
		OSMMI_ClrDisplay(ASCIIFONT57,OSDISP_MSG1ROW);
		pdKey = OSMMI_GetPin(OSDISP_MSG1ROW,0,0,6);
		while( pdKey->ucGenStatus == HALDRVSTATE_RUNNING);
		if( pdKey->ucGenStatus == HALDRVSTATE_ENDED )
		{
			if( pdKey->aucData[18] == KEY_CLEAR )
				return(OSERR_SUCCESS);
		}
		if(!memcmp(&pdKey->aucData[2],OSMEM_SRAMPASS,6))
			break;
	}	
    OSMMI_ClrDisplay(ASCIIFONT57,255);
    memset(aucSize,0,sizeof(aucSize));
    OSMMI_DisplayASCII(ASCIIFONT57,OSDISP_MSG0ROW,0,OSMSG_InputSRAMSize);
	while(1)
	{    
	    ucStatus = OSUTIL_Input(0,ASCIIFONT57,1,5,6,'H',aucSize,0);
	    if( ucStatus != OSERR_SUCCESS )
	        return(ucStatus);
	   	memset(aucHexSize,0,sizeof(aucHexSize));
	    CONV_StrHex(aucHexSize,5,aucSize);
        ulSize = CONV_HexLong(aucHexSize,5);
 		if( !(ulSize%0x80000))
 			break;
 		else
 			OSMMI_DisplayASCII(ASCIIFONT57,7,0,OSERRMSG_SRAMSIZE);		
	}        
    if( pucSize )
        memcpy(pucSize,aucSize,10);
#endif
    return(OSERR_SUCCESS);
}

void *OSMEM_Memcpy(void *pvDest,const void *pvSrc,unsigned int uiLen)
{
    return(memcpy(pvDest,pvSrc,uiLen));
}

int OSMEM_Memcmp(const void *pvDest, const void *pvSrc,unsigned int uiLen)
{
    return(memcmp(pvDest,pvSrc,uiLen));
}

void *OSMEM_Memset(void *pvDest,int iValue,unsigned int uiLen)
{
    return(memset(pvDest,iValue,uiLen));
}

char *OSMEM_Strcpy(char *pcDest,const char *pcSrc)
{
    return(strcpy(pcDest,pcSrc));
}

char *OSMEM_Strncpy(char *pcDest,const char *pcSrc,unsigned int uiLen)
{
    return(strncpy(pcDest,pcSrc,uiLen));
}

int OSMEM_Strcmp(const char *pcDest,const char *pcSrc)
{
    return(strcmp(pcDest,pcSrc));
}

int OSMEM_Strncmp(const char *pcDest,const char *pcSrc,unsigned int uiLen)
{
    return(strncmp(pcDest,pcSrc,uiLen));
}

char *OSMEM_Strcat(char * pcDest,const char *pcSrc)
{
    return(strcat(pcDest,pcSrc));
}

char *OSMEM_Strncat(char * pcDest,const char *pcSrc,unsigned int uiLen)
{
    return(strncat(pcDest,pcSrc,uiLen));
}

unsigned int OSMEM_Strlen(const char * pcPtr)
{
    return(strlen(pcPtr));
}


