#-------------------------------------------------
#
# Project created by QtCreator 2012-11-01T14:15:22
#
#-------------------------------------------------

QT       += core gui

TARGET = qpboc
TEMPLATE = app

#include($$PWD/test/test.pri)
include($$PWD/gui/gui.pri)



unix:!macx:!symbian: LIBS += -L$$PWD/appsrc/ -lvisawave

INCLUDEPATH += $$PWD/appsrc/VisaWaveInc
DEPENDPATH += $$PWD/appsrc/VisaWaveInc

unix:!macx:!symbian: LIBS += -L$$PWD/../../appLib/rel/ -lapp -ljpeg -liconv

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

HEADERS += \
    Util.h \
    File.h \
    Global.h \
    Digital.h \
    MasApp.h \
    Msg.h \
    EMVParam.h \
    include.h \
    smart.h

SOURCES += \
    Global.c \
    Util.c \
    MasApp.c \
    Msg.c \
    smart.c \
    File.c \
#    extprint.c \
    evecom.c \
    des.c \
    Digital.c \
    TermSetting.c \
    EMVParam.c \
    manager.c \
    libconv.c
