#-------------------------------------------------
#
# Project created by QtCreator 2012-06-13T17:12:12
#
#-------------------------------------------------

QT       += core gui

TARGET = 194
TEMPLATE = app
CONFIG+=thread

include($$PWD/sandpay/appsrc/appsrc.pri)
include($$PWD/sandpay/appinc/appinc.pri)
include ($$PWD/gui/gui.pri)



unix:!macx:!symbian: LIBS += -L$$PWD/sandpay/oslib/lib/ -liconv

INCLUDEPATH += $$PWD/sandpay/oslib
DEPENDPATH += $$PWD/sandpay/oslib

unix:!macx:!symbian: LIBS += -L$$PWD/sandpay/oslib/lib/ -lSMC_CHK

INCLUDEPATH += $$PWD/sandpay/oslib
DEPENDPATH += $$PWD/sandpay/oslib

unix:!macx:!symbian: LIBS += -L$$PWD/sandpay/oslib/lib/ -lVikang

INCLUDEPATH += $$PWD/sandpay/oslib
DEPENDPATH += $$PWD/sandpay/oslib

unix:!macx:!symbian: LIBS += -L$$PWD/sandpay/oslib/lib/ -liconv

INCLUDEPATH += $$PWD/sandpay/oslib
DEPENDPATH += $$PWD/sandpay/oslib

INCLUDEPATH += $$PWD/sandpay/appinc

unix:!macx:!symbian: LIBS += -L$$PWD/sandpay/oslib/lib/ -lapp

INCLUDEPATH += $$PWD/sandpay/oslib/osinc
DEPENDPATH += $$PWD/sandpay/oslib/osinc

unix:!macx:!symbian: LIBS += -L$$PWD/sandpay/oslib/lib/ -lcommsencrypt

INCLUDEPATH += $$PWD/sandpay/oslib/osinc
DEPENDPATH += $$PWD/sandpay/oslib/osinc






