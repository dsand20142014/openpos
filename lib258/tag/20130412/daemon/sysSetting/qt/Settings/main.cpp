#include <QApplication>
//#include <QScrollArea>
#include "mainmenu.h"
#include "parallaxhome.h"
//#include "scrollscreen.h"
#include <QFile>

extern "C"
{
    void manager()
    {

    }
}

int main(int argc,char ** argv)
{
    QApplication app(argc,argv);
    QCoreApplication::setOrganizationName("Sand");
    QCoreApplication::setApplicationName("Inifiles");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QFile file("./style.qss");
    file.open(QFile::ReadOnly);
    app.setStyleSheet(file.readAll());

//    MainMenu *mainMenu = new MainMenu;

//    ScrollScreen *scrollArea = new ScrollScreen;
//    scrollArea->setWidget(mainMenu);
//    scrollArea->viewport()->setBackgroundRole(QPalette::Dark);
//    scrollArea->viewport()->setAutoFillBackground(true);
//    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//    scrollArea->resize(240,320);
//    scrollArea->show();
    //mainMenu->show();

    //mainMenu->showFullScreen();

    ParallaxHome *parallaxHome = new ParallaxHome(argc,argv);
    parallaxHome->resize(240 , 320);
    //parallaxHome->show();
    parallaxHome->hide();
    app.exec();
}
