
#include "mythread.h"
#include <QDebug>


#include "include.h"
#include "global.h"
#include "xdata.h"
#include "mifare.h"
#include "osdrv.h"


MyThread::MyThread(int value)
{
    flag = value;
    tempValue[64] = 0;

    timer.start(100);
    connect(&timer,SIGNAL(timeout()),this,SLOT(slot_checkValue()));
}

void MyThread::slot_checkValue()
{
    qDebug()<<"~~~~~~~~~~~aucLine1  tempValue"<<ProUiFace.ProToUi.aucLine1<<tempValue;
    if(memcmp(ProUiFace.ProToUi.aucLine1,tempValue,64))
    {
        qDebug()<<"IN AAAAAAAAAAAA";
        emit value_Change(ProUiFace.ProToUi.aucLine1);
        memcpy(tempValue,ProUiFace.ProToUi.aucLine1,64);
    }
    qDebug()<<"IN BBBBBBBBB";

}

MyThread::~MyThread()
{
    if(timer.isActive())
        timer.stop();
}

void  MyThread::run()
{
    uchar ucResult=0;
    unsigned char  ucStatus,aucOutData[7],ucLen;

    UTIL_ClearGlobalData();

    NORMALTRANS_ucTransType = ProUiFace.UiToPro.ucTransType;
    switch(NORMALTRANS_ucTransType)
    {
    case TRANS_S_INIONLINE: // init
        qDebug()<<"init process***********************************";
        LOGON_IniProcess();
        sleep(3);
        emit(signal_mythread());
        break;
    case TRANS_S_LOGONON: // logon
        LOGON_LogonProcess();
         sleep(3);
        emit(signal_mythread());
        break;
    case TRANS_S_BATCHUP: // settle
        ucResult=STL_Settle();

        if(ucResult!=SUCCESS)
        {
            qDebug()<<"11111111111";
            MSG_DisplayErrMsg(ucResult);
             qDebug()<<"2222222222";
        }
         sleep(2);

        emit(signal_mythread());
        break;
    case TRANS_S_PURCHASE:
        ucResult=Untouch_Purchase();

#if 0
        while(1)
        {
            ucResult=Untouch_ReadSierial_tmp(&ucStatus,aucOutData,&ucLen);
            qDebug()<<"***********Untouch_Reau   ucResult %02x\n=================="<<ucResult;

            if(ucResult==0x04){
               ucResult = SUCCESS;
               break;
        }

        memset((uchar*)&ProUiFace.ProToUi,0,sizeof(ProUiFace.ProToUi));
        ProUiFace.ProToUi.uiLines=1;
        memcpy(ProUiFace.ProToUi.aucLine1,"消费成功,请取卡",30);

        }

        //MIFARE_CloseRF();
        //OSDRV_Abort(14);
#else
        sleep(3);
#endif
        emit(signal_mythread());
        break;
    default:
        break;

    }

    if(flag){
        switch(flag)
        {
        case 1:
            qDebug()<<"case 1111111111***************************logo off*********************";

            break;
        case 2:
            LOGON_DownTransKeyProcess();
             sleep(2);
            emit(signal_mythread());
            break;
        case 3:
            Untouch_OnlinePurchase();
             sleep(2);
            emit(signal_mythread());
            break;
        case 4:
            Untouch_Void();
             sleep(2);
            emit(signal_mythread());
            break;
        case 5:
            qDebug()<<"case 5**************************";
            // PRT_Reprint();
            // emit(signal_mythread());
            break;

        case 6:
            qDebug()<<"case 6**************************";
            //  PRT_ReprintbyTrace();
            break;
        case 7:
            qDebug()<<"case 7**************************";
            //  PRT_ReprintSettle();
            // emit(signal_mythread());
            break;
        case 8:
            qDebug()<<"case 7**************************";
            //  PRT_Detail();
            //emit(signal_mythread());
            break;
        case 9:
            qDebug()<<"case 7**************************";

            break;
        case 10:
            qDebug()<<"case 7**************************";
            // PRT_TerminalTt();
            // emit(signal_mythread());
            break;
        }
    }



}

}
