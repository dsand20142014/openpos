#-------------------------------------------------
#
# Project created by QtCreator 2012-09-03T09:43:37
#
#-------------------------------------------------

QT       += core gui
QT       += network
TRANSLATIONS +=  zh_CN.ts
TARGET = SMT
TEMPLATE = app
INCLUDEPATH += . ./ansifilter ./rsa

include(./serialport/qextserialport.pri)

SOURCES += main.cpp\
        smtmainwindow.cpp \
    tools.cpp \
    csysiteminfo.cpp \
    cappiteminfo.cpp \
    treewidget.cpp \
    sessionoptions.cpp

SOURCES += ./ansifilter/elementstyle.cpp ./ansifilter/plaintextgenerator.cpp ./ansifilter/codegenerator.cpp
SOURCES += ./ansifilter/platform_fs.cpp ./ansifilter/rtfgenerator.cpp ./ansifilter/htmlgenerator.cpp ./ansifilter/texgenerator.cpp
SOURCES += ./ansifilter/latexgenerator.cpp ./ansifilter/bbcodegenerator.cpp ./ansifilter/stringtools.cpp ./ansifilter/stylecolour.cpp ./ansifilter/preformatter.cpp
SOURCES += ./rsa/libsha1.c ./rsa/librsa.c ./rsa/libnn.c

HEADERS  += smtmainwindow.h \
    tools.h \
    csysiteminfo.h \
    cappiteminfo.h \
    treewidget.h \
    sessionoptions.h

FORMS    += smtmainwindow.ui \
    sessionoptions.ui

RESOURCES += \
    resource.qrc
RC_FILE = smtlogo.rc
