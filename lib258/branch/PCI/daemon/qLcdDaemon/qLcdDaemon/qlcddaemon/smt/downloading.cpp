#include "downloading.h"
#include <QMovie>
#include <sys/msg.h>
#include <sys/errno.h>

extern "C"{
extern int msqId;
extern UP_SER_MSG msgSMT;
}

Downloading::Downloading(bool process, QWidget *parent) :
    BaseForm(tr("DOWNLOADING"), parent)
{

    label_gif = new QLabel();
    //label_gif->setAlignment(Qt::AlignCenter);
    //label_gif->setMinimumSize(190, 60);

    layout->addWidget(label_gif);

    if (process) {
        progressBar = new QProgressBar();
        progressBar->setValue(0);
        progressBar->setMaximum(100);
        layout->addWidget(progressBar);

        msqid = msqId;
        msg = &msgSMT;

        connect(&timer,SIGNAL(timeout()),this,SLOT(slot_process()));
        start();
    } else {
        OSSPD_smt_update(1);//开始更新
        label_gif->setText(tr("Please reference the SMT on your\n PC."));
    }
}

void Downloading::start()
{
    btnExit->setEnabled(false);
    progressBar->show();
    OSSPD_smt_update(1);//开始更新
    timer.start(500);
}

void Downloading::stop()
{
    if(timer.isActive())
        timer.stop();

    progressBar->hide();
    label_gif->setText(tr("Download completed!"));
    btnExit->setEnabled(true);
}

void Downloading::stop_error()
{
    stop();
    label_gif->setText(tr("Download Error!"));
}

void Downloading::slot_process()
{    
    int ret, process=0;

    if(msqid < 0 || msg == NULL)
        stop_error();
    ret = msgrcv(msqid, msg, sizeof(UP_SER_MSG), MSG_TYPE_SERVER, IPC_NOWAIT);
    //qDebug("XXXXXXXXXXX msgrcv = %d msgtype = %d\n", ret, MSG_TYPE_SERVER);
    if(ret < 0 && errno != ENOMSG) {// rev msg from updateserver.
        messageBox *msgbox = new messageBox(tr("Updateserver ERROR!"));
        msgbox->exec();
        stop_error();
    }
    else if (ret > 0) {
        //qDebug("LLLLLLLLL Process = %d\n", msg->msg_data.ucProcess);
        process = msg->msg_data.ucProcess;
        if (process == 101) { // error
            messageBox *msgbox = new messageBox(tr("Download Error!"));
            msgbox->exec();
            stop_error();
        } else {
            progressBar->setValue(process);
            if (process == MSG_PROCESS_COMPLETE) {
                stop();
            }
        }
    }
}

Downloading::~Downloading()
{     
    if(timer.isActive())
        timer.stop();
    OSSPD_smt_update(0);//结束更新
}
