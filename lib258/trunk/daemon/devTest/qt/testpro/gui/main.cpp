#include <QTranslator>
#include "parallaxhome.h"
#include <QWSServer>
extern "C"{
  #include "ostools.h"
  #include "sysparam.h"
    void manager()
    {

    }
}


#define DBG

void _print(char *fmt,...)
{
#ifdef DBG
    Uart_Printf(fmt);
    return ;
#else
    return ;
#endif
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWSServer::setCursorVisible(false);


    QFile file("/daemon/qss/default.qss");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString line = in.readAll();

        qApp->setStyleSheet(line);
        file.close();
    }
    else {
        perror("Can not find the qss file!");
    }
    _print("IN MAIN AAA*******\n");

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    SYS_SETTING sysSetting;
    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"LANGUAGE_SETTING");
    strcpy(sysSetting.key,"LANGUAGE");
    PARAM_setting_get(&sysSetting);
    if(atoi(sysSetting.value)==1){
        /* Translator */
        QTranslator *translator = new QTranslator();
        translator->load("/daemon/qss/i18n_test_zh.qm");
        a.installTranslator(translator);
    }

    ParallaxHome *w = new ParallaxHome(argc, argv);

    w->resize(qApp->desktop()->screen(0)->width() , qApp->desktop()->screen(0)->height());

    _print("IN MAIN*******222222\n");

//#ifdef thread
//    w->hide();
//#elif
     w->show();
//#endif


    return a.exec();
}