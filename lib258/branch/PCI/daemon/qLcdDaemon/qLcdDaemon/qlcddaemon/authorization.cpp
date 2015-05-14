#include "authorization.h"
#include "gui/button.h"
#include "common.h"
#include "oslib.h"
#include "sand_pci.h"
#include "sysglobal.h"
}

extern int mqId;

/*
 * Window for Authorization
 */
AuthorizationForm::AuthorizationForm(QWidget *p) :
    QWidget(p, Qt::FramelessWindowHint)
{   
    setAttribute(Qt::WA_DeleteOnClose);
    label = new QLabel(tr("Not authorized! Please authorize!"));
//    label->setWordWrap(true);
//    label->setStyleSheet("font: bold 20px");

    QLabel *label1 = new QLabel(tr("Authorize"));
    label1->setAlignment(Qt::AlignCenter);
    label1->setStyleSheet("color: darkgray; font: bold 30px");
    QLabel *label2 = new QLabel(tr("Please input password:"));

    input = new QLineEdit();
    input->setEchoMode(QLineEdit::Password);
    input->setMaxLength(90);

    r1 = new QRadioButton(tr("Production"));
    r2 = new QRadioButton(tr("Maintainance"));
    r3 = new QRadioButton(tr("Attack"));
    r4 = new QRadioButton(tr("Update"));
    r1->setChecked(true);
    r1->setFocus();

    Button *btnExit = new Button(tr("Cancel"), 1);
    Button *btnOk = new Button(tr("OK"));
    connect(btnExit, SIGNAL(clicked()), this, SLOT(slot_btnExit_click()));
    connect(btnOk, SIGNAL(clicked()), this, SLOT(slot_btnOk_click()));

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(btnExit);
    hlayout->addWidget(btnOk);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(10, 30, 10, 10);
    layout->addWidget(label);
//    layout->addWidget(r1);
//    layout->addWidget(r2);
//    layout->addWidget(r3);
//    layout->addWidget(r4);
    layout->addSpacing(20);
    layout->addWidget(label1);
    layout->addSpacing(10);
    layout->addWidget(label2);
    layout->addWidget(input);
    layout->addStretch(1);
    layout->addLayout(hlayout);
    setLayout(layout);

    memset(&message, 0, sizeof(message));
    message.mtype = 48;
}

void AuthorizationForm::keyPressEvent(QKeyEvent *e)
{    
    switch(e->key())
    {
        case Qt::Key_Escape:
            slot_btnExit_click();
            break;

        case Qt::Key_Enter:
        {
            slot_btnOk_click();
            break;
        }
#if 0
        case '1':
            r1->setChecked(true);
            break;

        case '2':
            r2->setChecked(true);
            break;

        case '3':
            r3->setChecked(true);
            break;

        case '4':
            r4->setChecked(true);
            break;
#endif
        default:
            break;
    }
    e->accept();
}

/*
 * return: -1 com err; -2 send err; -3 rev err;
 */
//int AuthorizationForm::authorize()
//{
//    int ret = 0;

//    return ret;
//}

void AuthorizationForm::slot_btnOk_click()
{
    if(input->text()=="258036") {
        input->clear();
        label->setText(tr("Authorizing, please wait..."));
#if 0
        int i = 0;
        if (r1->isChecked())
            i = 1;
        else if (r2->isChecked())
            i = 2;
        else if (r3->isChecked())
            i = 3;
        else if (r4->isChecked())
            i = 4;
#endif


        int ret = authorize();
//        movie.stop();
//        labelWin->hide();

        if (ret == 0) {
            message.msg_data.gprs_d = 1;
            messageBox *msgbox = new messageBox(tr("Authorize success!"));
            msgbox->exec();
            close();
        }
        else {
            QString str;
            switch (ret) {
            case -1:
                str = "Serial connect or write error!\nDo you connect to OTG correctly?";
                break;
            case -2:
                str = "Authorize failed!\nDo you connect to OTG correctly?";
                break;
            case -3:
                str = "Write authorization error!";
                break;

            default:
                break;
            }
            messageBox *msgbox = new messageBox(str);
            msgbox->exec();
        }
    }
    else {
        input->clear();
    }
}
void AuthorizationForm::slot_btnExit_click()
{
    message.msg_data.gprs_d = 0;
    close();
}

void AuthorizationForm::sendmsg2linuxpos()
{
    //qDebug("\nExit Authorization!");
    if (msgsnd(mqId, &message, 140, 0) < 0) {
        perror("<sendmsg2linuxpos> msgsnd error");
    }
}

AuthorizationForm::~AuthorizationForm()
{
    sendmsg2linuxpos();
}

