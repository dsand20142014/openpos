#include <QtGui/QApplication>
#include <QWSServer>
#include <QObject>
#include <QTextCodec>
#include <QTranslator>
#include "lcddaemon.h"
#include "lcddaemonmsgqueue.h"
#include <signal.h>

/*
 * signal handle
 */
static void sigHandler(int sig)
{
    qDebug("!!QLCD ERROR!! Get terminate signal: %d!", sig);
}

/*
 * Get and handle system signal SIGTERM.
 * Add by xiagnliu 2013.3.14 for not exit when get SIGTERM.
 */
static void installSigaction()
{
    struct sigaction sigb;
    sigb.sa_handler = sigHandler;
    sigb.sa_flags  = 0;
    memset (&sigb.sa_mask, 0, sizeof(sigset_t));
    sigaction(SIGTERM, &sigb, NULL);
}

//入口
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //获取系统参数
    QStringList sysArgv;
    argv += 1;
    while(*argv)
        sysArgv << *argv++;

    QWSServer::setCursorVisible(false);//不显示鼠标
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QTranslator *translator = new QTranslator();
    translator->load("/daemon/qss/i18n_qlcddaemon_zh.qm");
    app.installTranslator(translator);

    QFile file("/daemon/qss/default.qss");
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    }
    else {
        perror("Can not find the qss file!");
    }

    LcdDaemonMsgQueue msgQueue;
    LcdDaemon win(0, &sysArgv);

    QObject::connect(&msgQueue,SIGNAL(newMessage(QVariant)),
            &win,SLOT(lcdMsgProc(QVariant)));

    installSigaction(); //install signal handle.

    msgQueue.start();
    win.showMaximized();

    return app.exec();
}
