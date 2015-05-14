#include "beeptest.h"
extern "C"{
#include "modules.h"
#include "sysparam.h"
}
//extern int PARAM_setting_get(SYS_SETTING *setting);

BeepTest::BeepTest(int flag,QWidget *parent) :
    BaseForm(tr("BUZZER TEST"), parent)
{
    SYS_SETTING st;
    char buffer[50];

    count = 0;
    flags = flag;
    if(flags == 3){
        title->setText(tr("BUZZER TEST"));

        picBeep = new QLabel;
        picBeep->setPixmap(QPixmap("/daemon/image/beep.png"));
        pushButton_2 = new Button(tr(" START TEST "), 9);
        pushButton_2->setFocus();

        layout->addWidget(picBeep);
        layout->addWidget(pushButton_2);
        connect(pushButton_2, SIGNAL(clicked()), this, SLOT(on_pushButton_2_clicked()));

    }
    else if(flags == 1){
        title->setText(tr("LED TEST"));
        pushButton_2 = new Button(tr(" START TEST "), 9);
        pushButton_2->setFocus();
        layout->addSpacerItem(new QSpacerItem(10, 20+2));
        layout->addWidget(pushButton_2);
        connect(pushButton_2, SIGNAL(clicked()), this, SLOT(on_pushButton_2_clicked()));
        connect(&timer,SIGNAL(timeout()),SLOT(led_on()));

    }
    else if(flags == 2){
        title->setText(tr("LIGHT TEST"));
        slider = new QSlider(Qt::Horizontal);
        slider->show();
        slider->setRange(20,255);
        slider->setTracking(true);
        slider->setFixedSize(180,40);

        layout->addSpacerItem(new QSpacerItem(10, 40+2));

        layout->addWidget(slider);

//        layout->setSpacing(10);
        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(new QLabel("DARK"));
        hlayout->addWidget(new QLabel("BRIGHT"));
        hlayout->setContentsMargins(10 ,5,10,20);
        layout->addLayout(hlayout);

        connect(slider,SIGNAL(valueChanged(int)),this,SLOT(setBrightness(int)));

        //modified by lichao at 20130201 for new system parameter API
        strcpy(st.section,"SCREEN_SETTING");
        strcpy(st.key,"BRIGHTNESS");
        PARAM_setting_get(&st);
        strcpy(buffer,st.value);
        brightness = atoi(buffer);
        slider->setValue(brightness);
        //end modified

        connect(btnExit,SIGNAL(clicked()),this,SLOT(refreshBrightness()));
    }

}

void BeepTest::refreshBrightness()
{
    //qDebug()<<"brightness=="<<brightness;
    Os__lcd_bright(brightness);
}
void BeepTest::setBrightness(int value)
{    //qDebug()<<"set  brightness=="<<brightness;

    //qDebug()<<"value"<<value;
    Os__lcd_bright(value);

}

void BeepTest::led_on()
{
    informationBox->hide();
    timer.stop();
    if(count==0)
         Os__led_display(1,2);
    else if(count==1)
        Os__led_display(1,3);
    else if(count==2)
         Os__led_display(1,4);
    else if(count==3){
        Os__led_display(0,2);
        Os__led_display(0,3);
        Os__led_display(0,4);


        showInforText(tr("TEST SUCCESS!"),true,15,135+20);


        QTimer::singleShot(1500,informationBox,SLOT(hide()));

        count = 0;
        return;
    }

    count++;
    timer.start();
}

BeepTest::~BeepTest()
{

    if(flags!=2)
        informationBox->move(15,135);

}



void BeepTest::on_pushButton_2_clicked()
{
    if(timer.isActive()) return;

    if(flags == 3){
        //qDebug()<<"os_beep**********";
        btnOk->setEnabled(false);

        Os__beep();
        Os__beep();
        Os__beep();

//       showInforText(tr("TEST SUCCESS!"),true,15,135+40);

        QTimer::singleShot(2000,informationBox,SLOT(hide()));
        btnOk->setEnabled(true);

    }
    else if(flags ==1)
        timer.start(400);

}


void BeepTest::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:
    case Qt::Key_Backspace:
    {
        if(flags ==1)
        {
            if(timer.isActive())
                timer.stop();
            Os__led_display(0,2);
            Os__led_display(0,3);
            Os__led_display(0,4);
        }
        else if(flags ==2)
            refreshBrightness();

        close();
    }
        break;

    case Qt::Key_Enter:
        if(flags!=2)
            on_pushButton_2_clicked();
        break;

    case Qt::Key_F2:
        if(flags!=2) return;

        if(slider->value()+10>255)
            slider->setValue(255);
        else
            slider->setValue(slider->value()+10);

        break;

    case Qt::Key_F1:
        if(flags!=2) return;

        if((slider->value()-10)<20)
            slider->setValue(20);
        else
            slider->setValue(slider->value()-10);

        break;
    }
}
