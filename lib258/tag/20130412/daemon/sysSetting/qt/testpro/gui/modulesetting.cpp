#include "modulesetting.h"
#include "ui_modulesetting.h"
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QRadioButton>

ModuleSetting::ModuleSetting(QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::ModuleSetting)
{
    ui->setupUi(this);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    setAttribute(Qt::WA_DeleteOnClose);

    for(int i=0;i<MAXCOUNT;i++){
        label[i] = 0;
        box[i] = 0;
        box2[i] = 0;
    }
    ui->pushButton->setFocus();

    getValue();
}

void ModuleSetting::readFile()
{
    QFile file("/daemon/daemon.sh");
    if(!file.open(QIODevice::ReadOnly| QIODevice::Text))
       { printf("read file failed\n");
           return;
    }


    QTextStream in(&file);
    while(!in.atEnd()){
        strList.append(in.readLine());
    }

    for(int i = 0; i<strList.count()-1;i++)
     {
printf("aaaaaaaaaaa\n");
        box2[i] = new QCheckBox(ui->scrollArea);
        box2[i]->setGeometry(190,30+i*40,30,20);
        if(strList[i+1].startsWith("#"))
            box2[i]->setChecked(true);
     }

    QMap<QString,int> set;
    qDebug()<<"strList"<<strList;
    splitStr(set,strList," /daemon/",">/dev/null 2>&1 &");


    QStringList keys = set.keys();
    qDebug() << "keys.count() = " << keys.count();
    int count = keys.count();
   for(int i = 0; i<count;i++)
    {
       label[i]= new QLabel(keys.at(i),ui->scrollArea);
       box[i] = new QCheckBox(ui->scrollArea);
       label[i]->setGeometry(10,30+i*40,120,30);
       box[i]->setGeometry(140,30+i*40,30,20);
       if(set.value(keys[i]))
            box[i]->setChecked(true);

    }

   file.close();
}


void ModuleSetting::writeFile()
{

    QFile file("/daemon/daemon.sh");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate| QIODevice::Text))
           return;

    file.write("#!/bin/sh\n");

    int count = strList.count();
    for(int i = 0; i< count - 1;i++)
       {
           QString str;
           if(box2[i]->isChecked())
               str = QString("#/daemon/"+label[i]->text().left(label[i]->text().indexOf(" "))+(box[i]->isChecked()?".dm.1.0.0  &\n":".dm.1.0.0 >/dev/null 2>&1 &\n"));
           else
               str = QString(" /daemon/"+label[i]->text().left(label[i]->text().indexOf(" "))+(box[i]->isChecked()?".dm.1.0.0  &\n":".dm.1.0.0 >/dev/null 2>&1 &\n"));

           file.write(str.toAscii().data());
       }

    file.close();

}


void ModuleSetting::splitStr(QMap<QString,int>& set,QStringList list,QString beginStr,QString flagStr )
{
    qDebug()<<"list.count()"<<list.count();

    for(int i = 1;i<list.count();i++)
    {
        int dotidx = list.at(i).indexOf(".");
        QString temp = list.at(i).left(dotidx);
        QString str = temp.remove(0,beginStr.length());
        qDebug()<<"str"<<str;

        QString s1 = list.at(i).right(list.at(i).length()-str.length()-beginStr.length()-1);

        qDebug()<<"s1"<<s1;
         int d1 = s1.indexOf(" ");
         QString t1 = s1.left(d1);
         t1.remove("dm.");
         qDebug()<<"t1 d1"<<t1<<d1;



        if(list.at(i).endsWith(">/dev/null 2>&1 &"))
            set[str+" "+t1]=0;
        else
            set[str+" "+t1]=1;

    }

}


void ModuleSetting::getValue()
{
    QSettings set("/daemon/setting.ini",QSettings::IniFormat);

    set.beginGroup("MODULE_SETTING");

    keyList = set.allKeys();

    {
    if(keyList.count()==0)
        return;
    QString temp1 = keyList.at(0);
    temp1.remove(0,3);
    ui->label->setText(temp1);

    QString value1 = set.value(keyList.at(0)).toString();
    int idx1 = value1.indexOf("#");
    QString str1 = value1.left(idx1).simplified();
    ui->lineEdit->setText(str1);

    qDebug()<<"value1 "<<value1<<value1.size()-idx1;
    noteList.append(value1.right(value1.size()-idx1));

    }

    {//two
     if(keyList.count()<2)
         return;
    QString temp2 = keyList.at(1);
    temp2.remove(0,3);
    ui->label_2->setText(temp2);

    QString value2 = set.value(keyList.at(1)).toString();
    int idx2 = value2.indexOf("#");
    QString str2 = value2.left(idx2).simplified();
    ui->lineEdit_2->setText(str2);

    noteList.append(value2.right(value2.size()-idx2));

    }

    set.endGroup();

    readFile();
}


void ModuleSetting::on_pushButton_2_clicked()//ok
{
    setValue();
    system("sync");
    close();
}



void ModuleSetting::setValue()
{
    QSettings set("/daemon/setting.ini",QSettings::IniFormat);
    set.beginGroup("MODULE_SETTING");

      if((!keyList.count())||(!noteList.count()))
        return;
      qDebug()<<"~~~~~~~~~~~~~~~~~~"<<ui->lineEdit->text()+" "+noteList.at(0);
      set.setValue(keyList.at(0),ui->lineEdit->text()+" "+noteList.at(0));

      if((keyList.count()<2)||(noteList.count()<2))
        return;
      set.setValue(keyList.at(1),ui->lineEdit_2->text()+" "+noteList.at(1));

      set.endGroup();

      writeFile();


}

int ModuleSetting::analysisDesc(QString desc, QString *brief, QString *valueList)
{
    int type = 0;
    //去掉两端空格
    QString strTemp = desc;
    QString valueTemp;
    QRegExp regExp("[0-9]{1,2}-");
    //判断第一个字符是否为#号
    if(strTemp.left(1).compare("#") != 0)
        return -1;
    //将#号去掉
    strTemp = strTemp.right(strTemp.size() - 1);
    //获取类型，即#号后的数值
    type = strTemp.left(strTemp.indexOf(" ")).toInt();
    //将已经获取的字符串去掉
    if(strTemp.contains(" "))
        strTemp = strTemp.right(strTemp.size() - strTemp.indexOf(" ") -1);
    else
        strTemp = "";
    //判断是否有|
    if(strTemp.contains("|"))
    {
        //|前的内容都是简介
        *brief = strTemp.left(strTemp.indexOf("|") - 1);
        //去掉|前的内容
        strTemp = strTemp.right(strTemp.size() - strTemp.indexOf("|") - 1);
        //循环判断是否有正则表达式"[0-9]{1,2}-"
        while(strTemp.contains(regExp))
        {
            //去掉两端的空格
            strTemp = strTemp.simplified();
            //判断中间是否有空格，多个不同的值需要用空格隔开
            if(strTemp.count(regExp) > 1)
            {
                //截取-后和空格前的内容
                valueTemp = strTemp.left(strTemp.indexOf(regExp,3) - 1);
                *valueList += valueTemp + ";";
                //去掉已经截取的信息
                strTemp = strTemp.right(strTemp.size() - strTemp.indexOf(regExp,3));
            }
            else
            {
                //没有空格说明是最后一个值，取值后退出
                valueTemp = strTemp;
                *valueList += valueTemp;
                break;
            }
        }
    }
    //没有|时后面内容都是简介
    else
        *brief = strTemp;

    return type;

}


ModuleSetting::~ModuleSetting()
{
    keyList.clear();
    noteList.clear();
    delete ui;
}


void ModuleSetting::on_pushButton_clicked()
{
    close();
}

void ModuleSetting::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()){
    case Qt::Key_Escape:
    case Qt::Key_Cancel:
        close();
        break;
    }
}
