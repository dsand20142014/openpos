/*
--------------------------------------------------------------------------
	FILE  osmifare.c
--------------------------------------------------------------------------
*/

#include <osicc.h>
#include <osdata.h>
#include <osmifare.h>
#include <classic.h>
#include <dicard.h>
#include <desfire.h>
#include <string.h>
#include <haldrv.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <assert.h>

//#define M1_TEST     0

#define rf_dbg 0
#define RFDBG(format, args...)  do{if (rf_dbg) printf("[libapp: %s, %d]" format "\n", __func__,__LINE__, args);}while(0)
#define RFDBG01(msg)  do{if (rf_dbg) printk("[libapp: %s, %d]" msg "\n", __func__,__LINE__);}while(0)
#define RFDBG0()  do{if (rf_dbg) printk(" **[libapp: %s, %d] ***\n\n", __func__,__LINE__);}while(0)

#define OSMIFARE_CMD_REQUEST    		0x00
#define OSMIFARE_CMD_ANTI		      	0x01
#define OSMIFARE_CMD_SELECT		      	0x02
#define OSMIFARE_CMD_AUTH				0x03
#define OSMIFARE_CMD_RATS				0x04
#define OSMIFARE_CMD_SENDDATA			0x05
#define OSMIFARE_CMD_SENDREVDATA		0x06
#define OSMIFARE_CMD_SENDREVFRAME		0x07
#define OSMIFARE_CMD_CLOSERF			0x08
#define OSMIFARE_CMD_HALT				0x09
#define OSMIFARE_CMD_POLLING			0x0A
#define OSMIFARE_CMD_ACTIVE				0x0B
#define OSMIFARE_CMD_DESELECT			0x0C
#define OSMIFARE_CMD_REMOVE				0x0D
#define OSMIFARE_CMD_POWERF				0x0E

#define CLASSIC_ORDER_READ	    	0x30
#define CLASSIC_ORDER_WRITE	      	0xA0
#define CLASSIC_ORDER_INC	      	0xC1
#define CLASSIC_ORDER_DEC			0xC0
#define CLASSIC_ORDER_RESTORE		0xC2
#define CLASSIC_ORDER_TRANSFER		0xB0
#define CLASSIC_ORDER_AUTHA			0x60
#define CLASSIC_ORDER_AUTHB			0x61


typedef struct
{
    unsigned char ucRxThresHoldTypeA;
	unsigned char ucRxThresHoldTypeB;
	unsigned char ucIndexTypeB;
	unsigned char ucIndexTypeA;
}MIFARECFG;

extern void OS_PCD_SETCFG(MIFARECFG option);

extern void OS_PCD_GETCFG(MIFARECFG *option);

#define PCD_IOC_MAGIC   'P'

//add PCD_RESET for app
#define PCD_IOC_PCDRESET            _IOWR(PCD_IOC_MAGIC,  17, int)

//add PCD_ANALOGUE for app
#define PCD_IOC_ANALOGUE            _IOWR(PCD_IOC_MAGIC,  18, int)


#define PCD_IOC_GET_CFG             _IOWR(PCD_IOC_MAGIC,  19, MIFARECFG)
#define PCD_IOC_SET_CFG             _IOWR(PCD_IOC_MAGIC,  20, MIFARECFG)

#define MF_CMD_PROCESS	            _IOWR(PCD_IOC_MAGIC,  0,DRV)


/*
EN: Internal Function
*/
void OSMIFARE_SetReport(unsigned char val);

static MIFARE_ANSWER  MIFAREReport;
static MIFIRE_ORDER   *pMIFAREOrder;

DRVIN	OSMIFAREDrvIn;
DRVOUT	OSMIFAREDrvOut;
DRVOUT	OSMIFAREDrvOut2;
DRVOUT *pOSMIFAREDrvOut;

int  fd_mifare = -1;


DRV_MMAP *myOSMFDrv;


void OSMIFARE_Close(void)
{
        if (fd_mifare) {
                close(fd_mifare);
              //  release(fd_mifare);
                fd_mifare=0;
             //   printf("11111close ok 22222");
        }
}

//new add
int  OSMIFARE_Open(void)
{
        OSMIFARE_Close();

        fd_mifare = open("/dev/rc531", O_RDWR);
        if (fd_mifare<0) {
                printf("open MIFARE device failed!\n");
                return -1;
        }

        myOSMFDrv = (DRV_MMAP *)malloc(sizeof(DRV_MMAP));
        if(!myOSMFDrv)
        {
            printf("myOSMFDrv malloc failed!\n");
            return -2;
        }

        return fd_mifare;
}



void OSDRV_Active_MF(unsigned char ucDrvID,DRVIN *pDrvIn,DRVOUT *pDrvOut)
{
        int i;
        int ret;

        DRV_MMAP tmp_drv;

		if(fd_mifare < 0)
		{
		    ret = OSMIFARE_Open();
			if (ret < 0)
			{
				printf("******OSMIFARE_Open failed, ret=%d********\n\n\n", ret);
				return;
			}
		}

        switch (ucDrvID) {
        case HALDRVID_MIFARE: {

//xu.xb     //mmap bug
#if     0
                memset(myOSMFDrv, 0xaa, sizeof(DRV_MMAP));
                memset(pDrvOut, 0xaa, sizeof(DRVOUT));

                myOSMFDrv->ucDrvID = ucDrvID;
                myOSMFDrv->ucDrvCMD = HALDRVCMD_EXECUTE;
                myOSMFDrv->pDrvIn.ucType = pDrvIn->ucType;
                myOSMFDrv->pDrvIn.ucLen = pDrvIn->ucLen;
                memcpy(myOSMFDrv->pDrvIn.aucData, pDrvIn->aucData, sizeof(pDrvIn->aucData));

                ioctl(fd_mifare,MF_CMD_PROCESS, (DRV_MMAP *)myOSMFDrv);

                memcpy(pDrvOut, &myOSMFDrv->pDrvOut, sizeof(DRVOUT));

#else

                memset((char *)&tmp_drv, 0xaa, sizeof(DRV_MMAP));
                memset(pDrvOut, 0xaa, sizeof(DRVOUT));

                tmp_drv.ucDrvID = ucDrvID;
                tmp_drv.ucDrvCMD = HALDRVCMD_EXECUTE;
                tmp_drv.pDrvIn.ucType = pDrvIn->ucType;
                tmp_drv.pDrvIn.ucLen = pDrvIn->ucLen;
                memcpy(tmp_drv.pDrvIn.aucData, pDrvIn->aucData, sizeof(pDrvIn->aucData));

                ioctl(fd_mifare,MF_CMD_PROCESS, (DRV_MMAP *)&tmp_drv);

                memcpy(pDrvOut, &tmp_drv.pDrvOut, sizeof(DRVOUT));

#endif


                pOSMIFAREDrvOut=&myOSMFDrv->pDrvOut;


#if 0
                for (i=0; i<pDrvOut->aucData[0]+2; i++)
                        printf("%02x ", pDrvOut->aucData[i]);
                printf("\n");
                printf("***************End of %s *******************\n\n\n", __func__);
#endif
                break;
        }

        }
}


unsigned char OSDRV_Wait_MF(DRVOUT *pDrvOut)
{
        int i;
        int ret;

        static unsigned long count = 1;

        if(fd_mifare < 0)
        {
            return HALDRVSTATE_ABSENT;
        }

#ifdef  M1_TEST
        //test driver read
        ret = read(fd_mifare, pDrvOut, count);
#else
        ret = read(fd_mifare, pDrvOut, sizeof(pDrvOut));
#endif


#ifdef  M1_TEST

        printf("\nAPI %s Num=%d, ucReqNR=%02x ucGenStatus=%02x ucDrvStatus=%02x\n", __func__, count,
                    pDrvOut->ucReqNR, pDrvOut->ucGenStatus, pDrvOut->ucDrvStatus);

        printf("%s aucData:\n", __func__);
        for (i=0; i<8; i++)
                printf("%02x ", pDrvOut->aucData[i]);
        printf("%s \n", __func__);

        count++;

#endif


#if 0
        for (i=0; i<pDrvOut->aucData[0]; i++)
                printf("%02x ", pDrvOut->aucData[i]);
        printf("End\n");
        printf("***************End of %s, ret=%#x,pDrvOut->ucDrvStatus=%#x, len=%#x\n\n\n",
               __func__, ret, pDrvOut->ucDrvStatus, pDrvOut->aucData[0]);
#endif


        return(pDrvOut->ucDrvStatus);
}




void OSMIFARE_DrvActive(void)
{
   OSDRV_Active_MF(HALDRVID_MIFARE,&OSMIFAREDrvIn,&OSMIFAREDrvOut);
}

void OSMIFARE_DrvRun(void)
{
        int i;
	  //  printf("OSMIFARE_DrvRun SEND data, type=%02x len=%#02x\n",OSMIFAREDrvIn.ucType, OSMIFAREDrvIn.ucLen);
        if (0) {
                for (i=0; i<OSMIFAREDrvIn.ucLen; i++)
                        printf("%02x ", OSMIFAREDrvIn.aucData[i]);
                printf("\n");
        }

        OSDRV_Active_MF(HALDRVID_MIFARE,&OSMIFAREDrvIn,&OSMIFAREDrvOut2);
        OSDRV_Wait_MF(&OSMIFAREDrvOut2);

	 // printf("OSMIFARE_DrvRun RCV data, len=%#02x\n", OSMIFAREDrvOut2.aucData[1]+2);
        if (0) {
                for (i=0; i<OSMIFAREDrvOut2.aucData[1]+2; i++)
                        printf("%02x ", OSMIFAREDrvOut2.aucData[i]);
                printf("\n");
        }
}


void OSMIFARE_CloseRF(void)
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_CLOSERF;
        OSMIFAREDrvIn.ucLen = 0;
        OSMIFARE_DrvRun();
}


unsigned char OS_PCD_RESET(void)
{
        int ret;

		if(fd_mifare < 0)
		{
		    ret = OSMIFARE_Open();
			if (ret < 0)
			{
				printf("******OSMIFARE_Open failed, ret=%d********\n\n\n", ret);
				return;
			}
		}

		ioctl(fd_mifare,PCD_IOC_PCDRESET);

        OSDRV_Wait_MF(&OSMIFAREDrvOut2);

        return(OSMIFAREDrvOut2.aucData[0]);
}

unsigned char OS_PCD_ANALOGUE(unsigned char ucCardType)
{
       int ret;
       int cardtype;

		if(fd_mifare < 0)
		{
		    ret = OSMIFARE_Open();
			if (ret < 0)
			{
				printf("******OSMIFARE_Open failed, ret=%d********\n\n\n", ret);
				return;
			}
		}

		cardtype = ucCardType;

		ioctl(fd_mifare,PCD_IOC_ANALOGUE, &cardtype);

        OSDRV_Wait_MF(&OSMIFAREDrvOut2);

        return(OSMIFAREDrvOut2.aucData[0]);
}


void OS_PCD_SETCFG(MIFARECFG option)
{
    MIFARECFG cfg;
    int ret;

    cfg = option;

   if(fd_mifare < 0)
   {
       ret = OSMIFARE_Open();
       if (ret < 0)
       {
           printf("******OSMIFARE_Open failed, ret=%d********\n\n\n", ret);
           return;
       }
   }

   ioctl(fd_mifare,PCD_IOC_SET_CFG, &cfg);
}

void OS_PCD_GETCFG(MIFARECFG *option)
{
    MIFARECFG cfg;
    int ret;   

   if(fd_mifare < 0)
   {
       ret = OSMIFARE_Open();
       if (ret < 0)
       {
           printf("******OSMIFARE_Open failed, ret=%d********\n\n\n", ret);
           return;
       }
   }

   ioctl(fd_mifare,PCD_IOC_GET_CFG, &cfg);
   
   *option = cfg;
}


unsigned char OSMIFARE_Request( unsigned char ucCardType,unsigned char ucReqMode )
{
        OSMIFARE_CloseRF(); //todo zy 0623
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_REQUEST;
        OSMIFAREDrvIn.ucLen = 2;
        OSMIFAREDrvIn.aucData[0] = ucCardType;
        OSMIFAREDrvIn.aucData[1] = ucReqMode;

        //new driver mode
        OSMIFARE_DrvRun();


#ifdef  M1_TEST
        printf("@@@@@ %s \n", __func__);
#endif

        return(OSMIFAREDrvOut2.aucData[0]);
}

unsigned char OSMIFARE_Anti( unsigned char ucAntiEn,unsigned char *pucCardId,
                             unsigned char *pucCardIdLen,unsigned char *pucCardClass )
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_ANTI;
        OSMIFAREDrvIn.ucLen = 0;
        OSMIFAREDrvIn.aucData[0] = ucAntiEn;
        OSMIFARE_DrvRun();
        if ( OSMIFAREDrvOut2.aucData[0]==OSMIFARE_ERR_SUCCESS ) {
                if ( OSMIFAREDrvOut2.aucData[1] ) {
                        *pucCardClass = OSMIFAREDrvOut2.aucData[2];
                        *pucCardIdLen = OSMIFAREDrvOut2.aucData[1]-1;
                        memcpy( pucCardId,&OSMIFAREDrvOut2.aucData[3],*pucCardIdLen );
                }
        }

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif

        return(OSMIFAREDrvOut2.aucData[0]);
}

unsigned char OSMIFARE_Select( unsigned char *pucCardID,unsigned char ucCardIDLen,unsigned char *pucCardClass )
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_SELECT;
        OSMIFAREDrvIn.ucLen = ucCardIDLen;
        memcpy( OSMIFAREDrvIn.aucData,pucCardID,ucCardIDLen );
        OSMIFARE_DrvRun();
        if ( OSMIFAREDrvOut2.aucData[0]==OSMIFARE_ERR_SUCCESS ) {
                if ( OSMIFAREDrvOut2.aucData[1] ) {
                        *pucCardClass = OSMIFAREDrvOut2.aucData[2];
                }
        }

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif

        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_Auth( unsigned char ucAuthType,unsigned char ucSectorIndex,unsigned char ucKeyType,
                             unsigned char *pucKey )
{
        unsigned char ucI, i=0;
#if 0
        printf("\n\n%s,%d, ucAuthType=%x,ucSectorInde=%x, ucKeyType=%x, key[0,2,5]=%02x %02x %02x \n",
               __func__, __LINE__,
               ucAuthType, ucSectorIndex, ucKeyType,
               pucKey[0], pucKey[2], pucKey[5]
              );
#endif
        ucI = 0;
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_AUTH;
        OSMIFAREDrvIn.aucData[ucI++] = ucAuthType;
        OSMIFAREDrvIn.aucData[ucI++] = ucSectorIndex;
        OSMIFAREDrvIn.aucData[ucI++] = ucKeyType;
        memcpy( &OSMIFAREDrvIn.aucData[ucI],pucKey,6 );
        ucI += 6;
        OSMIFAREDrvIn.ucLen = ucI;
#if 0
        for (i=0; i<OSMIFAREDrvIn.ucLen; i++)
                printf("%02x ", OSMIFAREDrvIn.aucData[i]);
        printf("\n");
#endif
        OSMIFARE_DrvRun();

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif

        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_GetAts( unsigned char *pucAts,unsigned char *pucAtsLen )
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_RATS;
        OSMIFAREDrvIn.ucLen = 0;
        OSMIFARE_DrvRun();
        if ( OSMIFAREDrvOut2.aucData[0]==OSMIFARE_ERR_SUCCESS ) {
                if ( OSMIFAREDrvOut2.aucData[1] ) {
                        *pucAtsLen = OSMIFAREDrvOut2.aucData[1];
                        memcpy( pucAts,&OSMIFAREDrvOut2.aucData[2],*pucAtsLen );
                }
        }
        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_SendRevData( unsigned char ucCheckMode,unsigned char *pucSendData,unsigned int uiSendLen,
                                    unsigned char *pucRevData,unsigned char*pucRevLen,unsigned int uiTimeDelay )
{
        unsigned char ucI, i;

        ucI = 0;
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_SENDREVDATA;
        OSMIFAREDrvIn.aucData[ucI++] = ucCheckMode;
        OSMIFAREDrvIn.aucData[ucI++] = uiTimeDelay/256;
        OSMIFAREDrvIn.aucData[ucI++] = uiTimeDelay%256;
        memcpy( &OSMIFAREDrvIn.aucData[ucI],pucSendData,uiSendLen );
        ucI += uiSendLen;
        OSMIFAREDrvIn.ucLen = ucI;
#if 0
        printf("%s %d\n", __func__, __LINE__);
        for (i=0; i<OSMIFAREDrvIn.ucLen; i++)
                printf("%02x ", OSMIFAREDrvIn.aucData[i]);
        printf("\n");
#endif
        OSMIFARE_DrvRun();

        if ( OSMIFAREDrvOut2.aucData[0]==OSMIFARE_ERR_SUCCESS ) {
                if ( OSMIFAREDrvOut2.aucData[1] ) {
                        *pucRevLen = OSMIFAREDrvOut2.aucData[1];
                        memcpy( pucRevData,&OSMIFAREDrvOut2.aucData[2],*pucRevLen );
                }
        }

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif

        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_SendData( unsigned char ucCheckMode,unsigned char *pucSendData,unsigned int uiSendLen )
{
        unsigned char ucI;

        ucI = 0;
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_SENDDATA;
        OSMIFAREDrvIn.aucData[ucI++] = ucCheckMode;
        memcpy( &OSMIFAREDrvIn.aucData[ucI],pucSendData,uiSendLen );
        ucI += uiSendLen;
        OSMIFAREDrvIn.ucLen = ucI;
        OSMIFARE_DrvRun();

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif

        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_SendRevFrame( unsigned char *pucSendFrame,unsigned int uiSendLen,
                                     unsigned char *pucRevFrame,unsigned int *puiRevLen,unsigned int uiTimeDelay )
{
        unsigned char ucI;
        unsigned int uiLen;

        ucI = 0;
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_SENDREVFRAME;
        OSMIFAREDrvIn.aucData[ucI++] = uiTimeDelay>>8&0xFF;
        OSMIFAREDrvIn.aucData[ucI++] = uiTimeDelay&0xFF;
        OSMIFAREDrvIn.aucData[ucI++] = uiSendLen/256;
        OSMIFAREDrvIn.aucData[ucI++] = uiSendLen%256;
        memcpy( &OSMIFAREDrvIn.aucData[ucI],pucSendFrame,uiSendLen );
        OSMIFAREDrvIn.ucLen = ucI+uiSendLen;
        OSMIFARE_DrvRun();
        if ( OSMIFAREDrvOut2.aucData[0]==OSMIFARE_ERR_SUCCESS ) {
                uiLen = OSMIFAREDrvOut2.aucData[1]*256+OSMIFAREDrvOut2.aucData[2];
                if ( uiLen ) {
                        *puiRevLen = uiLen;
                        memcpy( pucRevFrame,&OSMIFAREDrvOut2.aucData[3],uiLen );
                }
        }

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif

        return OSMIFAREDrvOut2.aucData[0];
}


unsigned char OSMIFARE_Halt(void)
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_HALT;
        OSMIFAREDrvIn.ucLen = 0;
        OSMIFARE_DrvRun();

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif
        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_AntiProc( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
        unsigned char aucRevData[OSMIFARE_CARDID_MAX_LEN],ucRevLen,ucCardClass;
        unsigned char ucResult;
        BASE_ORDER_TYPE *pSelectOrder;

        pSelectOrder = &pOrder->pt_order_in->base_order;
        memset( aucRevData,0,sizeof(aucRevData) );
        ucRevLen = 0;
        ucResult = OSMIFARE_Anti( pSelectOrder->aucInData[0],aucRevData,&ucRevLen,&ucCardClass );
        if ( ucResult==OSMIFARE_ERR_SUCCESS ) {
                if ( ucRevLen ) {
                        pOrder->ptout[0] = ucCardClass;
                        pAnswer->Len = ucRevLen+1;
                        memcpy( &pOrder->ptout[1],aucRevData,ucRevLen );
                }
        }
        pAnswer->card_status = ucResult;

 #ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,pAnswer->card_status);
#endif
        return pAnswer->card_status;
}

unsigned char OSMIFARE_SelectProc( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
        unsigned char ucResult,ucCardClass;
        BASE_ORDER_TYPE *pSelectOrder;

        pSelectOrder = &pOrder->pt_order_in->base_order;
        ucResult = OSMIFARE_Select( pSelectOrder->aucInData,pSelectOrder->ucInDataLen,&ucCardClass );
        if ( ucResult==OSMIFARE_ERR_SUCCESS ) {
                pOrder->ptout[0] = ucCardClass;
                pAnswer->Len = 1;
        }
        pAnswer->card_status = ucResult;
#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,pAnswer->card_status);
#endif
        return pAnswer->card_status;
}

unsigned char OSMIFARE_RatsProc( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
        unsigned char aucRevData[OSMIFARE_ATS_MAX_LEN],ucRevLen;
        unsigned char ucResult;

        memset( aucRevData,0,sizeof(aucRevData) );
        ucRevLen = 0;
        ucResult = OSMIFARE_GetAts( aucRevData,&ucRevLen );
        if ( ucResult==OSMIFARE_ERR_SUCCESS ) {
                if ( ucRevLen ) {
                        pAnswer->Len = ucRevLen;
                        memcpy( pOrder->ptout,aucRevData,ucRevLen );
                }
        }
        pAnswer->card_status = ucResult;
#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,pAnswer->card_status);
#endif
        return pAnswer->card_status;
}

unsigned char OSMIFARE_HaltProc( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
        unsigned char ucResult;

        ucResult = OSMIFARE_Halt();
#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,pAnswer->card_status);
#endif
        return pAnswer->card_status;
}

unsigned char OSMIFARE_CloseRfProc( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
        OSMIFARE_CloseRF();
        return OSMIFARE_ERR_SUCCESS;
}

unsigned char OSMIFARE_DeSelect()
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_DESELECT;
        OSMIFAREDrvIn.ucLen = 0;
        OSMIFARE_DrvRun();
        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_DeSelectProc( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
        unsigned char ucResult;

        ucResult = OSMIFARE_Halt();
        return pAnswer->card_status;
}

unsigned char OSMIFARE_DigitalProc( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
        unsigned char aucSendData[DICARD_SEND_MAX_LEN];
        unsigned char aucRevData[DICARD_REV_MAX_LEN];
        unsigned char ucResult;
        unsigned int uiSendLen,uiRevLen;
        unsigned int uiTimeDelay;

        memset( aucSendData,0,sizeof(aucSendData) );
        memset( aucRevData,0,sizeof(aucRevData) );
        uiRevLen = 0;
        uiSendLen = pOrder->pt_order_in->dicard_order.uiInDataLen;
        memcpy( aucSendData,pOrder->pt_order_in->dicard_order.aucInData,uiSendLen );
        ucResult = OSMIFARE_SendRevFrame( aucSendData,uiSendLen,aucRevData,&uiRevLen,0 );
        if ( ucResult==OSMIFARE_ERR_SUCCESS ) {
                if ( uiRevLen ) {
                        pAnswer->Len = uiRevLen;
                        memcpy( pOrder->ptout,aucRevData,uiRevLen );
                }
        }
        pAnswer->card_status = ucResult;
        return OSMIFARE_ERR_SUCCESS;
}

unsigned char OSMIFARE_Remove()
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_REMOVE;
        OSMIFAREDrvIn.ucLen = 0;
        OSMIFARE_DrvRun();
#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif
        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_PowerRF(unsigned char ucOnOf)
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_POWERF;
        OSMIFAREDrvIn.ucLen = 1;
        OSMIFAREDrvIn.aucData[0] = ucOnOf;
        OSMIFARE_DrvRun();

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif

        return OSMIFAREDrvOut2.aucData[0];
}

unsigned char OSMIFARE_RemoveProc( MIFARE_ANSWER *pAnswer,MIFIRE_ORDER * pOrder )
{
        unsigned char ucResult;

        ucResult = OSMIFARE_Remove();

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,pAnswer->card_status);
#endif

        return pAnswer->card_status;
}

MIFARE_ANSWER * OSMIFARE_command( MIFIRE_ORDER * pOrder )
{

        pMIFAREOrder = pOrder;
        MIFAREReport.drv_status = WRONGORDER;
        MIFAREReport.card_status = 0;
        MIFAREReport.Len = 0;
        switch (pMIFAREOrder->order) {
        case OSMIFARE_ANTI_ORDER:
                OSMIFARE_SetReport( OSMIFARE_AntiProc( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_SELECT_ORDER:
                OSMIFARE_SetReport( OSMIFARE_SelectProc( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_HALT_ORDER:
                OSMIFARE_SetReport( OSMIFARE_HaltProc( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_CLOSERF_ORDER:
                OSMIFARE_SetReport( OSMIFARE_CloseRfProc( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_RATS_ORDER:
                OSMIFARE_SetReport( OSMIFARE_RatsProc( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_CLASSIC_ORDER:
                OSMIFARE_SetReport( CLASSIC_Process( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_DESFIRE_ORDER:
                OSMIFARE_SetReport(DESFIRE_Process( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_DICARD_ORDER:
                OSMIFARE_SetReport(DICARD_Process( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_DIGITAL_ORDER:
                OSMIFARE_SetReport(OSMIFARE_DigitalProc( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_DESELECT_ORDER:
                OSMIFARE_SetReport(OSMIFARE_DeSelectProc( &MIFAREReport,pOrder ) );
                break;
        case OSMIFARE_REMOVE_ORDER:
                OSMIFARE_SetReport(OSMIFARE_RemoveProc( &MIFAREReport,pOrder ) );
                break;
        default:
                printf("@@@@@ %s ERROR COMMAND\n", __func__);
                break;
        }
        return((MIFARE_ANSWER *)&MIFAREReport);
}

void OSMIFARE_SetReport(unsigned char ucMifareStatus)
{
        MIFAREReport.drv_status = ucMifareStatus;
}

unsigned char OSMIFARE_Active( void )
{
        OSMIFAREDrvIn.ucType = OSMIFARE_CMD_ACTIVE;
        OSMIFAREDrvIn.ucLen = 0;
        OSMIFARE_DrvRun();

#ifdef  M1_TEST
        printf("@@@@@ %s ret=%d\n", __func__,OSMIFAREDrvOut2.aucData[0]);
#endif

        return(OSMIFAREDrvOut2.aucData[0]);
}

unsigned char OSMIFARE_Ioctl(unsigned char ucType,unsigned char *pucData)
{

    printf("@@@@@ %s \n", __func__);

        return(OSDRV_Ioctl(HALDRVID_MIFARE,ucType,pucData));
}

unsigned char OSMIFARE_TestWRBlock(char select)
{
        unsigned char ucResult;
        MIFIRE_ORDER Mifire_OrderBuf;
        MIFARE_ORDER_IN_TYPE Mifire_OrderInTypeBuf;
        unsigned char aucBuf[17];


        memset(aucBuf,0,sizeof(aucBuf));
        memset(&Mifire_OrderBuf,0,sizeof(Mifire_OrderBuf));
        memset(&Mifire_OrderInTypeBuf,0,sizeof(Mifire_OrderInTypeBuf));
        MIFAREReport.card_status=0;

        Mifire_OrderBuf.pt_order_in = &Mifire_OrderInTypeBuf;
        Mifire_OrderBuf.order = OSMIFARE_CLASSIC_ORDER;
        Mifire_OrderInTypeBuf.classic_order.order_type = CLASSIC_ORDER_AUTHA;
        Mifire_OrderInTypeBuf.classic_order.ucBlockIndex = 0x07;
        Mifire_OrderInTypeBuf.classic_order.ucAuthType = 0x01;//zy 0x0
        memset(Mifire_OrderInTypeBuf.classic_order.aucInData,0xFF,6);
        ucResult = CLASSIC_Process(&MIFAREReport,&Mifire_OrderBuf);
        printf("\n@@@@@@@@@@@@@@@@1 auth MIFAREReport.card_status=%02x\n",MIFAREReport.card_status);

        printf("@@@@@@@@ select =%#02x@@@@@@@@@\n", select);
        if (select ==1 || select==3 || select ==4) {

                if (!MIFAREReport.card_status) {
                        memset(&Mifire_OrderBuf,0,sizeof(Mifire_OrderBuf));
                        memset(&Mifire_OrderInTypeBuf,0,sizeof(Mifire_OrderInTypeBuf));
                        Mifire_OrderBuf.pt_order_in = &Mifire_OrderInTypeBuf;
                        Mifire_OrderBuf.order = OSMIFARE_CLASSIC_ORDER;
                        Mifire_OrderInTypeBuf.classic_order.order_type = CLASSIC_ORDER_WRITE;
                        Mifire_OrderInTypeBuf.classic_order.ucBlockIndex = 7*4;
                        Mifire_OrderInTypeBuf.classic_order.ucAuthType = 0;
                        memcpy(Mifire_OrderInTypeBuf.classic_order.aucInData,"1122334455667788",CLASSIC_BLOCK_LEN);
                        ucResult = CLASSIC_Process(&MIFAREReport,&Mifire_OrderBuf);
                }
                printf("\n@@@@@@@@@@@@@@@@2.1 write MIFAREReport.card_status=%02x\n",MIFAREReport.card_status);

#if 0
                if (!MIFAREReport.card_status && select==4) {
                        memset(&Mifire_OrderBuf,0,sizeof(Mifire_OrderBuf));
                        memset(&Mifire_OrderInTypeBuf,0,sizeof(Mifire_OrderInTypeBuf));
                        Mifire_OrderBuf.pt_order_in = &Mifire_OrderInTypeBuf;
                        Mifire_OrderBuf.order = OSMIFARE_CLASSIC_ORDER;
                        Mifire_OrderInTypeBuf.classic_order.order_type = CLASSIC_ORDER_WRITE;
                        Mifire_OrderInTypeBuf.classic_order.ucBlockIndex = 7*4+1;
                        Mifire_OrderInTypeBuf.classic_order.ucAuthType = 0;
                        memcpy(Mifire_OrderInTypeBuf.classic_order.aucInData,"8877665544332211",CLASSIC_BLOCK_LEN);
                        sleep(5);
                        ucResult = CLASSIC_Process(&MIFAREReport,&Mifire_OrderBuf);
                }
                printf("\n@@@@@@@@@@@@@@@@2.2 write MIFAREReport.card_status=%02x\n",MIFAREReport.card_status);
#endif
        }

        if (select==2 || select==3) {
                if (!MIFAREReport.card_status) {
                        memset(&Mifire_OrderBuf,0,sizeof(Mifire_OrderBuf));
                        memset(&Mifire_OrderInTypeBuf,0,sizeof(Mifire_OrderInTypeBuf));
                        Mifire_OrderBuf.pt_order_in = &Mifire_OrderInTypeBuf;
                        Mifire_OrderBuf.order = OSMIFARE_CLASSIC_ORDER;
                        Mifire_OrderInTypeBuf.classic_order.order_type = CLASSIC_ORDER_READ;
                        Mifire_OrderInTypeBuf.classic_order.ucBlockIndex = 7*4;
                        Mifire_OrderInTypeBuf.classic_order.ucAuthType = 0;
                        Mifire_OrderBuf.ptout = aucBuf;
                        memset(Mifire_OrderInTypeBuf.classic_order.aucInData,0x13,CLASSIC_BLOCK_LEN);
                        ucResult = CLASSIC_Process(&MIFAREReport,&Mifire_OrderBuf);
                }
                printf("\n@@@@@@@@@@@@@@@@3.1 read MIFAREReport.card_status=%02x\n",MIFAREReport.card_status);
#if 0
                if (!MIFAREReport.card_status) {
                        memset(&Mifire_OrderBuf,0,sizeof(Mifire_OrderBuf));
                        memset(&Mifire_OrderInTypeBuf,0,sizeof(Mifire_OrderInTypeBuf));
                        Mifire_OrderBuf.pt_order_in = &Mifire_OrderInTypeBuf;
                        Mifire_OrderBuf.order = OSMIFARE_CLASSIC_ORDER;
                        Mifire_OrderInTypeBuf.classic_order.order_type = CLASSIC_ORDER_READ;
                        Mifire_OrderInTypeBuf.classic_order.ucBlockIndex = 7*4+1;
                        Mifire_OrderInTypeBuf.classic_order.ucAuthType = 0;
                        Mifire_OrderBuf.ptout = aucBuf;
                        memset(Mifire_OrderInTypeBuf.classic_order.aucInData,0x12,CLASSIC_BLOCK_LEN);
                        ucResult = CLASSIC_Process(&MIFAREReport,&Mifire_OrderBuf);
                }
                printf("\n@@@@@@@@@@@@@@@@3.2 read MIFAREReport.card_status=%02x\n",MIFAREReport.card_status);
#endif
        }
        if (!MIFAREReport.card_status)
                Os__beep();


        /*
            if (!MIFAREReport.card_status)
            {
                if (memcmp(aucBuf,"1234567890ABCDEF",16))
                    MIFAREReport.card_status = OSMIFARE_ERR_COMPARE;
            }
        */

        return MIFAREReport.card_status;
}


MIFARE_ANSWER * Os__MIFARE_command( MIFIRE_ORDER *pOrder )
{
    return((MIFARE_ANSWER * )OSMIFARE_command(pOrder));
}


unsigned char Os__MIFARE_Request(unsigned char ucCardType,unsigned char ucReqMode)
{
    return (OSMIFARE_Request(ucCardType, ucReqMode));
}

unsigned char Os__MIFARE_Anti( unsigned char ucAntiEn,unsigned char *pucCardId,unsigned char *pucCardIdLen,unsigned char *pucCardClass )
{
    return OSMIFARE_Anti( ucAntiEn,pucCardId,pucCardIdLen,pucCardClass );
}

unsigned char Os__MIFARE_Select( unsigned char *pucCardID,unsigned char ucCardIDLen,unsigned char *pucCardClass )
{
    return OSMIFARE_Select( pucCardID,ucCardIDLen,pucCardClass );
}

unsigned char OSMIFARE_Auth( unsigned char ucAuthType,unsigned char ucSectorIndex,unsigned char ucKeyType,
                             unsigned char *pucKey );
unsigned char Os__MIFARE_Auth( unsigned char ucAuthType,unsigned char ucSectorIndex,unsigned char ucKeyType,
                               unsigned char *pucKey )
{
    return OSMIFARE_Auth( ucAuthType,ucSectorIndex,ucKeyType,pucKey );
}

unsigned char Os__MIFARE_SendData( unsigned char ucCheckMode,unsigned char *pucSendData,unsigned int uiSendLen )
{
    return OSMIFARE_SendData( ucCheckMode,pucSendData,uiSendLen );
}

unsigned char Os__MIFARE_SendRevFrame( unsigned char *pucSendFrame,unsigned int uiSendLen,
                                       unsigned char *pucRevFrame,unsigned int *puiRevLen,unsigned int uiTimeDelay )
{
    return OSMIFARE_SendRevFrame( pucSendFrame,uiSendLen,pucRevFrame,puiRevLen,uiTimeDelay );
}

unsigned char Os__MIFARE_DeSelect( void )
{
    return OSMIFARE_DeSelect();
}

unsigned char Os__MIFARE_Halt( void )
{
    return OSMIFARE_Halt();
}

unsigned char Os__MIFARE_Remove( void )
{
    return OSMIFARE_Remove();
}

/*
0 close
1 open
*/
unsigned char Os__MIFARE_PowerRF( unsigned char ucOnOff )
{
    return OSMIFARE_PowerRF( ucOnOff );
}


unsigned char Os__MIFARE_Active(void)
{
    return OSMIFARE_Active();
}

/*
mifare API Test
code come from sandpay & libVikang
*/
MIFIRE_ORDER 			  MIFARE_Order;
MIFARE_ANSWER 			*MIFARE_pAnswer;
MIFARE_ORDER_IN_TYPE 	  MIFARE_Type;

unsigned char MIFARE_Halt( void )
{
    unsigned char aucRevData[16];

    MIFARE_Order.order = OSMIFARE_HALT_ORDER;
    MIFARE_Order.ptout = aucRevData;
    MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
    if ( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
    {
        return (MIFARE_pAnswer->drv_status);
    }
    return( MIFARE_pAnswer->card_status );
}

unsigned char MIFARE_Anti( unsigned char ucAntiEn,unsigned char *pucCardId,
                           unsigned char *pucCardIdLen,unsigned char *pucCardClass )
{
    unsigned char aucRevData[16];

    MIFARE_Order.order = OSMIFARE_ANTI_ORDER;
    MIFARE_Order.ptout = aucRevData;
    MIFARE_Order.pt_order_in = &MIFARE_Type;
    MIFARE_Type.base_order.ucInDataLen = 1;
    MIFARE_Type.base_order.aucInData[0] = ucAntiEn;
    MIFARE_pAnswer = Os__MIFARE_command( &MIFARE_Order );
    if ( MIFARE_pAnswer->drv_status != OSMIFARE_ERR_SUCCESS )
    {
        return (MIFARE_pAnswer->drv_status);
    }
    if ( MIFARE_pAnswer->Len )
    {
        *pucCardClass = aucRevData[0];
        memcpy( pucCardId,&aucRevData[1],MIFARE_pAnswer->Len-1 );
        *pucCardIdLen = MIFARE_pAnswer->Len-1;
    }
    else
        *pucCardIdLen = 0;
    return( MIFARE_pAnswer->card_status );
}
