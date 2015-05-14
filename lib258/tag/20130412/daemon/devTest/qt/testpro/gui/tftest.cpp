#include "tftest.h"
extern "C"{
#include<fcntl.h>
#include "massstorage.h"
#include "sand_debug.h"
}
extern void _print(char *fmt,...);

TfTest::TfTest(int flag,QWidget *parent) : BaseForm(tr(" "), parent)
{
    //qDebug()<<"in tftest***************"<<flag;
    textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);

    btnOk->show();
    btnOk->setFocus();
    btnlayout->addWidget(btnOk);

    flags = flag;
    if(flags==1)
        title->setText("USB TEST");
    else
        title->setText("TF TEST");

    textEdit->setText("PRESS OK TO START TEST!");

    connect(btnOk,SIGNAL(clicked()),this,SLOT(getResult()));
}

TfTest::~TfTest()
{

}

void TfTest::getResult()
{
    btnOk->setEnabled(false);

    int ret;
    unsigned long fileSize;
    unsigned char pdata[100];
    char *f="/asd/mine.txt";

    textEdit->clear();
    //检测TF卡是否挂载成功
    if(flags ==1)
        ret=Os__massstorage_waitInsert(TYPE_USB,1);
    else
        ret=Os__massstorage_waitInsert(TYPE_TF,1);

        _print("waiting %d\n",ret);
        if(flags==1){
            if(ret==0)
                textEdit->setText("USB MOUNT SUCCESS!");
            else{
                textEdit->setText("USB MOUNT FAILED,PRESS OK TO RETRY!");//  usb mount failed! pess ok to retry");
                btnOk->setEnabled(true);

                return;
            }
        }
        else
        {
            if(ret==0)
                textEdit->setText("TF MOUNT SUCCESS!");
            else{
                textEdit->setText("TF MOUNT FAILED, PRESS OK TO RETRY!");//tf mount failed! pess ok to retry");
                btnOk->setEnabled(true);

                return;
            }
        }


        //在TF卡中打开一个文件
        ret=Os__massstorage_selectFile(f);
        _print("openfile %d\n",ret);
        if(ret==0)
            textEdit->append(QString("OPEN %1 SUCCESS!").arg(f));
        else
            textEdit->append(QString("OPEN %1 FAILED!").arg(f));

        //读取打开的文件
        memset(pdata,0,sizeof(pdata));
        Os__massstorage_getSize(&fileSize);
        ret=Os__massstorage_read(0,pdata,&fileSize);
        _print("read [%d] \n [length %ld] %s\n",ret,fileSize,pdata);
        if(ret==0)
            textEdit->append(QString("READ %1 SUCCESS!").arg(QString((char*)pdata)));
        else
            textEdit->append(QString("READ %1 FAILED!").arg(QString((char*)pdata)));

        //写打开的文件
        ret=Os__massstorage_write(0,(uchar*)"HELLO WORLD",11);
        _print("write [%d]\n",ret);
        if(ret==0)
            textEdit->append("WRITE FILE SUCCESS!");
        else
            textEdit->append("WRITE FILE FAILED!");

        //读打开的文件
        memset(pdata,0,sizeof(pdata));
        Os__massstorage_getSize(&fileSize);
        ret=Os__massstorage_read(0,pdata,&fileSize);
        _print("read [%d] \n [length %ld] %s\n",ret,fileSize,pdata);

        if(ret==0)
            textEdit->append(QString("READ %1 SUCCESS!").arg(QString((char*)pdata)));
        else
            textEdit->append(QString("READ %1 FAILED!").arg(QString((char*)pdata)));

        //追加打开的文件
        ret=Os__massstorage_append((uchar*)" !",2);
        _print("append %d\n",ret);
        if(ret==0)
            textEdit->append("APPEND  ! SUCCESS");
        else
            textEdit->append("APPEND ! FAILED!");

        //读打开的文件
        memset(pdata,0,sizeof(pdata));
        Os__massstorage_getSize(&fileSize);
        ret=Os__massstorage_read(0,pdata,&fileSize);
        _print("read [%d] \n [length %ld] %s\n",ret,fileSize,pdata);

        if(ret==0)
            textEdit->append(QString("READ %1 SUCCESS!").arg(QString((char*)pdata)));
        else
            textEdit->append(QString("READ %1 FAILDE!").arg(QString((char*)pdata)));

        //关闭打开的文件
        ret=Os__massstorage_closeFile();
        _print("close %d\n",ret);

        if(ret==0)
            textEdit->append("CLOSE  FILE SUCCESS!");
        else
            textEdit->append("CLOSE FILE FAILED!");

//        //删除创建的文件,需要open之后做
//        ret=Os__massstorage_eraseFile();
//        _print("delete %d\n",ret);

        btnOk->setEnabled(true);

}


//void TfTest::keyPressEvent(QKeyEvent *e)
//{
//    switch(e->key())
//    {
//    case Qt::Key_Enter:
//        getResult();
//        break;
//    case Qt::Key_Escape:
//        close();
//        break;
//    }
//}
