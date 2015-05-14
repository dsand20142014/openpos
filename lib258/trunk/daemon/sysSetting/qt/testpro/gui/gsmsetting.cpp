#include "gsmsetting.h"
#include "ui_gsmsetting.h"
#include "test/test.h"
#define TMPFILE         "/tmp/tmpfile"
#define GDIR     "/etc/ppp"
#include "sand_network.h"

GsmSetting::GsmSetting(QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint),
    ui(new Ui::GsmSetting)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    ui->pushButton->setFocus();
   // set = QSettings("./setting.ini",QSettings::IniFormat);

    getValue();
}

void GsmSetting::getValue()
{
    QSettings set("/daemon/setting.ini",QSettings::IniFormat);
    set.beginGroup("GSM_SETTING");

    keyList = set.allKeys();
    qDebug()<<"keyList"<<keyList;

    {//1111111111
    if(keyList.count()==0)
        return;
    QString temp1 = keyList.at(0);
    temp1.remove(0,3);
    ui->label->setText(temp1);

    QString value1 = set.value(keyList.at(0)).toString();
    int idx1 = value1.indexOf("#");
    QString str1 = value1.right(value1.size()-value1.indexOf("#"));
    qDebug()<<"STR1 "<<str1<<value1.size()-value1.indexOf("#");
    QString brief;
    QString valueList;
    analysisDesc(str1,&brief,&valueList);

    QStringList vlist = valueList.split(";");
    qDebug()<<"000vlist"<<vlist;
    for(int i=0;i<vlist.count();i++)
    {
        QString text = vlist.at(i);
        qDebug()<<"333text"<<text;
        ui->comboBox->addItem(text.right(text.size()-2));
    }

    QString simValue = value1.left(idx1).simplified();
    ui->comboBox->setCurrentIndex(simValue.toInt());

    noteList.append(str1);
    }

    {//222222222
    if(keyList.count()<2)
        return;
    QString temp2 = keyList.at(1);
    temp2.remove(0,3);
    ui->label_2->setText(temp2);

    QString value2 = set.value(keyList.at(1)).toString();
    int idx2 = value2.indexOf("#");
    QString str2 = value2.right(value2.size()-value2.indexOf("#"));
    qDebug()<<"STR1 "<<str2<<value2.size()-value2.indexOf("#");
    QString brief;
    QString valueList;
    analysisDesc(str2,&brief,&valueList);

    QStringList vlist = valueList.split(";");
    qDebug()<<"000vlist"<<vlist;
    for(int i=0;i<vlist.count();i++)
    {
        QString text = vlist.at(i);
        qDebug()<<"333text"<<text;
        ui->comboBox_2->addItem(text.right(text.size()-2));
    }

    QString simValue = value2.left(idx2).simplified();
    if(simValue.toInt()==0)
        ui->comboBox_2->setCurrentIndex(1);
    else if(simValue.toInt()==1)
        ui->comboBox_2->setCurrentIndex(0);

    noteList.append(str2);
    }

    {//3333
    if(keyList.count()<3)
        return;
    QString temp3 = keyList.at(2);
    temp3.remove(0,3);
    ui->label_3->setText(temp3);

    QString value3 = set.value(keyList.at(2)).toString();
    int idx3 = value3.indexOf("#");
    QString str3 = value3.right(value3.size()-value3.indexOf("#"));
    qDebug()<<"STR1 "<<str3<<value3.size()-value3.indexOf("#");
    QString brief;
    QString valueList;
    analysisDesc(str3,&brief,&valueList);

    QStringList vlist = valueList.split(";");
    qDebug()<<"000vlist"<<vlist;
    for(int i=0;i<vlist.count();i++)
    {
        QString text = vlist.at(i);
        qDebug()<<"333text"<<text;
        ui->comboBox_3->addItem(text.right(text.size()-2));
    }

    QString simValue = value3.left(idx3).simplified();

    ui->comboBox_3->setCurrentIndex(simValue.toInt()-1);


    noteList.append(str3);
    }

    {//444444444
    if(keyList.count()<4)
        return;
    QString temp4 = keyList.at(3);
    temp4.remove(0,3);
    ui->label_5->setText(temp4);

    QString value4 = set.value(keyList.at(3)).toString();
    int idx4 = value4.indexOf("#");
    QString str4 = value4.left(idx4).simplified();
    ui->lineEdit->setText(str4);

    qDebug()<<"value4 "<<value4<<value4.size()-idx4;
    noteList.append(value4.right(value4.size()-idx4));

    }

    {//55555555
     if(keyList.count()<5)
         return;
    QString temp5 = keyList.at(4);
    temp5.remove(0,3);
    ui->label_6->setText(temp5);

    QString value5 = set.value(keyList.at(4)).toString();
    int idx5 = value5.indexOf("#");
    QString str5 = value5.left(idx5).simplified();
    ui->lineEdit_2->setText(str5);

    noteList.append(value5.right(value5.size()-idx5));

    }

    {//66666666
    if(keyList.count()<6)
        return;
    QString temp6 = keyList.at(5);
    temp6.remove(0,3);
    ui->label_4->setText(temp6);

    QString value6 = set.value(keyList.at(5)).toString();
    int idx6 = value6.indexOf("#");
    QString str6 = value6.right(value6.size()-value6.indexOf("#"));
    qDebug()<<"STR6 "<<str6<<value6.size()-value6.indexOf("#");
    QString brief;
    QString valueList;
    analysisDesc(str6,&brief,&valueList);

    QStringList vlist = valueList.split(";");
    qDebug()<<"000vlist"<<vlist;
    for(int i=0;i<vlist.count();i++)
    {
        QString text = vlist.at(i);
        qDebug()<<"333text"<<text;
        ui->comboBox_4->addItem(text.right(text.size()-2));
    }

    QString simValue = value6.left(idx6).simplified();
    ui->comboBox_4->setCurrentIndex(simValue.toInt()-1);

    noteList.append(str6);
    }
    set.endGroup();
}

void GsmSetting::setValue()
{
    unsigned char buffer[100],cmd[50];


    QSettings set("/daemon/setting.ini",QSettings::IniFormat);
    set.beginGroup("GSM_SETTING");
   // config_gprs_apn(&g_cfg_gsm);
    //QSettings set("./setting.ini",QSettings::IniFormat);

      qDebug()<<"~~~~~~~~~~~~~~~~~~"<<ui->lineEdit->text()+" "+noteList.at(0);
      if((!keyList.count())||(!noteList.count()))
          return;
      set.setValue(keyList.at(0),QString::number(ui->comboBox->currentIndex())+ " "+noteList.at(0) );

      if((keyList.count()<2)||(noteList.count()<2))
          return;
      if(ui->comboBox_2->currentIndex()==0)
        set.setValue(keyList.at(1),QString::number(1)+ " "+noteList.at(1) );
      else if(ui->comboBox_2->currentIndex()==1)
        set.setValue(keyList.at(1),QString::number(0)+ " "+noteList.at(1) );

      if((keyList.count()<3)||(noteList.count()<3))
          return;
      set.setValue(keyList.at(2),QString::number(ui->comboBox_3->currentIndex()+1)+ " "+noteList.at(2) );


      if((keyList.count()<4)||(noteList.count()<4))
          return;
      set.setValue(keyList.at(3),ui->lineEdit->text()+" "+noteList.at(3));

      if((keyList.count()<5)||(noteList.count()<5))
          return;
      set.setValue(keyList.at(4),ui->lineEdit_2->text()+" "+noteList.at(4));

      if((keyList.count()<6)||(noteList.count()<6))
          return;
       set.setValue(keyList.at(5),QString::number(ui->comboBox_4->currentIndex()+1)+ " "+noteList.at(5) );

       set.endGroup();


        uchar ucResult = OSGSM_init();
       config_pap_chap();
       OSGSM_GprsSetAPN((uchar*)ui->comboBox_3->currentText().toAscii().data());

       memset(cmd,0,sizeof(cmd));

       if(ui->comboBox_4->currentIndex()==0)
           strcpy((char*)cmd,"AT+UBANDSEL=850");
       else if(ui->comboBox_4->currentIndex()==1)
           strcpy((char*)cmd,"AT+UBANDSEL=900");
       else if(ui->comboBox_4->currentIndex()==2)
           strcpy((char*)cmd,"AT+UBANDSEL=1800");
       else if(ui->comboBox_4->currentIndex()==3)
           strcpy((char*)cmd,"AT+UBANDSEL=1900");
       else if(ui->comboBox_4->currentIndex()==4)
           strcpy((char*)cmd,"AT+UBANDSEL=850,1900");
       else if(ui->comboBox_4->currentIndex()==5)
           strcpy((char*)cmd,"AT+UBANDSEL=900,1800");

    qDebug()<<"cmd=========="<<cmd;
       memset(buffer,0,sizeof(buffer));
       if (OSGSM_ATCmd((unsigned char*)cmd,buffer,5)!=0x00)
           ui->label_7->setText("set failed");

       memset(buffer,0,sizeof(buffer));

       OSGSM_ATCmd((unsigned char*)"AT+IFC=0,0\r",buffer,5);
       system("/etc/ppp/ppp-off > /dev/null 2>1&");
}



int GsmSetting::config_pap_chap()//set username and password
{
    char command[256];
    char _username[50],_passwd[50],pap[50],chap[50];
    char *_pap = "pap-secrets";
    char *_chap= "chap-secrets";

    memset(pap,0,sizeof(pap));
    sprintf(pap,"%s/%s",GDIR,_pap);
    memset(chap,0,sizeof(chap));
    sprintf(chap,"%s/%s",GDIR,_chap);


    //read ini

    //memcpy
    memset(_username,0,sizeof(_username));
    memset(_passwd,0,sizeof(_passwd));




//    QString value4 = set.value(keyList.at(3)).toString();
//    int idx4 = value4.indexOf("#");
//    QString str4 = value4.left(idx4).simplified();

//    if(str4==NULL)
//        memcpy(_username,"",1);
//    else
//        memcpy(_username,str4.toAscii().data(),str4.length());

    if(ui->lineEdit->text().isNull())
          memcpy(_username,"",1);
    else
          memcpy(_username,ui->lineEdit->text().toAscii().data(),ui->lineEdit->text().length());

//    QString value5 = set.value(keyList.at(4)).toString();
//    int idx5 = value5.indexOf("#");
//    QString str5 = value5.left(idx5).simplified();

//    if(str5==NULL)
//        memcpy(_passwd,"",1);
//    else
//        memcpy(_passwd,str5.toAscii().data(),str5.length());

    if(ui->lineEdit_2->text().isNull())
          memcpy(_passwd,"",1);
    else
          memcpy(_passwd,ui->lineEdit_2->text().toAscii().data(),ui->lineEdit_2->text().length());

    /* pap */
    memset(command, 0, sizeof(command));
    sprintf(command, "sed -e '$c\\ \"%s\"    *     \"%s\"    *  ' %s>%s ",_username, _passwd, pap, TMPFILE);
    system(command);
    copy_from_tmpfile(pap);


    /* chap */
    memset(command, 0, sizeof(command));
    sprintf(command, "sed -e '$c\\ \"%s\"    *     \"%s\"    *  ' %s>%s ",_username, _passwd, chap, TMPFILE);

    system(command);
    copy_from_tmpfile(chap);

    return 0;
}


void GsmSetting::copy_from_tmpfile(char *file)
{
    char command[100];

    memset(command, 0, sizeof(command));
    sprintf(command, "cp -f %s %s",TMPFILE,file);
    system(command);
}


int GsmSetting::analysisDesc(QString desc, QString *brief, QString *valueList)
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

GsmSetting::~GsmSetting()
{
    keyList.clear();
    noteList.clear();
    OSGSM_exit();
    delete ui;
}

void GsmSetting::on_pushButton_clicked()
{
    close();
}

void GsmSetting::on_pushButton_2_clicked()
{
    setValue();
    system("sync");
    close();
}

void GsmSetting::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()){
    case Qt::Key_Escape:
    case Qt::Key_Cancel:
        close();
        break;
    }
}
