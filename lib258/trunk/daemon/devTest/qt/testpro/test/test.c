#include "gsm.h"
#include "test.h"

#define DBG

void _print11(char *fmt,...)
{
#ifdef DBG
    printf(fmt);
    return ;
#else
    return ;
#endif
}


unsigned char SMART_NewReset(
        unsigned char ucReader,unsigned char ucCardStandard,
        unsigned char * pucReset,
        unsigned short * puiLength)
{
    unsigned char ucCardType;
    ICC_ORDER IccOrder;
    ICC_ANSWER* pAnswer;
    union ORDER_IN_TYPE OrderType;

    /* Power On */
    IccOrder.pt_order_in = &OrderType;
    IccOrder.ptout = pucReset;
    IccOrder.order = NEW_POWER_ON;
    IccOrder.pt_order_in->new_power.card_standards = ucCardStandard;
    IccOrder.pt_order_in->new_power.GR_class_byte_00 = 1;
    IccOrder.pt_order_in->new_power.preferred_protocol = 0xFF;

    /* Send Command */
    pAnswer = Os__ICC_command(ucReader, &IccOrder);

    /* Answer Command */
    if ( pAnswer->drv_status != OK )
    {
        _print11("SMART_NewReset return for drv_status != OK \r\n");
        return (SMART_DRIVER_ERROR);
    }

    switch ( pAnswer->card_status )
    {
    case ICC_ASY:
        ucCardType = SMART_ASYNC;
        break;
    case ICC_SYN:
        ucCardType = SMART_SYNC;
        break;
    default:
        _print11("SMART_NewReset return for SMART_CARD_ERROR\r\n");
        return (SMART_CARD_ERROR);
    }

    if ( pAnswer->Len > *puiLength )
    {
        _print11("SMART_NewReset return for SMART_OWERFLOW\r\n");
        return (SMART_OWERFLOW);
    }


    *puiLength = pAnswer->Len;


    return (ucCardType);
}




unsigned char OSGSM_Recv_dep(unsigned char *ucRcv,unsigned char *ucBuf)
{
    unsigned char *ret;

    ret=strstr(ucRcv,ucBuf);
    if(ret)
        return SUCCESS;
    else
        return FAILED;
}

int OSGSM_depRecv(unsigned char *ucRcv)
{
    unsigned char str[200];
    memset(str,0,sizeof(str));

    GSM_AT_cmd("",str,sizeof(str),3);
    if(strstr(str,ucRcv) != NULL)
        return 0;
    else
        return -1;
}

unsigned char OSGSM_answerCalling(void)
{
      char 	cmd[200];
      char 	rsp[200];
      unsigned char 	*pucPtr;
      int ret = 0;


      ret = OSGSM_depRecv("RING");
      _print11("ret==%02x \n",ret);

      _print11("ret==%d \n",ret);
      if(ret == 0)
      {
          memset(cmd, 0x00, sizeof(cmd));
          memset(rsp, 0x00, sizeof(rsp));
          _print11("ATA\n");

          strcpy(cmd, "ATA\r");
          int rec = GSM_AT_cmd(cmd,rsp,sizeof(rsp),1);
          if(rec) return FAILED;

          pucPtr = (unsigned char *)strstr( rsp, "OK");
          _print11("[%s]\n",rsp);

          if(pucPtr)
              return	SUCCESS;
      }


}

//?????ATH,????
unsigned char OSGSM_hangUp(void)
{
    char 	cmd[200];
    char 	rsp[200];
    unsigned char 	*pucPtr;

    memset(cmd, 0x00, sizeof(cmd));
    strcpy(cmd, "ATH");

    memset(rsp, 0x00, sizeof(rsp));
    int rec1 = GSM_AT_cmd(cmd,rsp,sizeof(rsp),1);
    if(rec1) return;

    pucPtr = (unsigned char *)strstr( rsp, "OK");
        if( pucPtr ){
        return	SUCCESS;
    }
    return FAILED;
}

int OSGSM_signalDetect(void)
{
    _print11("IN OSGSM_signalDetect\n");

    char cmd[200]="AT+CSQ\r";
    char rsp[200];
    int signal_value=0;
    int rlen,l;
    char *ptrb;
    char *ptre;
    char ucResult;

    _print11("11111 IN OSGSM_signalDetect\n");

    _print11("222222222 IN OSGSM_signalDetect\n");

    memset(rsp,0,sizeof(rsp));
    _print11("%s\n",rsp);
    int rec1 = GSM_AT_cmd("AT",rsp,sizeof(rsp),1);
    if(rec1) return;

    _print11("33333333 IN OSGSM_signalDetect\n");


    memset(rsp, 0, sizeof(rsp));
   rec1 = GSM_AT_cmd(cmd,rsp,sizeof(rsp),1);
   if(rec1) return;

    _print11("44444444 IN OSGSM_signalDetect\n");

    _print11("-->%s\n",rsp);
    rlen=strlen(rsp);
    _print11("555555 IN OSGSM_signalDetect\n");


    if(rlen>0)
    {
        _print11("66666666 IN OSGSM_signalDetect\n");

        ptrb=strpbrk(rsp,":");
        if(ptrb)
        {
            ptre=strpbrk(rsp,",");
            if(ptre)
            {
                l=ptre-ptrb;
                if(l>0)
                {
                    ptrb+=2;
                    signal_value=asc_long(ptrb,l-2);
                    if(signal_value>31)
                        signal_value=99;
                }
            }
        }

    }
    _print11("777777 IN OSGSM_signalDetect\n");

    return signal_value;
}





unsigned char SLE4442_FinishCommand(unsigned char *pucBuf)
{
    pucBuf[0] = EOC_CODE;	//0x00

    return (1);
}


unsigned char SLE4442_ResetAddress(unsigned char *pucBuf)
{
    pucBuf[0] = CLR_CLK_CODE;  	//0x08
    pucBuf[1] = CLR_RST_CODE;	//0x02
    pucBuf[2] = SET_IO_CODE;		//0x22
    pucBuf[3] = SET_RST_CODE;	//0x04
    pucBuf[4] = CLK_5US_CODE;	//0x0c
    pucBuf[5] = CLR_RST_CODE;	//0x02
    pucBuf[6] = IO_TO_LSB_CODE;	//0x2a

    return(7);
}

unsigned char SLE4442_NReadBits(unsigned char *pucBuf, unsigned char ucbitscnt)
{
    pucBuf[0] = MACRO_CODE;		//0xff
    pucBuf[1] = CLK_5US_CODE;	//0x0c
    pucBuf[2] = IO_TO_LSB_CODE;	//0x2a
    pucBuf[3] = MACRO_CODE;		//0xff
    pucBuf[4] = ucbitscnt;			//31
    return(5);
}

unsigned char SLE4442_End_ATR(unsigned char *pucBuf)
{
    pucBuf[0] = CLK_5US_CODE;	//0x0c

    return(1);
}

unsigned char SLE4442_ATR(unsigned char *pucBuf)
{
    unsigned char aucSle4442[256];
    unsigned char ucIndex;

    ICC_ORDER 	SLE4442_order;
    ICC_ANSWER *pSLE4442_answer;
    union ORDER_IN_TYPE SLE4442_type;

    ucIndex = 0;
    ucIndex += SLE4442_ResetAddress(aucSle4442 + ucIndex);	//return 7
    ucIndex += SLE4442_NReadBits(aucSle4442 + ucIndex, 31);	//return 5
    ucIndex += SLE4442_End_ATR(aucSle4442 + ucIndex);		//return 1
    ucIndex += SLE4442_FinishCommand(aucSle4442 + ucIndex);	//return 1

    if (ucIndex)		//ucIndex=14
    {
        SLE4442_type.new_sync_order.Len = ucIndex;
        SLE4442_type.new_sync_order.ptin = aucSle4442;
        SLE4442_order.pt_order_in = &SLE4442_type;
        SLE4442_order.order = NEW_SYNC_ORDER;
        SLE4442_order.ptout = aucSle4442;
        pSLE4442_answer = Os__ICC_command(0,&SLE4442_order);
        if (pSLE4442_answer->drv_status != OK )
        {
            _print11("SLE4442 DRV_STATUS ERROR\n");
            return 1;
        }
        if( pSLE4442_answer->card_status != ASY_OK )
        {
            _print11("SLE4442 CARD_STATUS ERROR\n");
            return 1;
        }
        memcpy(pucBuf,aucSle4442,pSLE4442_answer->Len);
    }

    return(OK);
}


unsigned char SLE4442_ReadMainMemory(unsigned char *pucBuf,
                                                                             unsigned char ucaddr,
                                                                             unsigned char ucsize)
{
    unsigned char aucSle4442[256];
    unsigned char ucIndex;

    ICC_ORDER 	SLE4442_order;
    ICC_ANSWER *pSLE4442_answer;
    union ORDER_IN_TYPE SLE4442_type;

    ucIndex = 0;

    if (ucsize == 0)
    {
        return 0;
    }
    if (ucaddr + ucsize > 255)
    {
        return 0;
    }

    ucIndex += SLE4442_Enter_Command_Mode(aucSle4442 + ucIndex);//return 5
    ucIndex += SLE4442_Send_Command(aucSle4442 + ucIndex, 0x30, ucaddr, 0);//rturn 48
    ucIndex += SLE4442_Leave_Command_Mode(aucSle4442 + ucIndex);//return 2
    ucIndex += SLE4442_NReadBits(aucSle4442 + ucIndex, ucsize * 8);//return 5
    ucIndex += SLE4442_Break(aucSle4442 + ucIndex);
    ucIndex += SLE4442_FinishCommand(aucSle4442 + ucIndex);//return 1

    if (ucIndex)
    {
        SLE4442_type.new_sync_order.Len = ucIndex;
        SLE4442_type.new_sync_order.ptin = aucSle4442;
        SLE4442_order.pt_order_in = &SLE4442_type;
        SLE4442_order.order = NEW_SYNC_ORDER;
        SLE4442_order.ptout = aucSle4442;
        pSLE4442_answer = Os__ICC_command(0,&SLE4442_order);

        _print11(" SLE4442_answer->drv_status = %d\n",  pSLE4442_answer->drv_status);

        _print11(" SLE4442_answer->card_status = %d\n",  pSLE4442_answer->card_status);


        if( pSLE4442_answer->drv_status != OK )
        {
            //return (ERR_READMEMORY);
            _print11("SLE4442 DRV_STATUS ERROR\n");
            return 1;
        }
        if( pSLE4442_answer->card_status != ASY_OK )
        {
            //return (ERR_READMEMORY);
            _print11("SLE4442 CARD_STATUS ERROR\n");
            return 1;
        }
        memcpy(pucBuf, aucSle4442,pSLE4442_answer->Len);
    }

    return(OK);
}

/************************** printer **************************************/



unsigned long asc_long(unsigned char *Pts, unsigned char Ls)
{
    unsigned char I;
    unsigned long Lg1,
    Lg2;

    Lg1 = 0 ;
    Lg2 = 1 ;
    Pts += Ls ;
    for (I = 0; I< Ls ; I++)
    {
        Lg1 += (Lg2 * (*--Pts & 0x0F) ) ;
        Lg2 *= 10 ;
    }

    return (Lg1) ;
}


/******************  wifi *********************************/

