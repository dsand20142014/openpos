
extern "C"{
#include "ostools.h"
#include "osdrv.h"
void manager()
    {

    }
}



#include "parallaxhome.h"
#include <QTranslator>
#include "wifisetwin.h"
#include "baseForm.h"
#include <QWSServer>

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

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

	/* Translator */
	QTranslator *translator = new QTranslator();
    translator->load("/daemon/qss/i18n_setting_zh.qm");
    a.installTranslator(translator);

//    OSDRV_install(DRVID_MMI,&gbl_mmi_op);

//    qDebug()<<"argc=="<<argc;
//    for(int i=0;i<argc;i++)
//         qDebug()<<"argv[i]"<<argv[i];


    if(argc>=2){
        if(!strcmp(argv[1],"wifi"))
        {
            WifiSetWin *wpa = new WifiSetWin();
            wpa->show();
        }

    }
    else{
        ParallaxHome *w = new ParallaxHome(argc, argv);
        int screenW = qApp->desktop()->screen(0)->width();
        int screenH = qApp->desktop()->screen(0)->height();
        w->resize(screenW, screenH);
        w->show();
    }


    return a.exec();
}
