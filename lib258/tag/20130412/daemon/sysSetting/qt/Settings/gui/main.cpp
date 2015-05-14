/*
 * wpa_gui - Application startup
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

extern "C"{
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
//#include "osdrv.h"
//#include "sand_time.h"
//#include "smart.h"
//#include "sand_lcd.h"
//#include "sand_modem.h"
//#include "sand_gprs.h"
//#include "oscfg_param.h"
//#include "new_drv.h"

//#include "tools.h"

/*********以下三个头文件有问题*******/
//#include "toolslib.h"
//#include "osicc.h"
//#include "oslib.h"

    void manager()
    {

    }
}

//#ifdef CONFIG_NATIVE_WINDOWS
//#include <winsock.h>
//#endif /* CONFIG_NATIVE_WINDOWS */
//#include <QApplication>
//#include <QtCore/QLibraryInfo>
//#include <QtCore/QTranslator>
//#include "wpagui.h"

#include "parallaxhome.h"


//class WpaGuiApp : public QApplication
//{
//public:
//	WpaGuiApp(int &argc, char **argv);

//#ifndef QT_NO_SESSIONMANAGER
//	virtual void saveState(QSessionManager &manager);
//#endif

//	WpaGui *w;
//};

//WpaGuiApp::WpaGuiApp(int &argc, char **argv) : QApplication(argc, argv)
//{
//}

//#ifndef QT_NO_SESSIONMANAGER
//void WpaGuiApp::saveState(QSessionManager &manager)
//{
//	QApplication::saveState(manager);
//	w->saveState();
//}
//#endif


//int main(int argc, char *argv[])
//{
//	WpaGuiApp app(argc, argv);
//	QTranslator translator;
//	QString locale;
//	QString resourceDir;
//	int ret;

//	locale = QLocale::system().name();
//	resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
//	if (!translator.load("wpa_gui_" + locale, resourceDir))
//		translator.load("wpa_gui_" + locale, "lang");
//	app.installTranslator(&translator);

//	WpaGui w(&app);

//#ifdef CONFIG_NATIVE_WINDOWS
//	WSADATA wsaData;
//	if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
//		/* printf("Could not find a usable WinSock.dll\n"); */
//		return -1;
//	}
//#endif /* CONFIG_NATIVE_WINDOWS */

//	app.w = &w;

//	ret = app.exec();

//#ifdef CONFIG_NATIVE_WINDOWS
//	WSACleanup();
//#endif /* CONFIG_NATIVE_WINDOWS */

//	return ret;
//}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file("./gui1.qss");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
          QTextStream in(&file);
          QString line = in.readAll();

          qApp->setStyleSheet(line);
          file.close();
      }

//    Widget w;
//    w.show();

//    QTranslator translator;
//    QString locale;
//    QString resourceDir;

//    locale = QLocale::system().name();
//    resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
//    if (!translator.load("wpa_gui_" + locale, resourceDir))
//        translator.load("wpa_gui_" + locale, "lang");
//    qApp->installTranslator(&translator);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));



    ParallaxHome w(argc, argv);// = new ParallaxHome(argc, argv);
    w.resize(240 , 320);
    w.show();


    return a.exec();
}
