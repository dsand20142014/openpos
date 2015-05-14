#include <QApplication>
#include <QFile>
#include "frame.h"
#include "qtextcodec.h"
#include "include.h"
#include "global.h"
#include <semaphore.h>

extern "C"
{
#include "sand_main.h"
}
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile file("./style.qss");
    file.open(QFile::ReadOnly);
    app.setStyleSheet(file.readAll());

    QTextCodec :: setCodecForTr( QTextCodec :: codecForName( "utf8" ));  //为QObject::tr设置字符编码

    QTextCodec :: setCodecForLocale( QTextCodec :: codecForName( "utf8" ));  //如果在linux上，设置local为gb18030可能会导致读取
                                                                                 //或输出的unicode中文字符不能正确显示
    QTextCodec ::setCodecForCStrings(QTextCodec :: codecForName( "utf8" ) );  //为QString设置字符编码，默认情况下为Latin1

    qDebug("entry to the main,wait to display frame");
    Frame frame(argc, argv);
//    frame.showFullScreen();
    frame.hide();
    qDebug("entry to the main,wait to entry gui");
    main_entry_gui(argc,argv);
    OnEve_power_on();
    sem_init(&binSem1,0,0);
    sem_init(&binSem2,0,0);
    printf("\n\n\n ------main-tree-begin-------\n\n");
    return app.exec();
}
}
