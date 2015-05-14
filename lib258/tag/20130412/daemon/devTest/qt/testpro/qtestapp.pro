#-------------------------------------------------
#
# Project created by QtCreator 2012-07-25T14:46:58
#
#-------------------------------------------------

QT       += core gui

TARGET = 00002
TEMPLATE = app
TRANSLATIONS = myexec_zh.ts


unix:!macx:!symbian: LIBS += -L$$PWD/../../../../appLib/rel/ -lapp -ljpeg -liconv

INCLUDEPATH += $$PWD/gui/osinc
DEPENDPATH += $$PWD/gui/osinc

#unix:!macx:!symbian: LIBS += -L$$PWD/../../../../appLib/rel/ -lapp -ljpeg -liconv

#INCLUDEPATH += $$PWD/../../../../appLib/rel/osinc
#DEPENDPATH += $$PWD/../../../../appLib/rel/osinc

include($$PWD/test/test.pri)
include($$PWD/gui/gui.pri)



