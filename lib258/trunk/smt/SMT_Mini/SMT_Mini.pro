#-------------------------------------------------
#
# Project created by QtCreator 2012-09-21T10:09:51
#
#-------------------------------------------------

QT       += core gui

TARGET = SMT_Mini
TEMPLATE = app
INCLUDEPATH += ./ ./ansifilter
include(./serialport/qextserialport.pri)

SOURCES += main.cpp\
        smt_mini.cpp \
    tools.cpp \
    libzip.c

SOURCES += ./ansifilter/elementstyle.cpp ./ansifilter/plaintextgenerator.cpp ./ansifilter/codegenerator.cpp
SOURCES += ./ansifilter/platform_fs.cpp ./ansifilter/rtfgenerator.cpp ./ansifilter/htmlgenerator.cpp ./ansifilter/texgenerator.cpp
SOURCES += ./ansifilter/latexgenerator.cpp ./ansifilter/bbcodegenerator.cpp ./ansifilter/stringtools.cpp ./ansifilter/stylecolour.cpp ./ansifilter/preformatter.cpp

HEADERS  += smt_mini.h \
    tools.h \
    lableMerge.h \
    libzip.h

FORMS    += smt_mini.ui

RESOURCES += resource.qrc

RC_FILE = smtlogo.rc
