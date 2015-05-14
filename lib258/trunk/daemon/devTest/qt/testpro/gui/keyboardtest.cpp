#include "keyboardtest.h"

#include <QtDebug>
extern "C"{
#include "sand_debug.h"
}

extern void _print(char *fmt,...);


KeyBoardTest::KeyBoardTest(QWidget *parent) :
    BaseForm(tr("KEYBOARD TEST"), parent)
{
    preKey = 0;
    count = 1;

    label = new QLabel(tr("PRESS KEY TO CONTINUE:"));
    label_2 = new QLabel(tr("REPEAT KEY COUNT:"));
    label_3 = new QLabel();
    label_4 = new QLabel();
    label_3->setObjectName("labelShowKey");
    label_4->setObjectName("labelShowKey");

    label_3->setAlignment(Qt::AlignCenter);
    label_4->setAlignment(Qt::AlignCenter);

    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(18);
    vbox->addWidget(label);
    vbox->addWidget(label_3);
    vbox->addWidget(label_2);
    vbox->addWidget(label_4);

    vbox->addStretch(1);
    vbox->setContentsMargins(30,10,30,10);

    textEdit->setLayout(vbox);

    layout->addWidget(textEdit);


}

KeyBoardTest::~KeyBoardTest()
{

}


void KeyBoardTest::keyPressEvent(QKeyEvent *event)
{
    QString key;
    int temp = 0;
    _print("event->key()===%02x\n",event->key());


    switch(event->key())
    {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:{
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }
        char value = event->key();
        key = QString(value);
    }
        break;

    case Qt::Key_Enter:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }
        key = QObject::tr("ENTER");
        break;
    case Qt::Key_Escape:
        if(preKey == event->key()){
            preKey = 0;
            //qDebug()<<"Qt::Key_Escape one*******"<<preKey;
            count = 1;
            showInforText(tr("TEST SUCCESS!"),false);

            QTimer::singleShot(1000,this,SLOT(close()));
            return;
        }
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }
        key = QObject::tr("CANCEL");
        break;

    case Qt::Key_Backspace:
        //qDebug()<<"Qt::Key_Backspace";
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = QObject::tr("CLEAR");
        break;
    case Qt::Key_Clear:
        //qDebug()<<"Qt::Key_clear";
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = QObject::tr("CLEAR");
        break;
    case Qt::Key_F12:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = QObject::tr("00");
        break;
    case Qt::Key_F9:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = QObject::tr("FEED");
        break;
    case Qt::Key_F3:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = QObject::tr("F3");
        break;
    case Qt::Key_F4:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = QObject::tr("F4");
        break;
    case Qt::Key_F1:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = "F1";
        break;
    case Qt::Key_F2:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = "F2";
        break;

    case Qt::Key_PowerOff:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = QObject::tr("POWER OFF");
        break;
    case Qt::Key_Camera:
        if(preKey == event->key())
            label_4->setText(QString::number(++count));
        else{
            label_4->setText(QString::number(1));
            count = 1;
        }

        key = QObject::tr("CAMERA");
        break;

    default:
        //qDebug()<<"default event->key()"<<event->key();
        break;

    }

    preKey = event->key();
    //qDebug()<<"~~~~~~~~~preKey"<<preKey;

    label_3->setText(key);


}
