#-------------------------------------------------
#
# Project created by QtCreator 2013-03-01T16:26:09
#
#-------------------------------------------------

QT       += core gui

TARGET = SMT_For_Customer
TEMPLATE = app

include(./serialport/qextserialport.pri)

SOURCES += main.cpp\
        smt_for_customer.cpp \
    tools.cpp

HEADERS  += smt_for_customer.h \
    tools.h

FORMS    += smt_for_customer.ui

RESOURCES += \
    resource.qrc
RC_FILE = smtlogo.rc
