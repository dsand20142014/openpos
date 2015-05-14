#-------------------------------------------------
#
# Project created by QtCreator 2012-07-25T14:46:58
#
#-------------------------------------------------

QT       += core gui

TARGET = 00002
TEMPLATE = app
TRANSLATIONS = myexec_zh.ts


unix:!macx:!symbian: LIBS += -L$$PWD/../../../../appLib/rel/ -lapp -ljpeg -liconv -liw -lts

INCLUDEPATH += $$PWD/gui/osinc
DEPENDPATH += $$PWD/gui/osinc



include($$PWD/test/test.pri)
include($$PWD/gui/gui.pri)

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

