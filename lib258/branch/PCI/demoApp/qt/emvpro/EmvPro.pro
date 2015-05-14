#-------------------------------------------------
#
# Project created by QtCreator 2012-11-01T14:15:22
#huangdanli@
#-------------------------------------------------

QT       += core gui

TARGET = emvPro
TEMPLATE = app

include($$PWD/gui/gui.pri)
include($$PWD/test/test.pri)


#unix:!macx:!symbian: LIBS += -L$$PWD/appsrc/ -lemv

#INCLUDEPATH += $$PWD/appsrc/EMVInc
#DEPENDPATH += $$PWD/appsrc/EMVInc



unix:!macx:!symbian: LIBS += -L$$PWD/../../appLib/rel/ -lapp -ljpeg -liconv

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

unix:!macx:!symbian: LIBS += -L$$PWD/appsrc/ -lemv

INCLUDEPATH += $$PWD/appsrc/EMVInc
DEPENDPATH += $$PWD/appsrc/EMVInc
