/*
 icc Ӧ�ò�ӿڣ�come from apiing.c
 
*/
#include <stdio.h>
#include "haldrv.h"


/* Smart cards and SIMs reader-writer */
unsigned char Os__ICC_detect(unsigned char ucReader)
{
    return(OSICC_Detect(ucReader));
}

ICC_ANSWER *Os__ICC_command(unsigned char ucReader, ICC_ORDER *pOrder)
{
    return(OSICC_Command(ucReader,pOrder));
}

DRV_OUT *Os__ICC_insert(void)
{
    return(OSICC_Insert());
}

DRV_OUT *Os__ICC_Async_order(unsigned char *pucOrder,unsigned short uiLen)
{
    return(OSICC_OrderAsync(pucOrder,uiLen));
}

DRV_OUT *Os__ICC_order(DRV_IN  *pDrvIn)
{
    return(OSICC_Order(pDrvIn));
}

void Os__ICC_remove(void)
{
    OSICC_Remove();
}

void Os__ICC_off(void)
{
    OSICC_PowerOff();
}

extern void OSICC_Init(void);
extern unsigned char Os__ICC_detect(unsigned char ucReader);
int main()
{
	OSICC_Init();

	while(1)
	Os__ICC_detect(0);
}

//end
