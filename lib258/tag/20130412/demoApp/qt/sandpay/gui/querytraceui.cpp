#include "querytraceui.h"
#include "ui_querytraceui.h"

#include <QtDebug>
#include "include.h"
#include "global.h"
#include "xdata.h"

QueryTraceUI::QueryTraceUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryTraceUI)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);


    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/back2.png")));
    setPalette(palette);
    setAutoFillBackground(true);


    uiIndex=0;
    uioldIndex=0;

    ui->pushButton->setFocus();


    while(uiIndex<TRANS_MAXNB)
    {
        XDATA_Read_Vaild_File(DataSaveTransInfo);
        if(DataSave0.TransInfoData.auiTransIndex[uiIndex]==1)
        {
            memset(&NormalTransData,0,sizeof(NORMALTRANS));
            XDATA_Read_SaveTrans_File(uiIndex);
            setData();
            break;
        }
        else
            uiIndex++;
    }
}

void QueryTraceUI::setData()
{
    qDebug()<<"uiIndex============uioldIndex=========="<<uiIndex<<uioldIndex;

    uchar aucBuf[31];
    uchar cardnumBuf[31];
    uchar transeDate[31];

    memset(aucBuf,0,sizeof(aucBuf));
    memset(cardnumBuf,0,sizeof(cardnumBuf));
    memset(transeDate,0,sizeof(transeDate));

    aucBuf[30] = '\0';
    cardnumBuf[30] = '\0';
    transeDate[30] = '\0';


    bcd_asc(&cardnumBuf[0],NormalTransData.aucUserCardNo ,16);

    Uart_Printf("aucPrintBuf = %s\n",cardnumBuf);

    ui->label_4->setText(QString((char*)cardnumBuf));

    Uart_Printf("NORMALTRANS_ucTransType = %02x\n",NORMALTRANS_ucTransType);
    switch(NORMALTRANS_ucTransType)
    {
        case TRANS_S_PURCHASE:
          memcpy(aucBuf,(unsigned char*)"消费",8);

          break;
        case TRANS_S_RETURNCARD:
          memcpy(aucBuf,(unsigned char*)"回收消费",16);
          break;
        case TRANS_S_ONLINEPURCHASE:
          if(NormalTransData.ucReturnflag==true)
              memcpy(aucBuf,(unsigned char*)"联机回收",16);
          else
              memcpy(aucBuf,(unsigned char*)"联机消费",16);
          break;
        case TRANS_S_PURCHASE_P:
          memcpy(aucBuf,(unsigned char*)"汇点消费",16);
          break;
        case TRANS_S_RETURNCARD_P:
          memcpy(aucBuf,(unsigned char*)"汇点回收",16);
          break;
        case TRANS_S_ONLINEPURCHASE_P:
          memcpy(aucBuf,(unsigned char*)"积点消费",16);
          break;
        case TRANS_S_REFUND:
          memcpy(aucBuf,(unsigned char*)"退货",8);
          break;
        case TRANS_S_VOID:
          memcpy(aucBuf,(unsigned char*)"撤销",8);
          break;
        case TRANS_S_EDLOAD:
          memcpy(aucBuf,(unsigned char*)"回收充值",16);
          break;
        case TRANS_S_RESETCARD:
          memcpy(aucBuf,(unsigned char*)"清余额",12);
          break;
        case TRANS_S_PRELOAD:
          memcpy(aucBuf,(unsigned char*)"预充值",12);
          break;
        case TRANS_S_SALE:
          memcpy(aucBuf,(unsigned char*)"卡激活",12);
          break;
        case TRANS_S_SETTLE_ONE:
          memcpy(aucBuf,(unsigned char*)"结算",8);
          break;
        case TRANS_S_CHANGEEXP:
          memcpy(aucBuf,(unsigned char*)"续期",8);
          break;
        case TRANS_S_PAY:
          memcpy(aucBuf,(unsigned char*)"缴费",8);
          break;
        case TRANS_S_PAYONLINE:
          memcpy(aucBuf,(unsigned char*)"联机缴费",16);
          break;
        case TRANS_S_MOBILECHARGE:
          memcpy(aucBuf,(unsigned char*)"脱机扣款",16);
          break;
        case TRANS_S_MOBILECHARGEONLINE:
          memcpy(aucBuf,(unsigned char*)"联机扣款",16);
          break;
        case TRANS_S_BARCODE:
          memcpy(aucBuf,(unsigned char*)"条码",8);
          break;
        case TRANS_S_PREAUTH:
          memcpy(aucBuf,(unsigned char*)"预授权",12);
          break;
        case TRANS_S_PREAUTHVOID:
          memcpy(aucBuf,(unsigned char*)"授权撤销",16);
          break;
        case TRANS_S_PREAUTHFINISH:
          memcpy(aucBuf,(unsigned char*)"授权完成",16);
          break;
        case TRANS_S_PREAUTHFINISHVOID:
          memcpy(aucBuf,(unsigned char*)"完成撤销",16);
          break;
        case TRANS_S_VOIDPAYONLINE:
          memcpy(aucBuf,(unsigned char*)"缴费撤销",16);
          break;
        case TRANS_S_LOADONLINE:
          memcpy(aucBuf,(unsigned char*)"联机充值",16);
          break;
        case TRANS_S_TRANSPURCHASE:
          memcpy(aucBuf,(unsigned char*)"移资扣款",16);
          break;
        case TRANS_S_TRANSFERRETURN:
          memcpy(aucBuf,(unsigned char*)"移资回收",16);
          break;
        case TRANS_S_TRANSFER:
          memcpy(aucBuf,(unsigned char*)"移资充值",16);
          break;
        case TRANS_S_EXPPURCHASE:
          memcpy(aucBuf,(unsigned char*)"过期消费",16);
          break;
        case TRANS_S_REFAPL:
          memcpy(aucBuf,(unsigned char*)"退货申请",16);
          break;
        case TRANS_S_VOIDREFAPL:
          memcpy(aucBuf,(unsigned char*)"撤销申请",16);
          break;
        case TRANS_S_CASHLOAD:
          memcpy(aucBuf,(unsigned char*)"现金充值",16);
          break;
        case TRANS_S_ZSHPURCHASE:
          memcpy(aucBuf,(unsigned char*)"加油扣款",16);
          break;
        case TRANS_S_PTCPURCHASE:
          memcpy(aucBuf,(unsigned char*)"公交扣款",16);
          break;
        case TRANS_S_POINTGIFT:
          memcpy(aucBuf,(unsigned char*)"积分赠送",16);
          break;
        case TRANS_S_CONVERT:
          memcpy(aucBuf,(unsigned char*)"积分兑换",16);
          break;
        case TRANS_S_VOIDCONVERT:
          memcpy(aucBuf,(unsigned char*)"兑换撤销",16);
          break;
        default:
          break;
    }

    Uart_Printf("aucBuf交易类型: %s\n",aucBuf);

    ui->label_5->setText(QString((char*)aucBuf));

    memset(aucBuf,0,sizeof(aucBuf));
    long_asc(&aucBuf[0],6,&NormalTransData.ulTraceNumber);
    ui->label_8->setText(QString((char*)aucBuf));

    UTIL_Form_Montant(transeDate, NormalTransData.ulAmount,2);
    Uart_Printf("NormalTransData.ulAmoun = %ld\n",NormalTransData.ulAmount);
    ui->label_6->setText(QString((char*)transeDate));

}
QueryTraceUI::~QueryTraceUI()
{
    delete ui;
}

void QueryTraceUI::on_pushButton_clicked()
{
     uioldIndex = uiIndex;//10

     if(uiIndex == 0)
         return;
     while((--uiIndex)>=0)
     {
          qDebug()<<"last uiIndex******************"<<uiIndex;
         XDATA_Read_Vaild_File(DataSaveTransInfo);
         if(DataSave0.TransInfoData.auiTransIndex[uiIndex]==1)
         {
             memset(&NormalTransData,0,sizeof(NORMALTRANS));
             XDATA_Read_SaveTrans_File(uiIndex);

             setData();
             return;
         }

     }

     uiIndex = uioldIndex;
}

void QueryTraceUI::on_pushButton_3_clicked()//NEXT
{
    uioldIndex = uiIndex;//10

    while((++uiIndex)<TRANS_MAXNB)
    {
        qDebug()<<"next uiIndex******************"<<uiIndex;
        XDATA_Read_Vaild_File(DataSaveTransInfo);
        if(DataSave0.TransInfoData.auiTransIndex[uiIndex]==1)
        {
            memset(&NormalTransData,0,sizeof(NORMALTRANS));
            XDATA_Read_SaveTrans_File(uiIndex);

            setData();
            return;
        }

    }

    uiIndex = uioldIndex;
}

void QueryTraceUI::on_pushButton_2_clicked()
{
    destroy();
}
}
