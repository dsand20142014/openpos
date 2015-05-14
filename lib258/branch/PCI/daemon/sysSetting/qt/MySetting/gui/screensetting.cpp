#include "screensetting.h"

#include "button.h"

extern "C"{
#include "modules.h"
#include "sysparam.h"
}

extern void _print(char *fmt,...);

ScreenSetting::ScreenSetting(QWidget *parent) : BaseForm(tr("SCREEN SETTING"),parent)
{
    scroll = 0;
    widget = 0;
    vlayout = 0 ;

    btnOk->setText(tr("Save"));
    btnOk->show();
    brightness = 0;

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

    //get brightness value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"BRIGHTNESS");
    PARAM_setting_get(&sysSetting);

    tabOneLabel[0] = new QLabel("BRIGHTNESS");
    vlayout->addWidget(tabOneLabel[0]);

    slider = new QSlider(Qt::Horizontal);
    slider->setRange(20,255);

    slider->setTracking(true);

    brightness = atoi(sysSetting.value);
    //qDebug()<<"brightness==="<<brightness;
    if(brightness <= 0)
        brightness = 75;
    slider->setValue(brightness);

    slider->setFixedSize(180,30);
    connect(slider,SIGNAL(valueChanged(int)),this,SLOT(setBrightness(int)));

    vlayout->addWidget(slider);

#if 0
    //get screen save time value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"SCREENSAVER_TIME");
    PARAM_setting_get(&sysSetting);
    tabOneLabel[1] = new QLabel("SCREEN SAVER TIME");
//    tabOneLabel[1]->setGeometry(30,90,170,31);
    vlayout->addWidget(tabOneLabel[1]);
    tabOneEdit[0] = new QLineEdit(QString(sysSetting.value));
    vlayout->addWidget(tabOneEdit[0]);
    tabOneEdit[0]->setFocus();

#endif

    //get sleep time value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"SLEEP_TIME");
    PARAM_setting_get(&sysSetting);
    tabOneLabel[2] = new QLabel("SLEEP TIME(s)");
    vlayout->addWidget(tabOneLabel[2]);
    tabOneEdit[1] = new QLineEdit(QString(sysSetting.value));
    vlayout->addWidget(tabOneEdit[1]);
    tabOneEdit[1]->setValidator(new QIntValidator(10,43200,this));


    tabOneEdit[1]->setFocus();

    connect(tabOneEdit[1],SIGNAL(returnPressed()),this,SLOT(findNextFocus()));

    widget->setLayout(vlayout);
    scroll->setWidget(widget);
    //add end
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
#if 0
    //set screen save time value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"SCREENSAVER_TIME");
    sprintf(sysSetting.value,"%d",tabOneEdit[0]->text().simplified().toInt());
    int rec1 =PARAM_setting_set(&sysSetting);
    //qDebug()<<"rec1==="<<rec1;
#endif

    //set sleep time value
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"SCREEN_SETTING");
    strcpy(sysSetting.key,"SLEEP_TIME");
    sprintf(sysSetting.value,"%d",tabOneEdit[1]->text().simplified().toInt());
    int rec2 =PARAM_setting_set(&sysSetting);
    //qDebug()<<"rec2==="<<rec2;

    //qDebug()<<"ScreenSetting::setValue()";
//add end
}



ScreenSetting::~ScreenSetting()
{

}


void ScreenSetting::on_pushButton_2_clicked()
{
    btnOk->setEnabled(false);

    if(tabOneEdit[1]->text().toInt()<20)
    {
        informationBox->setText(tr(" SLEEP TIME MUST BE OVER 20s! "));
        informationBox->raise();
        informationBox->show();
        QTimer::singleShot(2000,informationBox,SLOT(hide()));

        tabOneEdit[1]->setFocus();
        btnOk->setEnabled(true);

        return;
    }

    setValue();
    system("sync");

    informationBox->setText(tr("  SET SUCCESS!  "));
    informationBox->raise();
    informationBox->show();
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


