QT       += core gui

#TARGET = 00126
TEMPLATE = app

LANGUAGE	= C++

CONFIG	+= qt warn_on release

DEFINES += CONFIG_CTRL_IFACE



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
    mainentrythread.cpp


RESOURCES += \
    icons.qrc


unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}


#unix:!macx:!symbian: LIBS += -L$$PWD/../../../../appLib/rel/ -lapp

#INCLUDEPATH += $$PWD/gui/include
#DEPENDPATH += $$PWD/gui/include



HEADERS += \
    mainmenu.h \
    gsm.h \
    date.h \
    readsetting.h \
    gsm2.h \
    gsm3.h \
    gsm4.h \
    module.h \
    screen.h \
    wifi.h \
    wifi2.h \
    wifi3.h \
    ethernet.h \
    gps.h \
    node.h \
    navibar.h \
    moden.h \
    gsmscroll.h \
    parallaxhome.h \
    wpamsg.h \
    wpagui.h \
    userdatarequest.h \
    stringquery.h \
    scanresults.h \
    peers.h \
    networkconfig.h \
    eventhistory.h \
    addinterface.h \
    mainentrythread.h

SOURCES += \
    mainmenu.cpp \
    main.cpp \
    gsm.cpp \
    date.cpp \
    readsetting.cpp \
    gsm2.cpp \
    gsm3.cpp \
    gsm4.cpp \
    module.cpp \
    screen.cpp \
    wifi.cpp \
    wifi2.cpp \
    wifi3.cpp \
    ethernet.cpp \
    gps.cpp \
    node.cpp \
    navibar.cpp \
    moden.cpp \
    gsmscroll.cpp \
    parallaxhome.cpp \
    wpagui.cpp \
    userdatarequest.cpp \
    stringquery.cpp \
    scanresults.cpp \
    peers.cpp \
    networkconfig.cpp \
    eventhistory.cpp \
    addinterface.cpp

FORMS += \
    mainmenu.ui \
    gsm.ui \
    date.ui \
    gsm2.ui \
    gsm3.ui \
    gsm4.ui \
    module.ui \
    screen.ui \
    wifi.ui \
    wifi2.ui \
    wifi3.ui \
    ethernet.ui \
    gps.ui \
    moden.ui \
    wpagui.ui \
    userdatarequest.ui \
    scanresults.ui \
    peers.ui \
    networkconfig.ui \
    eventhistory.ui

unix:!macx:!symbian: LIBS += -L$$PWD/oslib/lib/ -lapp

INCLUDEPATH += $$PWD/oslib/osinc
DEPENDPATH += $$PWD/oslib/osinc

RESOURCES += \
    #global \
    #local \
    #icons_png.qrc \
    #icons.qrc


unix:!macx:!symbian: LIBS += -L$$PWD/../../../../appLib/rel/ -lapp

INCLUDEPATH += $$PWD/../../../../appLib/appApi/inc
DEPENDPATH += $$PWD/../../../../appLib/appApi/inc
