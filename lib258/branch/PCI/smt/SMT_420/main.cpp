#include <QtGui/QApplication>
#include <QTextCodec>
#include <QDesktopWidget>
#include <QTranslator>
#include "smtmainwindow.h"
#include "tools.h"

int main(int argc, char *argv[])
{
    QTextCodec* qc= QTextCodec::codecForName("GB2312");
    QTextCodec::setCodecForLocale(qc);
    QTextCodec::setCodecForCStrings(qc);
    QTextCodec::setCodecForTr(qc);

    QApplication a(argc, argv);
    //ÉèÖÃÓïÑÔ
    QTranslator *translator = new QTranslator(qApp);
    Tools tool;
    if(tool.getPrivateProfileString("PortSettings","Language","CN",NULL).compare("CN") == 0)
    {
        translator->load("./language/zh_CN.qm");
        qApp->installTranslator(translator);
    }

    SMTMainWindow smtMainWindow;
    QDesktopWidget* desktop = QApplication::desktop();
    smtMainWindow.move((desktop->width() - smtMainWindow.width())/2, (desktop->height() - smtMainWindow.height())/2-40);

    smtMainWindow.show();
    
    return a.exec();
}
