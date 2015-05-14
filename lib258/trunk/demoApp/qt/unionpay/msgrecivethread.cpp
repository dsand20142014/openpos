#if  0

#include "msgrecivethread.h"
#include <QDebug>

MsgReciveThread::MsgReciveThread()
{

}
void MsgReciveThread::run()
{
    while(1)
    {
        reciveMsg();
        //sleep(1);
    }
}

void MsgReciveThread::stop()
{

}

void MsgReciveThread::reciveMsg(void)
{
    unsigned int ret;
    NEW_DRV_TYPE  new_drv;

    qDebug("===reciveMsg:Os_Wait_Event -----!");
    ret = Os_Wait_Event(MAG_DRV|KEY_DRV, &new_drv, 100000);
    qDebug("===ret:%02x",ret);

    if(ret == 0)
    {
        qDebug("The card is checked !");
        if(new_drv.drv_type == KEY_DRV)
        {
//              按键数据处理;
        }
        else if(new_drv.drv_type == MAG_DRV)
        {
//              磁卡数据处理;
        }
        else if(new_drv.drv_type == MFR_DRV)
        {
//              非接卡轮询结果处理;
        }
        else if(new_drv.drv_type == ICC_DRV)
        {
//              IC卡插卡结果处理;
        }
        else if(new_drv.drv_type == DRV_TIMEOUT)
        {
//              超时处理;
        }
    }
    else
    {
        if(ret == KEY_DRV_ERROR)
        {
//              按键驱动打开失败处理;
        }
        else if(ret == MAG_DRV_ERROR)
        {
//              磁卡驱动打开失败处理;
        }
        else if(ret == MFR_DRV_ERROR)
        {
//              非接卡驱动打开失败处理;
        }
        else if(ret == ICC_DRV_ERROR)
        {
//              IC卡驱动打开失败处理;
        }
        else
        {
//              其他错误处理;
        }
    }
}


unsigned char MsgReciveThread::MAG_GetTrackInfo(unsigned char *pucTrack)
{
    unsigned short uiI,usJJ,ucJ;
    unsigned char  *pucPtr,ucAccType;

    unsigned char ucISO2Status;
    unsigned int uiISO2Len;
    unsigned char aucISO2[38];
    unsigned char ucISO3Status;
    unsigned int uiISO3Len;
    unsigned char aucISO3[105];

    pucPtr = pucTrack;
    pucPtr += 2;
    //printf("Get:\n");
    //for(i=0;i<100;i++)
    //	printf("%02x ",pucPtr[i]);
    //printf("\n");

    // Track2 information
    ucISO2Status = *(pucPtr);

    pucPtr ++;

    uiISO2Len = *(pucPtr);

    //判断磁道长度是否正确
    if(uiISO2Len > TRANS_ISO2LEN || uiISO2Len<TRANS_ISO2MINLEN)
        return(ERR_MAG_TRACKDATA);

    pucPtr ++;

    printf("\nucISO2Status=%02x\n",ucISO2Status);
    printf("uiISO2Len=%02x\n",uiISO2Len);


    for( ucJ=0; ucJ<uiISO2Len; ucJ++,pucPtr ++)
    {
            aucISO2[ucJ] =  *(pucPtr) | 0x30;
    }
    printf("Track2 Data:%s\n",aucISO2);

       if (!memcmp(&aucISO2[20],"===",3))
        return (ERR_SANDMAGCARD);

    //Track3 information
    ucISO3Status = *(pucPtr);

    pucPtr ++;
    uiISO3Len = *(pucPtr);

    //判断磁道长度是否正确
    if(uiISO3Len > TRANS_ISO3LEN)
        return(ERR_MAG_TRACKDATA);
    pucPtr ++;

    printf("Track3 ISO3Status=%02x\n",ucISO3Status);
    printf("Track3 uiISO3Len=%02x\n",uiISO3Len);


    for( uiI=0; uiI<uiISO3Len; uiI++,pucPtr ++)
        {
            aucISO3[uiI] =  *(pucPtr) | 0x30;
        }
    printf("Track3 Data:%s\n",aucISO3);

    return 0;
}

#endif
