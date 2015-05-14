#include "poweroff.h"
#include <QHBoxLayout>
extern "C"
{
#include "sysglobal.h"
}

PowerOff::PowerOff(QWidget *) :
    QWidget(0, Qt::Tool| Qt::WindowStaysOnTopHint| Qt::FramelessWindowHint)
{   

    setStyleSheet("QWidget{background-color: rgba(51, 70, 98, 180)} QPushButton:pressed{background-color: rgba(20, 30, 50, 220)}");
    setFocusPolicy(Qt::StrongFocus);

    btn_poweroff = new Button(tr("Power off"));
    btn_reboot = new Button(tr("Reboot"), 2);
    btn_suspend = new Button(tr("Suspend"));
    btn_exit = new Button(tr("Cancel"), 1);

    label = new QLabel(tr("Are you sure to power off \nor reboot?"));
    label->setStyleSheet("color: white; font: bold 13px; background-color: rgba(255, 255, 255, 0);");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(20, 30, 20, 30);
    layout->addWidget(label);
    layout->addWidget(btn_poweroff);
    layout->addWidget(btn_reboot);
    layout->addWidget(btn_suspend);
    layout->addWidget(btn_exit);
    setLayout(layout);

    connect(btn_poweroff, SIGNAL(clicked()), this, SLOT(slot_poweroff()));
    connect(btn_reboot, SIGNAL(clicked()), this, SLOT(slot_reboot()));
    connect(btn_suspend, SIGNAL(clicked()), this, SLOT(slot_suspend()));
    connect(btn_exit, SIGNAL(clicked()), this, SLOT(close()));
}

void PowerOff::slot_poweroff()
{
    label->setText("System will halt...\n");
    system("halt");
}

void PowerOff::slot_reboot()
{
    label->setText("System will reboot...\n");
    system("reboot");
}

void PowerOff::slot_suspend()
{    
    hide();
    SYSGLOBAL_sys_suspend();
}

/*void PowerOff::keyPressEvent(QKeyEvent *e)
{
    qDebug("PowerOff::keyPressEvent");
    switch(e->key())
    {
        case Qt::Key_Escape:
            hide();
            break;
        default:
            break;
    }
    e->accept();
}
*/

PowerOff::~PowerOff()
{
}
