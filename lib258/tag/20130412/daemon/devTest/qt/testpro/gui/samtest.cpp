#include "samtest.h"
#include <QtDebug>
#include "test/smart.h"

extern "C"{
#include "tools.h"

#include "sand_debug.h"
#include "test/test.h"
}

extern void _print(char *fmt,...);

#define OSERRMSG_ICCAsyncReset 		"icc async reset"
#define OSMSG_TestICCATR 			"icc ATR test"
#define OSERRMSG_SAMSelectFile		"sam card select file error"

extern ISO7816OUT iso7816out;

SamTest::SamTest(QWidget *parent) :
    BaseForm("SAM TEST",parent)
{
    btnOk->show();
    btnlayout->addWidget(btnOk,1,Qt::AlignBottom);

    QLabel *labelType = new QLabel(tr("SAM TYPE:"));

    comboBox = new QComboBox;
    QStringList items;
    items<<"1.EMV"<<"2.7816"<<"3.MPEV5"<<"4.MONDEX"<<"5.GJ ISAM"<<"6.GJ PSAM"<<"7.Shebao"<<"8.Qingdaotong"<<"9.NEW";
    comboBox->addItems(items);
    comboBox->setCurrentIndex(0);
    comboBox->setFocus();

     QHBoxLayout *samTypeLayout = new QHBoxLayout;
     samTypeLayout->addWidget(labelType);
     samTypeLayout->addWidget(comboBox);


    QLabel *labelInfo = new QLabel(tr("SAM NUM:"));
    QHBoxLayout *samNumLayout = new QHBoxLayout;

    radBtn1 = new QRadioButton("1");
    radBtn2 = new QRadioButton("2");
    radBtn3 = new QRadioButton("3");
    samNumLayout->addWidget(labelInfo);
    samNumLayout->addWidget(radBtn1);
    samNumLayout->addWidget(radBtn2);
    samNumLayout->addWidget(radBtn3);

    textEdit = new QTextEdit;
    textEdit->setTextColor(QColor(0,0,255));
    textEdit->setReadOnly(true);

    layout->addLayout(samTypeLayout);
    layout->addLayout(samNumLayout);
    layout->addWidget(textEdit);


    connect(btnOk,SIGNAL(clicked()),this,SLOT(getResult()));


    radBtn1->setChecked(true);

}

SamTest::~SamTest()
{

}



void SamTest::getResult()
{
    btnOk->setEnabled(false);

    unsigned short uiI;
    unsigned short uiLen;
    unsigned char ucResult = -1;
    unsigned char aucData[30];
    unsigned char ucRow;
    unsigned char ucLen;
    unsigned char aucDispBuf[21];
    unsigned char ucReader,ucCardStandard;
    unsigned char ucFlag=0;
    unsigned char aucBuf[200];
    int f=0; //0-其他 1-同步卡 2-异步卡
    ICC_ORDER icc_order;
    ICC_ANSWER *picc_answer;


    ucCardStandard = comboBox->currentIndex();

    textEdit->clear();

    if(radBtn1->isChecked())
        ucReader = 0x31;
    else if(radBtn2->isChecked())
        ucReader = 0x32;
    else
        ucReader = 0x33;

    memset(aucDispBuf,0,sizeof(aucDispBuf));
    aucDispBuf[0] = ucReader;

    ucReader = ucReader - '0';
    uiLen = sizeof(aucData);
    memset(aucData,0,sizeof(aucData));

    ucResult = SMART_NewReset(ucReader,ucCardStandard,aucData,&uiLen);

    if( ucResult != SMART_ASYNC)
    {
        f=1;
        _print(" NOT SMART_ASYNC\r\n");
        memset(aucDispBuf,0x20,sizeof(aucDispBuf));
        aucDispBuf[20] = 0;
        memcpy(aucDispBuf,OSERRMSG_ICCAsyncReset,strlen((char *)OSERRMSG_ICCAsyncReset));
        sprintf((char *)&aucDispBuf[18],"%02X",ucResult);

        textEdit->append((char*)aucDispBuf);

    }
    else
    {
        f=2;
        _print(" SMART_ASYNC  %s\n",(char*)aucDispBuf);
        textEdit->append("SAM["+QString((char*)aucDispBuf)+"] ATR:");

        ucLen = 0;
        memset(aucDispBuf,0,sizeof(aucDispBuf));


        for(uiI=0;uiI<uiLen;uiI++)
        {
            sprintf((char *)&aucDispBuf[ucLen],"%02X",aucData[uiI]);
            ucLen += 2;
        }

        textEdit->append((char*)aucDispBuf);
    }



    //异步卡才做这个流程

    if(f==2){
        textEdit->append("");
        textEdit->append("SELECT DDF01:");
        ucResult = PBOC_selectfile(1, ucReader, (unsigned char *)"1PAY.SYS.DDF01",14);

    if( !ucResult )
    {
        ucLen = 0;
        memset(aucDispBuf,0,sizeof(aucDispBuf));
        for(uiI=0;uiI<iso7816out.uiLen;uiI++)
        {
            sprintf((char *)&aucDispBuf[ucLen],"%02X",iso7816out.aucData[uiI]);
            ucLen += 2;
        }
        textEdit->append((char*)aucDispBuf);

    }
    else
    {
        _print("ERR %s\n",OSERRMSG_SAMSelectFile);
         textEdit->append((char*)OSERRMSG_SAMSelectFile);
    }
    }

    textEdit->append("");
    textEdit->append("REMOVE CARD");

    icc_order.ptout = aucBuf;
    icc_order.order = POWER_OFF;
    picc_answer = Os__ICC_command(ucReader, &icc_order);

    btnOk->setEnabled(true);

}



void SamTest::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_1:
        radBtn1->setChecked(true);
        break;
    case Qt::Key_2:
        radBtn2->setChecked(true);
        break;
    case Qt::Key_3:
        radBtn3->setChecked(true);
        break;
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Enter:
            getResult();
        break;

    }
}


