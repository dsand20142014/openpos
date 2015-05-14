#include "pinyindialog.h"
#include "ui_pinyindialog.h"
#include "pinyinime.h"
#include "qimeframe.h"
#include <QDebug>

using namespace ime_pinyin;

PinyinDialog::PinyinDialog(IMFrame* im)
  :QDialog(0, Qt::Tool| Qt::WindowStaysOnTopHint| Qt::FramelessWindowHint),
    ui(new Ui::PinyinDialog)
{
    imf=im;
    /** 初始化谷歌拼音 **/
    if(true==im_open_decoder("./dict_pinyin.dat","./mydict.dat"))
    {
            //qDebug("im_open_decoder success!\n");
    }
    else
    {
            //qDebug("im_open_decoder failure!\n");
    }

    timer=new QTimer(this);
    timer->setSingleShot(true);

    ui->setupUi(this);

    label[0]=ui->label;
    label[1]=ui->label_2;
    label[2]=ui->label_3;
    label[3]=ui->label_4;
    label[4]=ui->label_5;

    signFlag = false;

    connect(im,SIGNAL(imeTypeChanged(ImeType)),this,SLOT(changeImeType(ImeType)));

    connect(ui->lineEdit,SIGNAL(textChanged(QString)),this,SLOT(getCandidate(QString)));

    connect(ui->label,SIGNAL(clicked()),this,SLOT(label1clicked()));
    connect(ui->label_2,SIGNAL(clicked()),this,SLOT(label2clicked()));
    connect(ui->label_3,SIGNAL(clicked()),this,SLOT(label3clicked()));
    connect(ui->label_4,SIGNAL(clicked()),this,SLOT(label4clicked()));
    connect(ui->label_5,SIGNAL(clicked()),this,SLOT(label5clicked()));
}

void PinyinDialog::changeImeType(ImeType type)
{
    imeType_temp = type;
    if(imeType_temp==PINYIN){
        signList.clear();
        signList<<tr("，")<<tr("。")<<tr("？")<<tr("！")<<tr("、")<<tr("：")<<tr("......")<<tr("＂")<<tr("＂")<<tr("；")<<tr("（")<<tr("）")<<tr("《")<<tr("》")<<tr("～")<<tr("＆")<<tr("＃")<<tr("＊"
               )<<tr("【")<<tr("】")<<tr("￥")<<tr("＄")<<tr("％")<<tr("｛")<<tr("｝")<<tr("『")<<tr("』")<<tr("［")<<tr("］")<<tr("＋")<<tr("－")<<tr("、")<<tr("／");//29
    }
    else{
        signList.clear();
        signList<<"."<<","<<"?"<<"!"<<"'"<<":"<<"..."<<"@"<<";"<<"\""<<"/"<<"("<<")"<<"_"<<"-"<<"+"<<"="<<"`"<<"~"<<"^"<<"#"<<"*"
               <<"%"<<"&"<<"\\"<<"["<<"]"<<"<"<<">"<<"{"<<"}"<<"|"<<"$";//33
    }

    showCandidate(0);
}

PinyinDialog::~PinyinDialog()
{
    delete ui;
    im_close_decoder();
}

void PinyinDialog::sendMyContent(const QString& newcontent)
{
    mycontent += newcontent;
    ui->lineEdit->setText(oldcontent+newcontent);
}

void PinyinDialog::confirmMyContent()
{
    //ui->lineEdit->setText(oldcontent+newcontent);
    mycontent.clear();
}

bool PinyinDialog::keyPush(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat)
{
    (void)unicode;
    (void)modifiers;
    (void)autoRepeat;
    static int pushNum;
    static int preKeyCode;

    if(isPress)
    {
        if(preKeyCode!=keycode)
        {
            timer->stop();
            confirmMyContent();
            preKeyCode=keycode;
        }
        if(!timer->isActive())
        {
            timer->setInterval(1000);
            timer->start();
            connect(timer,SIGNAL(timeout()),this,SLOT(confirmMyContent()));
            pushNum=0;


             oldcontent=ui->lineEdit->text();
        }
        else
        {
            timer->setInterval(1000);
            pushNum++;
        }
        switch(keycode)
        {
        case Qt::Key_0:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%3)
            {
            case 0:
                mycontent=" ";
                break;
            case 1:
                mycontent="*";
                break;
            case 2:
                mycontent="0";
                break;
            }
            break;
        case Qt::Key_1:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%3)
            {
            case 0:
                mycontent="q";
                break;
            case 1:
                mycontent="z";
                break;
            case 2:
                mycontent="1";
                break;
            }
            break;
        case Qt::Key_2:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%4)
            {
            case 0:
                mycontent="a";
                break;
            case 1:
                mycontent="b";
                break;
            case 2:
                mycontent="c";
                break;
            case 3:
                mycontent="2";
                break;
            }
            break;
        case Qt::Key_3:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%4)
            {
            case 0:
                mycontent="d";
                break;
            case 1:
                mycontent="e";
                break;
            case 2:
                mycontent="f";
                break;
            case 3:
                mycontent="3";
                break;
            }
            break;
        case Qt::Key_4:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%4)
            {
            case 0:
                mycontent="g";
                break;
            case 1:
                mycontent="h";
                break;
            case 2:
                mycontent="i";
                break;
            case 3:
                mycontent="4";
                break;
            }
            break;
        case Qt::Key_5:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%4)
            {
            case 0:
                mycontent="j";
                break;
            case 1:
                mycontent="k";
                break;
            case 2:
                mycontent="l";
                break;
            case 3:
                mycontent="5";
                break;
            }
            break;
        case Qt::Key_6:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%4)
            {
            case 0:
                mycontent="m";
                break;
            case 1:
                mycontent="n";
                break;
            case 2:
                mycontent="o";
                break;
            case 3:
                mycontent="6";
                break;
            }
            break;
        case Qt::Key_7:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%4)
            {
            case 0:
                mycontent="p";
                break;
            case 1:
                mycontent="r";
                break;
            case 2:
                mycontent="s";
                break;
            case 3:
                mycontent="7";
                break;
            }
            break;
        case Qt::Key_8:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%4)
            {
            case 0:
                mycontent="t";
                break;
            case 1:
                mycontent="u";
                break;
            case 2:
                mycontent="v";
                break;
            case 3:
                mycontent="8";
                break;
            }
            break;
        case Qt::Key_9:
            show();
            if(imeType_temp!=PINYIN)
                 return true;
            signFlag = false;
            switch(pushNum%4)
            {
            case 0:
                mycontent="w";
                break;
            case 1:
                mycontent="x";
                break;
            case 2:
                mycontent="y";
                break;
            case 3:
                mycontent="9";
                break;
            }
            break;
        case Qt::Key_F12://sign
            show();
            signFlag = true;
            if(pushNum<signList.count())
                 mycontent = signList.at(pushNum);
            oldcontent.clear();
           break;

        case Qt::Key_F3:
            on_pageup_clicked();
            break;
        case Qt::Key_F4:
            on_pagedown_clicked();
            break;

        case Qt::Key_Enter:
           if(ui->label->text().length()>0)
            {
                imf->sendComString(ui->label->text());
                ui->lineEdit->clear();
                hide();
                return true;
            }
           else if(ui->label->text().length()==0&&ui->lineEdit->text().length()!=0)
           {
                imf->sendComString(ui->lineEdit->text());
                ui->lineEdit->clear();
                hide();
                return true;
           }
           else
            {
                return false;
            }
            break;
        case Qt::Key_Backspace:
            if(ui->lineEdit->text().length()>0)
            {
                timer->stop();
                confirmMyContent();
                ui->lineEdit->backspace();
                return true;
            }
            else
            {
                return false;
            }
            break;
        default:
            timer->stop();
            return false;
            break;
        }
        ui->lineEdit->setText(oldcontent+mycontent);
        return true;
    }
    else
    {
        return false;
    }
}

void PinyinDialog::getCandidate(QString pinyinStr)
{
     page=0;
     if(signFlag)
        cadidateNum = signList.count();
     else
         cadidateNum=im_search(pinyinStr.toAscii(),pinyinStr.length());
    if(cadidateNum%5==0)
    {
        maxPage=cadidateNum/5;
    }
    else
    {
        maxPage=cadidateNum/5+1;
    }
    emit alreadyGetCandidate(cadidateNum);
    showCandidate(0);
}

void PinyinDialog::showCandidate(int pageIndex)
{
    int i;
    char16 hanzi[100];
    char16 * s;
    QString cadidateStr;
    for(i=0;i<5;i++)
    {
        label[i]->clear();
    }


    for(i=0;i<5;i++)
    {
        if(pageIndex*5+i>=cadidateNum)
        {
            break;
        }
        else
        {
            if(signFlag){
                if(!ui->lineEdit->text().isEmpty())
                     label[i]->setText(signList[page*5+i]);
                continue;
            }
            else{
                s= im_get_candidate(pageIndex*5+i, hanzi,sizeof(hanzi));
                if(s==NULL)
                {
                    //qDebug("im_get_candidate err.\n");
                }
                else
                {
                    cadidateStr=QString::fromUtf16(s);
                    label[i]->setText(cadidateStr);
                }
            }
        }
    }
    if(pageIndex==0)
    {
            cadidateStr=label[0]->text();
            if(cadidateNum==1)
            {
                imf->sendComString(cadidateStr);
                ui->lineEdit->clear();
                hide();
            }
            else
            {
                imf->sendPreString(cadidateStr);
            }

    }
}

void PinyinDialog::on_pagedown_clicked()
{
    page++;
    if(page>=maxPage)
    {
        page=maxPage-1;
        return;
    }
    else
    {
        showCandidate(page);
    }
}



void PinyinDialog::on_pageup_clicked()
{
    page--;
    if(page<0)
    {
        page=0;
        return;
    }
    else
    {
        showCandidate(page);
    }
}

void PinyinDialog::label1clicked()
{
    if(!ui->label->text().isEmpty())
    {
         if(signFlag)
         {
             imf->sendComString(label[0]->text());
             ui->lineEdit->clear();
             hide();
             return;
         }
        cadidateNum=im_choose(page*5+0);
        page=0;

        if(cadidateNum%5==0)
        {
            maxPage=cadidateNum/5;
        }
        else
        {
            maxPage=cadidateNum/5+1;
        }
        emit alreadyGetCandidate(cadidateNum);
        showCandidate(0);
    }

}

void PinyinDialog::label2clicked()
{
    if(!ui->label_2->text().isEmpty())
    {
        if(signFlag)
        {
            imf->sendComString(label[1]->text());
            ui->lineEdit->clear();
            hide();
            return;
        }
        cadidateNum=im_choose(page*5+1);
        page=0;
        if(cadidateNum%5==0)
        {
            maxPage=cadidateNum/5;
        }
        else
        {
            maxPage=cadidateNum/5+1;
        }
        emit alreadyGetCandidate(cadidateNum);
        showCandidate(0);
    }
}

void PinyinDialog::label3clicked()
{
    if(!ui->label_3->text().isEmpty())
    {
        if(signFlag)
        {
            imf->sendComString(label[2]->text());
            ui->lineEdit->clear();
            hide();
            return;
        }
        cadidateNum=im_choose(page*5+2);
        page=0;
        if(cadidateNum%5==0)
        {
            maxPage=cadidateNum/5;
        }
        else
        {
            maxPage=cadidateNum/5+1;
        }
        emit alreadyGetCandidate(cadidateNum);
        showCandidate(0);
    }
}

void PinyinDialog::label4clicked()
{
    if(!ui->label_4->text().isEmpty())
    {
        if(signFlag)
        {
            imf->sendComString(label[3]->text());
            ui->lineEdit->clear();
            hide();
            return;
        }
        cadidateNum=im_choose(page*5+3);
        page=0;
        if(cadidateNum%5==0)
        {
            maxPage=cadidateNum/5;
        }
        else
        {
            maxPage=cadidateNum/5+1;
        }
        emit alreadyGetCandidate(cadidateNum);
        showCandidate(0);
    }
}

void PinyinDialog::label5clicked()
{
    if(!ui->label_5->text().isEmpty())
    {
        if(signFlag)
        {
            imf->sendComString(label[4]->text());
            ui->lineEdit->clear();
            hide();
            return;
        }
        cadidateNum=im_choose(page*5+4);
        page=0;
        if(cadidateNum%5==0)
        {
            maxPage=cadidateNum/5;
        }
        else
        {
            maxPage=cadidateNum/5+1;
        }
        emit alreadyGetCandidate(cadidateNum);
        showCandidate(0);
    }
}
