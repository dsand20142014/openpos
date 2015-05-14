#include "poweroff.h"
#include <QHBoxLayout>
extern "C"
{
#include "sysglobal.h"
}

PowerOff::PowerOff(QWidget *) :
    QWidget(0, Qt::FramelessWindowHint)
{   

    setStyleSheet("QWidget{background-color: rgba(51, 70, 98, 230)} QPushButton:pressed{background-color: rgba(20, 30, 50, 250)}");
    setFocusPolicy(Qt::StrongFocus);

    btn_suspend = new Button(tr("1. Suspend"));
    btn_poweroff = new Button(tr("2. Power off"));
    btn_reboot = new Button(tr("3. Reboot"), 2);
    btn_exit = new Button(tr("4. Cancel"), 1);

    label = new QLabel(tr("Are you sure to power off or reboot?"));
	label->setWordWrap(true);
    label->setStyleSheet("color: white; font: bold 14px; background-color: rgba(255, 255, 255, 0);");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(10);
	layout->setMargin(25);
    layout->addWidget(label);
    layout->addWidget(btn_suspend);
    layout->addWidget(btn_poweroff);
    layout->addWidget(btn_reboot);
    layout->addWidget(btn_exit);
    setLayout(layout);

    connect(btn_poweroff, SIGNAL(clicked()), this, SLOT(slot_poweroff()));
    connect(btn_reboot, SIGNAL(clicked()), this, SLOT(slot_reboot()));
    connect(btn_suspend, SIGNAL(clicked()), this, SLOT(slot_suspend()));
    connect(btn_exit, SIGNAL(clicked()), this, SLOT(close()));
}

void PowerOff::slot_poweroff()
{
    label->setText(tr("System will halt...\n"));
    system("halt");
}

void PowerOff::slot_reboot()
{
    label->setText(tr("System will reboot...\n"));
    system("reboot");
}

void PowerOff::slot_suspend()
{    
    hide();
    if (sleepThread.isRunning())
        sleepThread.quit();
    sleepThread.start();
    //SYSGLOBAL_sys_suspend();
}

void PowerOff::keyReleaseEvent(QKeyEvent *e)
//void PowerOff::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_Escape:
            hide();
            break;
		case '1':
            slot_suspend();                                                          
            break;
        case '2':
            slot_poweroff();
            break;
        case '3':
            slot_reboot();
            break;

        default:
            break;
    }
    e->accept();
}

PowerOff::~PowerOff()
{
}
