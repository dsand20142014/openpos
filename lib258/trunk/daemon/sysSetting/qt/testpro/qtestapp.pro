#-------------------------------------------------
#
# Project created by QtCreator 2012-07-25T14:46:58
#
#-------------------------------------------------

QT       += core gui

TARGET = 00126
TEMPLATE = app

LANGUAGE	= C++

CONFIG	+= qt warn_on release

DEFINES += CONFIG_CTRL_IFACE



#include($$PWD/test/test.pri)
include($$PWD/gui/gui.pri)



win32 {
  LIBS += -lws2_32 -static
  DEFINES += CONFIG_NATIVE_WINDOWS CONFIG_CTRL_IFACE_NAMED_PIPE
  SOURCES += ./src/utils/os_win32.c
} else:win32-g++ {
  # cross compilation to win32
  LIBS += -lws2_32 -static -mwindows
  DEFINES += CONFIG_NATIVE_WINDOWS CONFIG_CTRL_IFACE_NAMED_PIPE
  SOURCES += ./src/utils/os_win32.c
  RESOURCES += icons_png.qrc
} else:win32-x-g++ {
  # cross compilation to win32
  LIBS += -lws2_32 -static -mwindows
  DEFINES += CONFIG_NATIVE_WINDOWS CONFIG_CTRL_IFACE_NAMED_PIPE
  DEFINES += _X86_
  SOURCES += ./src/utils/os_win32.c
  RESOURCES += icons_png.qrc
} else {
  DEFINES += CONFIG_CTRL_IFACE_UNIX
  SOURCES += ./src/utils/os_unix.c
}

INCLUDEPATH	+=  ./src ./src/utils


SOURCES	+= src/common/wpa_ctrl.c \


RESOURCES += \
    icons.qrc


unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}



unix:!macx:!symbian: LIBS += -L$$PWD/../../../../appLib/rel/ -lapp -ljpeg -liconv

INCLUDEPATH += $$PWD/gui/include
DEPENDPATH += $$PWD/gui/include
