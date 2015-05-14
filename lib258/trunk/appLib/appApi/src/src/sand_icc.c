/*
 icc 应用层接口，come from apiing.c

*/
#include <stdio.h>
#include <haldrv.h>
#include <osicc.h>
#include <osdriver.h>
#include "osmifare.h"
#include <string.h>
#include "oslib.h"
#include <unistd.h>


/* Smart cards and SIMs reader-writer */
/* OK 0 , has a card, 1 not*/
unsigned char Os__ICC_detect(unsigned char ucReader)
{
    return(OSICC_Detect(ucReader));
}

ICC_ANSWER *Os__ICC_command(unsigned char ucReader, ICC_ORDER *pOrder)
{
    return(OSICC_Command(ucReader,pOrder));
}

DRV_OUT *Os__ICC_Async_order(unsigned char *pucOrder,unsigned short uiLen)
{
    return((DRV_OUT *)OSICC_OrderAsync(pucOrder,uiLen));
}

DRV_OUT *Os__ICC_order(DRV_IN  *pDrvIn)
{
    return((DRV_OUT *)OSICC_Order((DRVIN *)pDrvIn));
}

void Os__ICC_remove(void)
{
    OSICC_Remove();
}

void Os__ICC_off(void)
{
    OSICC_PowerOff();
}


