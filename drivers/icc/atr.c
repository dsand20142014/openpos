/*
	Library Function for ICC
--------------------------------------------------------------------------
	FILE  atr.c
--------------------------------------------------------------------------
    INTRODUCTION
    ============
    Created :		2003-02-22		Xiaoxi Jiang
    Last modified :	2003-02-22		Xiaoxi Jiang
    Module :
    Purpose :
        Source file.

    List of routines in file :

    File history :
*/

#include "halicc.h"
#include  "libicc.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/completion.h>
//#include <tools.h>
extern int icc_dbg;
extern int gxd;
//extern struct completion comp;
//extern struct completion comp1;
extern struct completion comp;

extern unsigned char QINGDAO_WarmReset(unsigned char ucReader);
extern unsigned char QINGDAO_AsyncReset(unsigned char ucReader);
extern void QINGDAO_RecvCharProc(unsigned char ucReader,unsigned char ucCh);
extern void QINGD_TimeoutProc(unsigned char ucReader,unsigned char ucTimeoutType);
extern unsigned char QINGDAO_CheckICCParam(unsigned char ucReader);

extern unsigned char PROT_AsyncCommand(unsigned char ucReader,
        unsigned char *pucInData,
        unsigned short uiInLen);

extern unsigned char pollpw_switch ;

/*zyadd from libmisc.c*/
unsigned char MISC_GetCharBitNB(unsigned char ucCh,unsigned char ucBit)
{
        unsigned char ucMask;
        unsigned char ucNB;

        ucMask = 0x80;
        ucNB = 0;
        if ( !ucBit ) {
                ucCh = ~ucCh;
        }
        do {
                if ( ucCh & ucMask ) {
                        ucNB ++;
                }
                ucMask = ucMask >> 1;
        } while ( ucMask );
        return(ucNB);
}


unsigned char ATR_PowerOn(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
	//init_completion(&comp);
        if (icc_dbg>=1)
                printk("%s stander=%d reader=%d\n", __func__, pCurrICCInfo->ucStandard, ucReader);
	pollpw_switch = 0;

        switch ( pCurrICCInfo->ucStandard) {
        case ICCSTANDARD_ISO7816:
                return(ISO7816_AsyncReset(ucReader));
        case ICCSTANDARD_EMV:
                return(EMV_AsyncReset(ucReader));
        case ICCSTANDARD_GJISAM:
                return(GJISAM_AsyncReset(ucReader));
        case ICCSTANDARD_GJPSAM:
                return(GJPSAM_AsyncReset(ucReader));
        case ICCSTANDARD_SHEBAO:
                return(SHEBAO_AsyncReset(ucReader));
        case ICCSTANDARD_QINGD:
           	return(QINGDAO_AsyncReset(ucReader));
        default:
                LIBICC_CODENOTIMPLEMENT
                return(LIBICCERR_UNKNOWNPROTOCOL);
        }
}

unsigned char ATR_WarmReset(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;
	pollpw_switch = 0;

        pCurrICCInfo = &ICCInfo[ucReader];
        switch ( pCurrICCInfo->ucStandard) {
        case ICCSTANDARD_ISO7816:
                return(ISO7816_WarmReset(ucReader));
        case ICCSTANDARD_EMV:
                return(EMV_WarmReset(ucReader));
        case ICCSTANDARD_GJISAM:
                return(GJISAM_WarmReset(ucReader));
        case ICCSTANDARD_GJPSAM:
                return(GJPSAM_WarmReset(ucReader));
        case ICCSTANDARD_SHEBAO:
                return(SHEBAO_WarmReset(ucReader));
        case ICCSTANDARD_QINGD:
            	return(QINGDAO_WarmReset(ucReader));
        default:
                LIBICC_CODENOTIMPLEMENT
                return(LIBICCERR_UNKNOWNPROTOCOL);
        }
}
extern int icc_deactiveflag;
#if 0
unsigned char ATR_CheckFinished(unsigned char *pucATR,unsigned short ucATRLen)
{
        unsigned short uiI;
        unsigned short uiATRTotalLen;
        unsigned char ucTDn;
        unsigned char ucTDIndex;
        unsigned char ucNextTDnExist;
        unsigned char ucTCharNB;

        uiI = 0;
        ucTDIndex = 0;

        if ( ucATRLen >= 2 ) {
                uiI = 1;
                ucTDn = *(pucATR+uiI);
                uiATRTotalLen = (ucTDn&0x0F)+2;
                do {
                        ucTDn = *(pucATR+uiI);
                        if ( ucTDIndex == 1 ) {
                                /*EN: TCK Exist */
                                if (  ( ucTDn & 0x0F )
                                                ||( ucTDn & 0x80 )
                                   ) {
                                        uiATRTotalLen ++;
                                }
                        }
                        ucNextTDnExist = ucTDn & 0x80;
                        ucTCharNB = MISC_GetCharBitNB(ucTDn&0xF0,1);
                        uiATRTotalLen += ucTCharNB;
                        if ( uiATRTotalLen > ucATRLen) {
                                if ( uiATRTotalLen == ucATRLen+1 ) {
                                        return(LIBICCERR_ATRHASLASTCHAR);
                                } else {
                                        return(LIBICCERR_ATRNOTFINISHED);
                                }
                        } else {
                                if (  (uiATRTotalLen == ucATRLen )
                                                &&( !ucNextTDnExist )
                                   ) {
                                        return(LIBICCERR_SUCCESS);
                                } else {
                                        if ( ucNextTDnExist ) {
                                                uiI += ucTCharNB;
                                                ucTDIndex ++;
                                        } else {
                                                if ( uiATRTotalLen > ucATRLen ) {
                                                        return(LIBICCERR_ATRTOOLONG);
                                                }
                                        }
                                }
                        }
		if ( icc_deactiveflag ) break;
                } while (1);
        }
        return(LIBICCERR_ATRNOTFINISHED);
}
#else

unsigned char ATR_CheckFinished(unsigned char *pucATR,unsigned short ucATRLen)
{
        unsigned short uiI;
        unsigned short uiATRTotalLen;
        unsigned char ucTDn;
        unsigned char ucTDIndex;
        unsigned char ucNextTDnExist;
        uiI = 0;
        ucTDIndex = 0;
		
        if ( ucATRLen >= 2 ) {
                uiI = 1;
                ucTDn = *(pucATR+uiI);
                uiATRTotalLen = (ucTDn&0x0F)+2;
                do {
                 ucNextTDnExist = 0 ;
                        ucTDn = *(pucATR+uiI);
                        if ( ucTDIndex == 1 ) {
                                /*EN: TCK Exist */
                                if (  ( ucTDn & 0x0F )||( ucTDn & 0x80 )
                                   ) {
                                        uiATRTotalLen ++;
                                }
                        }
                        if ( ucTDn&0x10 )
                        {
                         uiATRTotalLen++;
                         uiI++;
                        }
                        if ( ucTDn&0x20 )
                        {
                         uiATRTotalLen++;
                         uiI++;
                        }
                        if ( ucTDn&0x40 )
                        {
                         uiATRTotalLen++;
                         uiI++;
                        }
                        if ( ucTDn&0x80 )
                        {
                         uiATRTotalLen++;
                         uiI++;
				ucTDIndex ++;
				ucNextTDnExist = 1;
                        }
                        if ( uiATRTotalLen > ucATRLen  )
                        {
                                if ( uiATRTotalLen == ucATRLen+1 )   
					return(LIBICCERR_ATRHASLASTCHAR);
				else  
					return(LIBICCERR_ATRNOTFINISHED);
			 } else {
					if ( !ucNextTDnExist )
					{
						if ( uiATRTotalLen == ucATRLen )
							return(LIBICCERR_SUCCESS);
						else
							return(LIBICCERR_ATRTOOLONG);
					}
												}


				} while ((ucNextTDnExist)&&(ucTDIndex<5));
        }
        return(LIBICCERR_ATRNOTFINISHED);
}

#endif
unsigned char ATR_Process(unsigned char ucReader)
{
        unsigned char ucResult;
        ICCINFO *pCurrICCInfo;
        unsigned char *pucATR;
        unsigned short uiI;
        unsigned char ucI;
        unsigned char ucNB;
        unsigned char ucTDn;
        unsigned char ucTCharNB;
        unsigned char ucNextTDnExist;
        unsigned char ucMask;
        unsigned char ucCh;
        unsigned char ucT0;
        unsigned char ucTA1;
        unsigned char ucTA2;
        unsigned char ucTC1;
        unsigned char ucTC2;
        unsigned char ucTC3;
        unsigned char ucTB3;
        unsigned char ucTA3;
        unsigned char ucTD1;
	unsigned char ucTD2;
        unsigned char ucTB1Exist;
        unsigned char ucTD1Exist;
        unsigned char ucTA2Exist;
        unsigned char ucProtocol;

        ucResult = LIBICCERR_SUCCESS;
        pCurrICCInfo = &ICCInfo[ucReader];
        pucATR = pCurrICCInfo->aucATR;
        ucTC1 = 0x00;
        ucTA1 = 0x11;
        ucTC2 = 10;
        ucTC3 = 0x00;
        ucTB3 = 0xFF;
        ucTA3 = 0x20;
        ucTB1Exist = 0;
        ucTD1Exist = 0;
        ucTA2Exist = 0;
        ucProtocol = 0;

#if 0
        if ( pCurrICCInfo->ucResetType == LIBICC_COLDRESET ) {
                memcpy(pCurrICCInfo->aucATR,"\x3B\x60\x05\x00",4);
                pCurrICCInfo->ucATRLen = 4;
        }
        if ( pCurrICCInfo->ucResetType == LIBICC_WARMRESET ) {
                memcpy(pCurrICCInfo->aucATR,"\x3B\xF0\x11\x00\x00\x10\x80",7);
                pCurrICCInfo->ucATRLen = 7;
        }
#endif

        if ( pCurrICCInfo->ucATRLen > 34 ) {
                return(LIBICCERR_ATRTOOLONG);
        }
        if (  (*pucATR != 0x3B )
                        &&(*pucATR != 0x3F )
           )
                ucResult = LIBICCERR_BADTS;

     //emv test Level1 20110329
#if 0
      if((!ucResult)&&(*pucATR==0x3B)&&(pCurrICCInfo->ucATRLen>=3))
      {
    	  ucT0=*(pucATR+1);
    	  switch(ucT0&0xF0)
    	  {
    	  case 0x20:
    		  ucResult = LIBICCERR_BADT0_20;
    		  break;
    	  case 0x40:
    		  ucResult = LIBICCERR_BADT0_40;
    		  break;
    	  default:
    		  break;
    	  }

      }
#endif

        if (  (!ucResult)
                        &&(pCurrICCInfo->ucATRLen >= 2 )
           ) {
                uiI = 1;
                for (ucI=0; ucI<4; ucI++) {
                        ucTDn = *(pucATR+uiI);
                        ucNextTDnExist = ucTDn & 0x80;
                        ucTCharNB = MISC_GetCharBitNB(ucTDn&0xF0,1);
                        if ( !ucTCharNB )
                                break;
                        ucMask = 0x10;
                        ucNB = 0;
                        do {
                                if ( ucTDn & ucMask ) {
                                        uiI++;
                                        ucCh = *(pucATR+uiI);
                                        switch (ucNB) {
                                        case 0:
                                                switch (ucI) {
                                                case 0:
                                                        ucTA1 = ucCh;
                                                        break;
                                                case 1:
                                                        ucTA2 = ucCh;
                                                        ucTA2Exist = 1;
                                                        break;
                                                case 2:
                                                        if (  (ucCh == 0xFF)
                                                                        ||(ucCh <= 0x0F)
                                                           ) {
                                                                ucResult = LIBICCERR_BADTA3;
                                                                break;
                                                        } else {
                                                                ucTA3 = ucCh;
                                                        }
                                                        break;
                                                default:
                                                        break;
                                                }
                                                break;
                                        case 1:
                                                switch (ucI) {
                                                case 0:
                                                        ucTB1Exist = 1;
                                                        if (  (pCurrICCInfo->ucResetType == LIBICC_COLDRESET )
                                                                        &&(ucCh)
                                                           ) {
                                                                ucResult = LIBICCERR_BADTB1;
                                                                break;
                                                        }
                                                        break;
                                                case 1:
                                                        ucResult = LIBICCERR_BADTB2;
                                                        break;
                                                case 2:
                                                        ucTB3 = ucCh;
                                                        break;
                                                default:
                                                        break;
                                                }
                                                break;
                                        case 2:
                                                switch ( ucI ) {
                                                case 0:
                                                        ucTC1 = ucCh;
                                                        break;
                                                case 1:
                                                       if (  (ucCh == 0x00 )
                                                                        &&(ucCh != 0x0A )
                                                                        &&(ucCh != 0x01 )
                                                                        &&(ucCh != 0x14 )
                                                           )
                                                //	if (ucCh != 0x0A&&ucCh!=0x09)
                                                				{
                                                                ucResult = LIBICCERR_BADTC2;
                                                                break;
                                                        }
                                                    //    if ( ucTC2!=0 )
                                                                ucTC2 = ucCh;
                                                        break;
                                                case 2:
                                                        if ( ucCh != 0x00) {
                                                                ucResult = LIBICCERR_BADTC3;
                                                        } else {
                                                                ucTC3 = ucCh;
                                                        }
                                                        break;
                                                default:
                                                        break;
                                                }
                                                break;
                                        case 3:
                                                switch (ucI) {
                                                case 0:
                                                        ucTD1Exist = 1;
                                                        ucTD1 = ucCh;
                                                        if (  ( (ucCh&0x0F) == 0x00)
                                                                        ||( (ucCh&0x0F) == 0x01)
                                                           ) {
                                                                ucProtocol = ucCh & 0x0F;
                                                        } else {
                                                                ucResult = LIBICCERR_BADTD1;
                                                                break;
                                                        }
                                                        break;
                                                case 1:
                                                        if ( (ucCh&0x0F) != 0x01) {
                                                          //      if ( (ucCh&0x0F) == 0x0F )
                                                        	if ( (ucCh&0x0F) == 0x0E )
                                                        		{
                                                                        if (  (ucTD1Exist)
                                                                              &&( (ucTD1&0x0F) == 0x00)
                                                                           ) {
                                                                                ucTD2=ucCh;
                                                                                break;
                                                                        } else {
                                                                                ucResult = LIBICCERR_BADTD2;
                                                                        }
                                                                } else {
                                                                        ucResult = LIBICCERR_BADTD2;
                                                                }
                                                        }
                                                        break;
                                                default:
                                                        break;
                                                }
                                                break;
                                        default:
                                                break;
                                        }
                                }
                                ucMask = ucMask << 1;
                                ucNB ++;

                                if ( ucResult )
                                        break;
                        } while ( ucMask );
                        if ( ucResult )
                                break;
                        if ( !ucNextTDnExist )
                                break;
                }
        }
        if ( !ucResult ) {
                if (  (!ucTB1Exist )
                                &&(pCurrICCInfo->ucResetType == LIBICC_COLDRESET)
                   )
            ucResult = LIBICCERR_BADTB1;
        }
 //20110330
   if(!ucResult)
   {
	    ucT0 = *(pucATR+1);

	    uiI += ucT0 & 0x0F;
	    //gxd
	  //  printk("[%s,%d], uiI:0x%x pCurrICCInfo->ucATRLen:0x%x\n",__func__,__LINE__, uiI, pCurrICCInfo->ucATRLen);
	    if(uiI==pCurrICCInfo->ucATRLen-2)
	/*
	    EN: Check TCK 
	    if(  ( !ucResult )
	       &&( ucTD1Exist )
	       &&(  ( ucTD1 & 0x0F )
	          ||( ucTD1 & 0x80 )
	         )
	      )
	      */
	    {
	        ucCh = 0x00;
	        for(ucI=1;ucI<pCurrICCInfo->ucATRLen;ucI++)
	        {
	            ucCh = ucCh ^ pCurrICCInfo->aucATR[ucI];
	        }
	        if( ucCh )
		    if(ucProtocol)
	            	ucResult = LIBICCERR_BADTCK;
		    else
			ucResult =LIBICCERR_BADT0_20;
	    }
	    else if(uiI<pCurrICCInfo->ucATRLen-2)
			ucResult = LIBICCERR_UNKNOWNPROTOCOL;
   	}

        if ( !ucResult ) {
                switch ( ucTA1&0x0F ) {
                case 1:
                        pCurrICCInfo->ucDi = 1;
                        break;
                case 2:
                        pCurrICCInfo->ucDi = 2;
                        break;
                case 3:
                        pCurrICCInfo->ucDi = 4;
                        break;
                case 4:
                        pCurrICCInfo->ucDi = 4;
                        break;
                case 8:
                        pCurrICCInfo->ucDi = 8;
                        break;
                default:
                	printk("LIBICCERR_BADTA1 439");
                        ucResult = LIBICCERR_BADTA1;
                        break;
                }
                if ( !ucResult ) {
                        if ( ((ucTA1&0xF0) != 0x10)
                                        &&((ucTA1&0xF0) != 0x90) )
                	//if  ((ucTA1&0xF0) != 0x10)
                	// && ((ucTA1&0xF0) != 0x90)
                        {
                        	printk("LIBICCERR_BADTA1 447");
                                ucResult = LIBICCERR_BADTA1;

                        }
                }
        }

        if ( !ucResult ) {
                if ( !ucTA2Exist ) {
                        pCurrICCInfo->ucDi = 1;
                } else {
                        if ( ucTA2&0x10 ) {
                                ucResult = LIBICCERR_BADTA2;
                        }
                }
        }
        if ( !ucResult ) {
                pCurrICCInfo->ucProtocol = ucProtocol;
#if 0
                if ( ucTC1 == 255 ) {
                        pCurrICCInfo->ucGTR = 0;
                } else {
                        pCurrICCInfo->ucGTR = ucTC1;
                }
#else
                pCurrICCInfo->ucGTR = ucTC1;
#endif
                pCurrICCInfo->ucFi = (ucTA1&0xF0)>>4;
                if ( ucTC2 ) {
                        pCurrICCInfo->ulWWT = 960*ucTC2*(pCurrICCInfo->ucDi)+(pCurrICCInfo->ucDi*480);
                        pCurrICCInfo->ulWWT += 1;
                } else {
                        ucResult = LIBICCERR_BADTC2;
                }
        }
        if ( !ucResult ) {
                if ( ucProtocol ) {
                        ucCh = ucTB3&0x0F;
                        if ( ucCh > 5 ) {
                                ucResult = LIBICCERR_BADTB3;
                        }
                        if ( !ucResult ) {
                                ucCh = (ucTB3&0xF0) >> 4;
                                if ( ucCh > 4 ) {
                                        ucResult = LIBICCERR_BADTB3;
                                }
                        }
                        if ( !ucResult ) {
                                if (  (ucTC1 == 0x00 )
                                                &&(ucTB3 == 0x00 )
                                   )
                                        ucResult = LIBICCERR_BADTB3;
                                else {
                                        if (  ( ucTC1 > 0x1E )
                                                        &&( ucTC1 != 0xFF)
                                           ) {
                                                ucResult = LIBICCERR_BADTB3;
                                        } else {
                                                ucCh = 1;
                                                for (uiI=0; uiI<(ucTB3&0x0F); uiI++) {
                                                        ucCh *= 2;
                                                }
                                                ucCh += 11;
                                                if ( ucTC1 == 0xFF ) {
                                                        if ( ucCh <= 11 ) {
                                                                ucResult = LIBICCERR_BADTB3;
                                                        }
                                                } else {
                                                        if ( ucCh <= (ucTC1+1) )
                                                                ucResult = LIBICCERR_BADTB3;
                                                }
                                        }
                                }
                        }
                }
        }
        if ( !ucResult ) {
                switch (pCurrICCInfo->ucStandard) {
                case ICCSTANDARD_ISO7816:
                        ucResult = ISO7816_CheckICCParam(ucReader);
                        break;
                case ICCSTANDARD_EMV:
                        ucResult = EMV_CheckICCParam(ucReader);
                        break;
                case ICCSTANDARD_GJISAM:
                        ucResult = GJISAM_CheckICCParam(ucReader);
                        break;
                case ICCSTANDARD_GJPSAM:
                        ucResult = GJPSAM_CheckICCParam(ucReader);
                        break;
                case ICCSTANDARD_SHEBAO:
                        ucResult = SHEBAO_CheckICCParam(ucReader);
                        break;
                default:
                        LIBICC_CODENOTIMPLEMENT
                        break;
                }
        }
        /*EN: Set TDA8007 register*/
        if ( !ucResult ) {
                ucResult = HALICC_SetProtocol(ucReader,ucProtocol);
        }
        if ( !ucResult ) {
                ucResult = HALICC_SetGTR(ucReader,pCurrICCInfo->ucGTR);
        }
        if ( !ucResult ) {
                switch (pCurrICCInfo->ucStandard) {
                case ICCSTANDARD_ISO7816:
                case ICCSTANDARD_EMV:
                case ICCSTANDARD_GJISAM:
                case ICCSTANDARD_SHEBAO:
                        ucResult = HALICC_SetDi(ucReader,pCurrICCInfo->ucDi);
                        break;
                case ICCSTANDARD_GJPSAM:
                        break;
                default:
                        LIBICC_CODENOTIMPLEMENT
                        break;
                }
        }
        if (  (!ucResult)
                        &&( ucProtocol )
           ) {
                pCurrICCInfo->ucT1Status = 0;
                pCurrICCInfo->ucT1ErrorCount = 0;
                pCurrICCInfo->ucT1IFSC = ucTA3;
                pCurrICCInfo->ucT1IFSReqFlag = 1;
                pCurrICCInfo->ucT1EDCMode = ucTC3&0x01;
                pCurrICCInfo->ucT1SendSeq = 0;
                pCurrICCInfo->ucT1RecvSeq = 0;
                pCurrICCInfo->uiCWT = 1;
                ucCh = ucTB3&0x0F;
                for (uiI=0; uiI<ucCh; uiI++) {
                        pCurrICCInfo->uiCWT *= 2;
                }
                pCurrICCInfo->uiCWT += 11;
                pCurrICCInfo->uiCWT += 4;
                pCurrICCInfo->ulBWT = 1;
                ucCh = (ucTB3&0xF0)>>4;
                for (uiI=0; uiI<ucCh; uiI++) {
                        pCurrICCInfo->ulBWT *= 2;
                }
                pCurrICCInfo->ulBWT *= 960;
                pCurrICCInfo->ulBWT *= pCurrICCInfo->ucDi;
                pCurrICCInfo->ulBWT += 11;
                pCurrICCInfo->ulBWT += pCurrICCInfo->ucDi*960;
                pCurrICCInfo->ucWTX = 1;
        }
        pCurrICCInfo->ucLastError = ucResult;

        /*TRACE*/
#if 0
        int ii=0;
        for(ii = 0; ii < pCurrICCInfo->ucATRLen; ii++)
        printk(" pCurrICCInfo->aucATR[%d]:0x%x\n", pCurrICCInfo->aucATR[ii]);


        unsigned char pucInData1[19] = {  0x00,0xA4,0x04,0x00,
              		                                             0x0e,0x31,0x50,0x41,
              		                                             0x59,0x2E,0x53,0x59,
              		                                             0x53,0x2E,0x44,0x44,
              		                                             0x46,0x30,0x31};

              PROT_AsyncCommand(0,pucInData1,sizeof(pucInData1) / sizeof(unsigned char));

#endif

        return(HALICCERR_SUCCESS);
}

unsigned char ATR_SpProcess(unsigned char ucReader)
{
    unsigned char ucResult;
    ICCINFO *pCurrICCInfo;
    unsigned char *pucATR;
    unsigned short uiI;
    unsigned char ucI;
    unsigned char ucNB;
    unsigned char ucTDn;
    unsigned char ucTCharNB;
    unsigned char ucNextTDnExist;
    unsigned char ucMask;
    unsigned char ucCh;
    unsigned char ucTA1;
    unsigned char ucTA2;
    unsigned char ucTC1;
    unsigned char ucTC2;
    unsigned char ucTC3;
    unsigned char ucTB3;
    unsigned char ucTA3;
    unsigned char ucTD1;
    unsigned char ucTB1Exist;
    unsigned char ucTD1Exist;
    unsigned char ucTA2Exist;
    unsigned char ucProtocol;

    ucResult = LIBICCERR_SUCCESS;
    pCurrICCInfo = &ICCInfo[ucReader];
    pucATR = pCurrICCInfo->aucATR;
    ucTC1 = 0x00;
    ucTA1 = 0x11;
    ucTC2 = 10;
    ucTC3 = 0x00;
    ucTB3 = 0xFF;
    ucTA3 = 0x20;
    ucTB1Exist = 0;
    ucTD1Exist = 0;
    ucTA2Exist = 0;
    ucProtocol = 0;

#if 0
    if( pCurrICCInfo->ucResetType == LIBICC_COLDRESET )
    {
        memcpy(pCurrICCInfo->aucATR,"\x3B\x60\x05\x00",4);
        pCurrICCInfo->ucATRLen = 4;
    }
    if( pCurrICCInfo->ucResetType == LIBICC_WARMRESET )
    {
        memcpy(pCurrICCInfo->aucATR,"\x3B\xF0\x11\x00\x00\x10\x80",7);
        pCurrICCInfo->ucATRLen = 7;
    }
#endif

    if( pCurrICCInfo->ucATRLen > 34 )
    {
        return(LIBICCERR_ATRTOOLONG);
    }
	if(  (*pucATR != 0x3B )
	   &&(*pucATR != 0x3F )
	  )
		ucResult = LIBICCERR_BADTS;

    if(  (!ucResult)
       &&(pCurrICCInfo->ucATRLen >= 2 )
      )
    {
        uiI = 1;
        for(ucI=0;ucI<3;ucI++)
        {
            ucTDn = *(pucATR+uiI);
            ucNextTDnExist = ucTDn & 0x80;
            ucTCharNB = MISC_GetCharBitNB(ucTDn&0xF0,1);
            if( !ucTCharNB )
                break;
            ucMask = 0x10;
            ucNB = 0;
            do{
                if( ucTDn & ucMask )
                {
                    uiI++;
                    ucCh = *(pucATR+uiI);
                    switch(ucNB)
                    {
                    case 0:
                        switch(ucI)
                        {
                        case 0:
                            ucTA1 = ucCh;
                            break;
                        case 1:
                            ucTA2 = ucCh;
                            ucTA2Exist = 1;
                            break;
                        case 2:
                            if(  (ucCh == 0xFF)
                               ||(ucCh <= 0x0F)
                              )
                            {
                                ucResult = LIBICCERR_BADTA3;
                                break;
                            }else
                            {
                                ucTA3 = ucCh;
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case 1:
                        switch(ucI)
                        {
                        case 0:
                            ucTB1Exist = 1;
                            if(  (pCurrICCInfo->ucResetType == LIBICC_COLDRESET )
                               &&(ucCh)
                              )
                            {
                                ucResult = LIBICCERR_BADTB1;
                                break;
                            }
                            break;
                        case 1:
                            ucResult = LIBICCERR_BADTB2;
                            break;
                        case 2:
                            ucTB3 = ucCh;
                            break;
                        default:
                            break;
                        }
                        break;
                    case 2:
                    	switch( ucI )
                    	{
                    	case 0:
                    		ucTC1 = ucCh;
                    		break;
                    	case 1:
                    	    if(  (ucCh == 0x00 )
                    	       &&(ucCh != 0x0A )
                    	       &&(ucCh != 0x01 )
                    	       &&(ucCh != 0x14 )
                    	      )
                    	    {
                    	        ucResult = LIBICCERR_BADTC2;
                    	        break;
                    	    }
                    	    if( ucTC2!=0 )
                    			ucTC2 = ucCh;
                    		break;
                    	case 2:
                    	    if( ucCh != 0x00)
                    	    {
                    	        ucResult = LIBICCERR_BADTC3;
                    	    }else
                    	    {
                    		    ucTC3 = ucCh;
                    		}
                    		break;
                    	default:
                    		break;
                    	}
                        break;
                    case 3:
                        switch(ucI)
                        {
                        case 0:
                            ucTD1Exist = 1;
                            ucTD1 = ucCh;
                            if(  ( (ucCh&0x0F) == 0x00)
                               ||( (ucCh&0x0F) == 0x01)
                              )
                            {
                                ucProtocol = ucCh & 0x0F;
                            }else
                            {
                                ucResult = LIBICCERR_BADTD1;
                                break;
                            }
                            break;
                        case 1:
                            if( (ucCh&0x0F) != 0x01)
                            {
                                if( (ucCh&0x0F) == 0x0F )
                                {
                                    if(  (ucTD1Exist)
                                       &&( (ucTD1&0x0F) == 0x00)
                                      )
                                    {
                                        break;
                                    }else
                                    {
                                        ucResult = LIBICCERR_BADTD2;
                                    }
                                }else
                                {
                                    ucResult = LIBICCERR_BADTD2;
                                }
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                }
                ucMask = ucMask << 1;
                ucNB ++;
                if( ucResult )
                    break;
            }while( ucMask );
            if( ucResult )
                break;
            if( !ucNextTDnExist )
                break;
        }
    }
    if( !ucResult )
    {
        if(  (!ucTB1Exist )
           &&(pCurrICCInfo->ucResetType == LIBICC_COLDRESET)
          );
//            ucResult = LIBICCERR_BADTB1;
    }

    /*EN: Check TCK */
    if(  ( !ucResult )
       &&( ucTD1Exist )
       &&(  ( ucTD1 & 0x0F )
          ||( ucTD1 & 0x80 )
         )
      )
    {
        ucCh = 0x00;
        for(ucI=1;ucI<pCurrICCInfo->ucATRLen;ucI++)
        {
            ucCh = ucCh ^ pCurrICCInfo->aucATR[ucI];
        }
        if( ucCh )
            ucResult = LIBICCERR_BADTCK;
    }
    if( !ucResult )
    {
    	switch( ucTA1&0x0F )
    	{
    	case 1:
    	    pCurrICCInfo->ucDi = 4;
    	    break;
    	case 2:
    	    pCurrICCInfo->ucDi = 2;
    	    break;
    	case 3:
    	    pCurrICCInfo->ucDi = 4;
    		break;
    	case 4:
    	    pCurrICCInfo->ucDi = 4;
    		break;
    	case 8:
    	    pCurrICCInfo->ucDi = 8;
    		break;
    	default:
			if( pCurrICCInfo->ucStandard==ICCSTANDARD_QINGD )
    			pCurrICCInfo->ucDi = 4;
    		else
    			ucResult = LIBICCERR_BADTA1;
    		break;
    	}
    }


    if( !ucResult )
    {
	    pCurrICCInfo->ucProtocol = ucProtocol;
	    pCurrICCInfo->ucGTR = ucTC1;
	    pCurrICCInfo->ucFi = (ucTA1&0xF0)>>4;
	    if( ucTC2 )
	    {
		    pCurrICCInfo->ulWWT = 960*ucTC2*(pCurrICCInfo->ucDi)+(pCurrICCInfo->ucDi*480);
		    pCurrICCInfo->ulWWT += 1;
		}else
		{
			ucResult = LIBICCERR_BADTC2;
		}
	}
	if( !ucResult )
	{
	    if( ucProtocol )
	    {
    		ucCh = ucTB3&0x0F;
    		if( ucCh > 5 )
    		{
    			ucResult = LIBICCERR_BADTB3;
    		}
   			if(pCurrICCInfo->ucStandard != ICCSTANDARD_NEW)
   			{
	    		if( !ucResult )
	    		{
	    			ucCh = (ucTB3&0xF0) >> 4;
	    			if( ucCh > 4 )
	    			{
	    				ucResult = LIBICCERR_BADTB3;
	    			}
	    		}
			}
    		if( !ucResult )
    		{
    		    if(  (ucTC1 == 0x00 )
    		       &&(ucTB3 == 0x00 )
    		      )
    		        ucResult = LIBICCERR_BADTB3;
    		    else
    		    {
        	        if(  ( ucTC1 > 0x1E )
        	           &&( ucTC1 != 0xFF)
        	          )
        	        {
        	            ucResult = LIBICCERR_BADTB3;
        	        }else
        	        {
        	            ucCh = 1;
                    	for(uiI=0;uiI<(ucTB3&0x0F);uiI++)
                    	{
                    		ucCh *= 2;
                    	}
                    	ucCh += 11;
                    	if( ucTC1 == 0xFF )
                    	{
                    	    if( ucCh <= 11 )
                    	    {
                        	    ucResult = LIBICCERR_BADTB3;
                    	    }
                    	}else
                    	{
                        	if( ucCh <= (ucTC1+1) )
                        	    ucResult = LIBICCERR_BADTB3;
                        }
        	        }
        	    }
    	    }
    	}
	}
	if( !ucResult )
	{
	    switch(pCurrICCInfo->ucStandard)
	    {
            case ICCSTANDARD_QINGD:
    	    	ucResult = QINGDAO_CheckICCParam(ucReader);
    	    	break;
    	    default:
    	        LIBICC_CODENOTIMPLEMENT
    	        break;
	    }
	}
    /*EN: Set TDA8007 register*/
    if( !ucResult )
    {
        ucResult = HALICC_SetProtocol(ucReader,ucProtocol);
    }
    if( !ucResult )
    {
        ucResult = HALICC_SetGTR(ucReader,pCurrICCInfo->ucGTR);
    }
    if( !ucResult )
    {
    	switch(pCurrICCInfo->ucStandard)
	    {
            case ICCSTANDARD_QINGD:
		    	ucResult = HALICC_SetDi(ucReader,pCurrICCInfo->ucDi);
		    	break;
		    default:
		        LIBICC_CODENOTIMPLEMENT
		        break;
		 }
    }
    if(  (!ucResult)
       &&( ucProtocol )
      )
    {
    	pCurrICCInfo->ucT1Status = 0;
		pCurrICCInfo->ucT1ErrorCount = 0;
        pCurrICCInfo->ucT1IFSC = ucTA3;
        pCurrICCInfo->ucT1IFSReqFlag = 1;
        pCurrICCInfo->ucT1EDCMode = ucTC3&0x01;
    	pCurrICCInfo->ucT1SendSeq = 0;
    	pCurrICCInfo->ucT1RecvSeq = 0;
    	pCurrICCInfo->uiCWT = 1;
    	ucCh = ucTB3&0x0F;
    	for(uiI=0;uiI<ucCh;uiI++)
    	{
    		pCurrICCInfo->uiCWT *= 2;
    	}
    	pCurrICCInfo->uiCWT += 11;
    	pCurrICCInfo->uiCWT += 4;
    	pCurrICCInfo->ulBWT = 1;
    	ucCh = (ucTB3&0xF0)>>4;
    	for(uiI=0;uiI<ucCh;uiI++)
    	{
    		pCurrICCInfo->ulBWT *= 2;
    	}
    	pCurrICCInfo->ulBWT *= 960;
    	pCurrICCInfo->ulBWT *= pCurrICCInfo->ucDi;
    	pCurrICCInfo->ulBWT += 11;
    	pCurrICCInfo->ulBWT += pCurrICCInfo->ucDi*960;
    	pCurrICCInfo->ucWTX = 1;
    }
	pCurrICCInfo->ucLastError = ucResult;

    return(HALICCERR_SUCCESS);
}


extern unsigned char HALICC_Deactive(unsigned char ucReader);

unsigned char ATR_FinishedProc(unsigned char ucReader)
{
        unsigned char ucLen;
        unsigned char aucResult[LIBICC_MAXATRBUF];
        ICCINFO *pCurrICCInfo;

   //     unsigned char pucInData1[19] = {  0x00,0xA4,0x04,0x00,0x0e, 0x31,0x50,0x41,0x59,0x2E,0x53,0x59,0x53,0x2E,0x44,0x44,0x46,0x30,0x31};



        pCurrICCInfo = &ICCInfo[ucReader];
        pCurrICCInfo->ucAutoPower = 0;
        memset(aucResult,0,sizeof(aucResult));
        ucLen = 1;
      //  if (gxd)
#if 0
        printk("%s, line:%d,pCurrICCInfo->ucLastError :0x%x\n",__func__, __LINE__,pCurrICCInfo->ucLastError);
#endif
     //   printk("%s, line:%d,pCurrICCInfo->ucLastError :0x%x\n",__func__, __LINE__,pCurrICCInfo->ucLastError);
        switch ( pCurrICCInfo->ucLastError ) {
        case LIBICCERR_ATRTOOLONG:
        case LIBICCERR_BADTA1:
        case LIBICCERR_BADTB1:
        case LIBICCERR_BADTD1:
        case LIBICCERR_BADTA2:
        case LIBICCERR_BADTB2:
        case LIBICCERR_BADTC2:
        case LIBICCERR_BADTD2:
        case LIBICCERR_BADTA3:
        case LIBICCERR_BADTB3:
        case LIBICCERR_BADTC3:
        case LIBICCERR_BADT0_20:
        	//if (gxd)
        	// printk("%s, line:%d,pCurrICCInfo->ucResetType :0x%x\n",__func__, __LINE__,pCurrICCInfo->ucResetType);
                if ( pCurrICCInfo->ucResetType == LIBICC_COLDRESET ) {
                        aucResult[ucLen] = LIBICCERR_NEEDWARMRESET;
                } else {
                        aucResult[ucLen] = pCurrICCInfo->ucLastError;
                }

                break;
        case LIBICCERR_SUCCESS:
                aucResult[ucLen] = LIBICCERR_ISASYNCCARD;
                break;
        default:
                aucResult[ucLen] = pCurrICCInfo->ucLastError;
                break;
        }

 	//if(aucResult[ucLen] !=LIBICCERR_NEEDWARMRESET ||aucResult[ucLen] !=LIBICCERR_ISASYNCCARD)
        
	// HALICC_Deactive(ucReader);
		
        ucLen ++;
        aucResult[ucLen] = (unsigned char )pCurrICCInfo->ucATRLen;
        ucLen ++;
        memcpy(&aucResult[ucLen],pCurrICCInfo->aucATR,pCurrICCInfo->ucATRLen);
        ucLen += pCurrICCInfo->ucATRLen;
        aucResult[0] = ucLen-1;


        /*********************************************************
        		��ӡATR��λ���
        **********************************************************/
      extern int dbg;
     int i;
     if (dbg)
     {
        printk("%s aucResult\n", __func__);
               for (i=0; i<ucLen; i++)
                       printk("%02x ",aucResult[i]);
               printk("\n");
     }
 #if 0

        int i;
        printk("%s atr data:\n", __func__);
        for (i=0; i<pCurrICCInfo->ucATRLen ; i++)
                printk("%02x ",pCurrICCInfo->aucATR[i]);
        printk("\n");

        printk("%s aucResult\n", __func__);
        for (i=0; i<ucLen; i++)
                printk("%02x ",aucResult[i]);
        printk("\n");
//#if 0

//        printk("start  PROT_AsyncCommand()\n");
   //     #if 0
        unsigned char pucInData1[] = {  0x04,0x00,0xA4,0x04,0x00,
        		                                             0x0e,0x31,0x50,0x41,
        		                                             0x59,0x2E,0x53,0x59,
        		                                             0x53,0x2E,0x44,0x44,
        		                                             0x46,0x30,0x31};

        PROT_AsyncCommand(0,pucInData1,sizeof(pucInData1) / sizeof(unsigned char));

#endif

        HALICC_DRVEnd(LIBICCERR_SUCCESS,aucResult,ucLen);

    //    complete(&comp1);

        return(pCurrICCInfo->ucLastError);
}

void ATR_CallbackRecvChar(unsigned char ucReader,unsigned char ucCh)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];

        if (icc_dbg>=3)
                printk("%s r:%x, pCurrICCInfo->ucStandard=%x\n", __func__, ucCh, pCurrICCInfo->ucStandard);

        switch ( pCurrICCInfo->ucStandard) {
        case ICCSTANDARD_ISO7816:
                ISO7816_RecvCharProc(ucReader,ucCh);
                break;
        case ICCSTANDARD_EMV:
                EMV_RecvCharProc(ucReader,ucCh);
                break;
        case ICCSTANDARD_GJISAM:
                GJISAM_RecvCharProc(ucReader,ucCh);
                break;
        case ICCSTANDARD_GJPSAM:
                GJPSAM_RecvCharProc(ucReader,ucCh);
                break;
        case ICCSTANDARD_SHEBAO:
                SHEBAO_RecvCharProc(ucReader,ucCh);
                break;
        case ICCSTANDARD_QINGD:
                QINGDAO_RecvCharProc(ucReader,ucCh);
        default:
                LIBICC_CODENOTIMPLEMENT
                break;
        }
}

void ATR_CallbackTimeout(unsigned char ucReader,unsigned char ucTimeoutType)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        if (icc_dbg>=2) printk("%s ucSession:%x\n", __func__, pCurrICCInfo->ucSession);
        switch ( pCurrICCInfo->ucStandard) {
        case ICCSTANDARD_ISO7816:
                ISO7816_TimeoutProc(ucReader,ucTimeoutType);
                break;
        case ICCSTANDARD_EMV:
                EMV_TimeoutProc(ucReader,ucTimeoutType);
                break;
        case ICCSTANDARD_GJISAM:
                GJISAM_TimeoutProc(ucReader,ucTimeoutType);
                break;
        case ICCSTANDARD_GJPSAM:
                GJPSAM_TimeoutProc(ucReader,ucTimeoutType);
                break;
        case ICCSTANDARD_SHEBAO:
                SHEBAO_TimeoutProc(ucReader,ucTimeoutType);
                break;
        case ICCSTANDARD_QINGD:
               QINGD_TimeoutProc(ucReader,ucTimeoutType);
               break;
        default:
                LIBICC_CODENOTIMPLEMENT
                break;
        }
}

void ATR_CallbackCardRemove(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        if (icc_dbg)
                printk("%s:%d\n", __func__, __LINE__);
        pCurrICCInfo = &ICCInfo[ucReader];
//    if( pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_IDLE )
        {

        	//	complete(&comp);
                pCurrICCInfo->ucLastError = LIBICCERR_CARDREMOVE;
                pCurrICCInfo->ucSession = ICCSESSION_IDLE;
                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_IDLE;
                ATR_FinishedProc(ucReader);
        }
}

void ATR_CallbackFault(unsigned char ucReader)
{
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        if (icc_dbg)
                printk("%s:%d, pCurrICCInfo->ucSessionStep=%x\n", __func__, __LINE__, pCurrICCInfo->ucSessionStep);

//        return;//todo

        if ( pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_IDLE ) {
                pCurrICCInfo->ucLastError = LIBICCERR_VCCPROBLEM;
                pCurrICCInfo->ucSession = ICCSESSION_IDLE;
                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_IDLE;
                ATR_FinishedProc(ucReader);
        }
}

void ATR_CallbackParityError(unsigned char ucReader)
{
        unsigned char ucI;
        ICCINFO *pCurrICCInfo;

        pCurrICCInfo = &ICCInfo[ucReader];
        if (icc_dbg)
                printk("%s:%d, pCurrICCInfo->ucSessionStep=%x\n", __func__, __LINE__, pCurrICCInfo->ucSessionStep);

//        return;//todo
        
 //       if (  (pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_COLDRESET1 )
//       &&(pCurrICCInfo->ucSessionStep != ICCSESSIONSTEP_IDLE )
 //          )
            {
                HALICC_SetTimer(ucReader,0x00,0x00,0x00,0x00,1);
                pCurrICCInfo->ucLastError = LIBICCERR_ATRPARITYERROR;
                pCurrICCInfo->ucSession = ICCSESSION_IDLE;
                pCurrICCInfo->ucSessionStep = ICCSESSIONSTEP_IDLE;
                printk("PARITY ERROR ATR=");
                for (ucI=0; ucI<pCurrICCInfo->ucATRLen; ucI++) {
                        printk("%02x,",pCurrICCInfo->aucATR[ucI]);
                }
                printk("\n");
                ATR_FinishedProc(ucReader);
        }
}
