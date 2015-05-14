#-------------------------------------------------
#
# Project created by QtCreator 2012-04-09T09:49:41
#
#-------------------------------------------------

QT       += core gui

TARGET = qLcdDaemon
TEMPLATE = app



SOURCES += main.C\
                lcddaemon.C \
        lcddaemonmsgqueue.C \
        gui/parallaxhome.cpp \
        gui/node.cpp \
        gui/navibar.cpp \
    topform.cpp \
    syswindow.cpp \
    socketserver.C \
    loadgif.cpp \
    gui/button.cpp \
    gui/baseForm.cpp \
    gui/secondmenu.cpp \
    smt/downloadMode.cpp \
    smt/appManagement.cpp \
    smt/downloading.cpp \
    smt/download.cpp \
    smt/tablewidget.cpp \
    poweroff.cpp \
    wifithread.cpp \
    sanddisplay.cpp

HEADERS  += lcddaemon.h \
        lcddaemonmsgqueue.h \
        common.h \
        gui/parallaxhome.h \
        gui/node.h \
        gui/navibar.h \
    topform.h \
    syswindow.h \
    socketserver.h \
    loadgif.h \
    gui/secondmenu.h \
    gui/button.h \
    gui/baseForm.h \
    gui/secondmenu.h \
    smt/downloadMode.h \
    smt/appManagement.h \
    smt/downloading.h \
    smt/download.h \
    smt/tablewidget.h \
    poweroff.h \
    wifithread.h \
    sanddisplay.h

RESOURCES +=

FORMS    +=
#    start.ui \

LIBS += -L$$OUT_PWD/../qSandIME/ -lqSandIME -L../../../../appLib/rel -lapp -liconv -ljpeg -liw

INCLUDEPATH += $$PWD/../qSandIME ../../../../appLib/appApi/inc
DEPENDPATH += $$PWD/../qSandIME ../../../../appLib/appApi/inc


INCLUDEPATH += ../../../../appLib/appApi/src/src/
DEPENDPATH += ../../../../appLib/appApi/src/src/

OTHER_FILES +=

UI_DIR = ui
MOC_DIR = moc
OBJECTS_DIR = obj
