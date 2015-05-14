#include "baseForm.h"
extern "C"{
#include "sand_key.h"
}

/*
 * Base Form - A basic widget with a title and buttons on bottom
 * All forms with a title and buttons must be based on BaseForm.
 */
BaseForm::BaseForm(const QString & text, QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint)
{
    int screenWidth = qApp->desktop()->screen(0)->width();
    int screenHeight = qApp->desktop()->screen(0)->height();
    int titleHeight = 25;
    int topbarHeight = 20; // statusbar height

    setAttribute(Qt::WA_DeleteOnClose);
    resize(screenWidth, screenHeight);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("/daemon/image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    if (text != NULL) {
        title = new QLabel(text);
        title->setAlignment(Qt::AlignCenter);
        title->setFixedHeight(titleHeight);
        title->setObjectName(tr("title"));
    }
    btnExit = new Button(tr("Exit"), 1);
    btnExit->setObjectName("btnExit");
    connect(btnExit, SIGNAL(clicked()),this, SLOT(close()));
    btnOk = new Button(tr("Ok"));
    //btnOk->setFocus();
    btnOk->hide();

    layout = new QVBoxLayout();
    //layout->setSpacing(10);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(20, topbarHeight, 20, 0);

    btnlayout = new QHBoxLayout();
    btnlayout->setContentsMargins(10, 5, 10, 10);
    btnlayout->addWidget(btnExit, Qt::AlignBottom);
    btnlayout->addWidget(btnOk, Qt::AlignBottom);

    QVBoxLayout *lt = new QVBoxLayout();
    lt->setMargin(0);
    lt->setSpacing(10);
    lt->addSpacerItem(new QSpacerItem(10, topbarHeight+2));
    if (text != NULL) {
        lt->addWidget(title);
    }
    lt->addLayout(layout);
    lt->addLayout(btnlayout);
    lt->setAlignment(btnlayout, Qt::AlignBottom|Qt::AlignCenter);
    setLayout(lt);
}

BaseForm::~BaseForm()
{

}

/*void BaseForm::mousePressEvent(QMouseEvent *)
{
    //Os__set_key_ensure(200); // for sleep
}*/

void BaseForm::keyPressEvent(QKeyEvent *e)
{
    //Os__set_key_ensure(200); // for sleep

    switch(e->key())
    {
        case Qt::Key_Escape:
        case Qt::Key_Backspace:
            close();
            break;
        case Qt::Key_Enter:
            if (btnExit->hasFocus()) {
                emit btnExit->click();
            }
            else if (btnOk->hasFocus()) {
                emit btnOk->click();
            }

        break;
    }

}
