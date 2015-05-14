#include "screensetting.h"

#include "button.h"

extern "C"{
#include "modules.h"
#include "sysparam.h"
#include "printer.h"
}

extern void _print(char *fmt,...);

ScreenSetting::ScreenSetting(QWidget *parent) : BaseForm(tr("PERSONALITY SETTING"),parent)
{
    moveY = 0;
    moveYmax = 0;
    pressflag = false;

    windowHeight = 0;

    lastpointY = 0;

    scroll = 0;
    widget = 0;
    vlayout = 0 ;
    comboBox = 0;
    comboBoxLa = 0;
    btnOk->setText(tr("Save"));
    btnOk->show();
    brightness = 0;
    preLanguageValue = 0;
    languageFlag = false;

    btnExit->setFocus();

    getValue();

    connect(btnOk, SIGNAL(clicked()), this, SLOT(on_pushButton_2_clicked()));
    connect(btnExit,SIGNAL(clicked()),this,SLOT(resetBrightness()));
}

void ScreenSetting::getValue()
{
    //add by xiongjian at 20130131
    SYS_SETTING sysSetting;

    scroll = new QScrollArea();
    widget = new QWidget;
    vlayout = new QVBoxLayout;
    layout->addWidget(scroll);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //get brightness value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"BRIGHTNESS");
    PARAM_setting_get(&sysSetting);

    tabOneLabel[0] = new QLabel(tr("BRIGHTNESS"));
    vlayout->addWidget(tabOneLabel[0]);

    slider = new QSlider(Qt::Horizontal);
    slider->setRange(20,255);

    slider->setTracking(true);

    brightness = atoi(sysSetting.value);
    //qDebug()<<"brightness==="<<brightness;
    if(brightness <= 0)
        brightness = 75;
    slider->setValue(brightness);

    slider->setFixedSize((180*screenWidth)/240,(30*screenHeight)/320);
    connect(slider,SIGNAL(valueChanged(int)),this,SLOT(setBrightness(int)));

    vlayout->addWidget(slider);

    //get sleep time value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"SLEEP_TIME");
    PARAM_setting_get(&sysSetting);
    tabOneLabel[2] = new QLabel(tr("SLEEP TIME(s)"));
    vlayout->addWidget(tabOneLabel[2]);
    tabOneEdit[1] = new QLineEdit(QString(sysSetting.value));
    vlayout->addWidget(tabOneEdit[1]);
    tabOneEdit[1]->setValidator(new QIntValidator(10,43200,this));


    tabOneEdit[1]->setFocus();

    connect(tabOneEdit[1],SIGNAL(returnPressed()),this,SLOT(findNextFocus()));


    tabOneLabel[3] = new QLabel(tr("PRINTER FONT TYPE"));
    comboBox = new QComboBox;
    vlayout->addWidget(tabOneLabel[3]);
    vlayout->addWidget(comboBox);

    QStringList list;
    list<<"LIGHT"<<"NORMAL"<<"DARK";
    comboBox->addItems(list);

    int level = PRINTER_get_printlevel();
    comboBox->setCurrentIndex(level);



    //get language
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"LANGUAGE_SETTING");
    strcpy(sysSetting.key,"LANGUAGE");
    int cn2 = PARAM_setting_get(&sysSetting);

    tabOneLabel[4] = new QLabel(tr("LANGUAGE "));
    vlayout->addWidget(tabOneLabel[4]);
    comboBoxLa = new QComboBox;
    vlayout->addWidget(comboBoxLa);

    int i2 = 0;
    while(i2<cn2)
        comboBoxLa->addItem(tr("%1").arg(sysSetting.briefList[i2++]));
    if(atoi(sysSetting.value) > 0)
        comboBoxLa->setCurrentIndex(atoi(sysSetting.value)-1);

    preLanguageValue = atoi(sysSetting.value);

    widget->setLayout(vlayout);
    scroll->setWidget(widget);

}


void ScreenSetting::findNextFocus()
{
    if(/*tabOneEdit[0]!=NULL &&*/ tabOneEdit[1]!=NULL)
    {
       if(tabOneEdit[1]->hasFocus())
            btnOk->setFocus();

    }
}

void ScreenSetting::resetBrightness()
{
    Os__lcd_bright(brightness);
    close();
}

void ScreenSetting::setBrightness(int value)
{
    //qDebug()<<"value"<<value;
    Os__lcd_bright(value);

}


void ScreenSetting::setValue()
{
    _print("000ScreenSetting::setValue()\n");

    //add by xiongjian at 20130131
    SYS_SETTING sysSetting;
    //set brightness value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"BRIGHTNESS");
    sprintf(sysSetting.value,"%d",slider->value());
    int rec = PARAM_setting_set(&sysSetting);
    _print("rec===%d\n",rec);


    //set sleep time value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"SLEEP_TIME");
    sprintf(sysSetting.value,"%d",tabOneEdit[1]->text().simplified().toInt());
    int rec2 =PARAM_setting_set(&sysSetting);


    _print("~~~~~~comboBox->currentIndex(==%d\n",comboBox->currentIndex());
    int recss = PRINTER_set_printlevel(comboBox->currentIndex());
    _print("~~~~~recss ===%d\n",recss);


    //set language
    _print("preLanguageValue==%d\n",preLanguageValue);
    if(preLanguageValue==(comboBoxLa->currentIndex()+1)) return;

    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"LANGUAGE_SETTING");
    strcpy(sysSetting.key,"LANGUAGE");
    sprintf(sysSetting.value,"%d",comboBoxLa->currentIndex()+1);
    int rec4 = PARAM_setting_set(&sysSetting);

    if(!rec4)
        languageFlag = true;
}



ScreenSetting::~ScreenSetting()
{

}


void ScreenSetting::on_pushButton_2_clicked()
{
    btnOk->setEnabled(false);

    if(tabOneEdit[1]->text().toInt()<20)
    {
        showInforText(tr(" SLEEP TIME MUST BE OVER 20s! "));
        QTimer::singleShot(2000,informationBox,SLOT(hide()));

        tabOneEdit[1]->setFocus();
        btnOk->setEnabled(true);

        return;
    }

    setValue();
    system("sync");

    _print("languageFlag==%d\n",languageFlag);
    if(languageFlag)
        showInforText(tr("SET SUCCESS,LANGUAGE \nNEED RESTART TO SHOW."));
    else
        showInforText(tr("  SET SUCCESS! "));

    QTimer::singleShot(2000,this,SLOT(close()));

}

void ScreenSetting::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Enter:
        on_pushButton_2_clicked();
        break;
    case Qt::Key_Escape:
        resetBrightness();
        close();
        break;
    case Qt::Key_F2:
        if(slider->value()+10>255)
            slider->setValue(255);
        else
            slider->setValue(slider->value()+10);
        break;

    case Qt::Key_F1:
        if((slider->value()-10)<20)
            slider->setValue(20);
        else
            slider->setValue(slider->value()-10);
        break;
    }

}


void ScreenSetting::mousePressEvent( QMouseEvent * event )
{
    pressflag = true;
    lastpointY = event->pos().y();
}

void ScreenSetting::mouseMoveEvent(QMouseEvent * event)
{
    windowHeight = this->rect().height();

    if(pressflag == true)
    {
        QPoint nowpoint = event->pos();
        int y = nowpoint.y()-lastpointY;

        moveY+=y;
        moveYmax = 480 - windowHeight;


        if(moveYmax<= 0|| abs(moveY)>moveYmax|| moveY>0)
        {
            moveY-=y;
            y = 0 ;
        }
        widget->scroll(0,y);

        lastpointY = nowpoint.y();
    }
}

void ScreenSetting::mouseReleaseEvent(QMouseEvent * event)
{
    pressflag = false;
}

