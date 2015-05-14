#include <QtGui/QApplication>
#include <QTextCodec>
#include "smt_for_customer.h"

int main(int argc, char *argv[])
{
    QTextCodec* qc= QTextCodec::codecForName("GB2312");
    QTextCodec::setCodecForLocale(qc);
    QTextCodec::setCodecForCStrings(qc);
    QTextCodec::setCodecForTr(qc);

    QApplication a(argc, argv);
    SMT_For_Customer w;
    w.show();
    
    return a.exec();
}
