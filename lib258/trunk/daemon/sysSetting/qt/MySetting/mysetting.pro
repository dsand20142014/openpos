#-------------------------------------------------
#
# Project created by QtCreator 2012-07-25T14:46:58
#
#-------------------------------------------------

QT       += core gui

TARGET = 00001
TEMPLATE = app

LANGUAGE	= C++

CONFIG	+= qt warn_on release

DEFINES += CONFIG_CTRL_IFACE


unix:!macx:!symbian: LIBS += -L$$PWD/../../../../appLib/rel/ -lapp -ljpeg -liconv -liw -lts
#LIBS += -lts

INCLUDEPATH += $$PWD/gui/osinc
INCLUDEPATH += gui
DEPENDPATH += $$PWD/gui/osinc

unix: LIBS += $$PWD/libwpa.a
INCLUDEPATH += $$PWD/wpa -l


include($$PWD/test/test.pri)
include($$PWD/gui/gui.pri)


unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}


CODECFORTR = utf-8
TRANSLATIONS += i18n_setting_zh.ts

