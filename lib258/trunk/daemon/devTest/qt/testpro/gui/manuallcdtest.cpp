#include "manuallcdtest.h"
//#include "button.h"
#include <QtDebug>

ManualLcdTest::ManualLcdTest(QWidget *parent): BaseForm(NULL, parent)
{
    Button *btnTest = new Button(tr("Start Test"), 9);
    btnTest->setFocus();

    layout->addSpacerItem(new QSpacerItem(10, 50+2));

    layout->addWidget(btnTest);
    connect(btnTest, SIGNAL(clicked()), this, SLOT(on_btnOk_clicked()));
    connect(btnOk, SIGNAL(clicked()), this, SLOT(on_btnOk_clicked()));

    num = 0;
}

ManualLcdTest::~ManualLcdTest()
{

}

void ManualLcdTest::on_btnOk_clicked()
{
//    hide();
//    qDebug()<<"BEGIN on_btnOk_clicked";
//    system("/app/00001/00001 -qws wifi ");
//    qDebug()<<"END on_btnOk_clicked";
//    show();

    num++;
    if(num%3==1)
    {
        palette.setBrush(QPalette::Background, QBrush(QColor("red")));
        setPalette(palette);
    }
    else if(num%3==2) {
        palette.setBrush(QPalette::Background, QBrush(QColor("green")));
        setPalette(palette);
    }
    else {
        palette.setBrush(QPalette::Background, QBrush(QColor("blue")));
        setPalette(palette);

    }
}

//void ManualLcdTest::keyPressEvent(QKeyEvent *e){
//    switch(e->key()){
//    case Qt::Key_Enter:
//        on_btnOk_clicked();
//        break;
//    case Qt::Key_Escape:
//        close();
////        informationBox->setText(tr("Test Success!"));
////        informationBox->show();
////        informationBox->raise();
////        QTimer::singleShot(1000,this,SLOT(close()));
//        break;

//    }

//}
