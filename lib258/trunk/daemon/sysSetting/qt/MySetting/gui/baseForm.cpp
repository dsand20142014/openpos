#include "baseForm.h"

extern void _print(char *fmt,...);

/*
 * Base Form - A basic widget with a title and buttons on bottom
 * All forms with a title and buttons must be based on BaseForm.
 */
BaseForm::BaseForm(const QString & text, QWidget *parent) :
    QWidget(parent,Qt::FramelessWindowHint)
{
    screenWidth = qApp->desktop()->screen(0)->width();//240
    screenHeight = qApp->desktop()->screen(0)->height();//320
    int titleHeight = 25;
    int topbarHeight = 20; // statusbar height

    setAttribute(Qt::WA_DeleteOnClose);
    resize(screenWidth, screenHeight);

    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("/daemon/image/thirdback.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    if (!text.isNull()) {
        title = new QLabel(text);
        title->setAlignment(Qt::AlignCenter);
        title->setFixedHeight(titleHeight);
        title->setObjectName(tr("title"));
    }
    btnExit = new Button(tr("Exit"), 1);
    btnExit->setObjectName("btnExit");
    connect(btnExit, SIGNAL(clicked()),this, SLOT(close()));
    btnOk = new Button(tr("Ok"));
    btnOk->hide();

    informationBox = new QLabel(this);
    informationBox->setObjectName("InfoLabel");
    informationBox->hide();
    informationBox->setAlignment(Qt::AlignCenter);

    informationBox->setFixedSize((210*screenWidth)/240,(50*screenHeight)/320);
    informationBox->setGeometry((screenWidth-informationBox->width())/2,(screenHeight-informationBox->height())/2,informationBox->width(),informationBox->height());

    layout = new QVBoxLayout();
    //layout->setSpacing(10);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(10, 0, 10, 0);

    btnlayout = new QHBoxLayout();
    btnlayout->setContentsMargins(10, 5, 10, 10);
    btnlayout->addWidget(btnExit, Qt::AlignBottom);
    btnlayout->addWidget(btnOk, Qt::AlignBottom);

    QVBoxLayout *lt = new QVBoxLayout();
    lt->setMargin(0);
    lt->setSpacing(10);
    lt->addSpacerItem(new QSpacerItem(10, topbarHeight+2));
    if (!text.isNull()) {
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

void BaseForm::showInforText(QString text,bool moveFLag,int w ,int h )
{
    informationBox->setText(text);
    informationBox->raise();
    informationBox->show();

    if(moveFLag)
       informationBox->move(w,h);

}


void BaseForm::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:{
        _print("btn exit click\n");
        btnExit->click();

    }
        break;
    case Qt::Key_Enter:
        btnOk->click();
        break;
    }

}
