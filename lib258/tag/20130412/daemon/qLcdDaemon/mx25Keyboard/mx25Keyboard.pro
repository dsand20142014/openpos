#-------------------------------------------------
#
# Project created by QtCreator 2012-04-17T13:48:34
#
#-------------------------------------------------


TEMPLATE = lib
isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.8.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
CONFIG += qt plugin

QT       -= gui

TARGET = mx25Keyboard
TARGET = $$qtLibraryTarget($$TARGET)
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

DEFINES += MX25KEYBOARD_LIBRARY

SOURCES += \
	mx25keyboardplugin.C \
	mx25keyboardhandler.C

HEADERS +=\
	mx25keyboardplugin.h \
	mx25keyboardhandler.h \
	mx25Keyboardhandler_global.h \
	scancode.h

symbian {
	MMP_RULES += EXPORTUNFROZEN
	TARGET.UID3 = 0xEDD89D4E
	TARGET.CAPABILITY =
	TARGET.EPOCALLOWDLLDATA = 1
	addFiles.sources = mx25Keyboard.dll
	addFiles.path = !:/sys/bin
	DEPLOYMENT += addFiles
}

unix:!symbian {
	maemo5 {
		target.path = /opt/usr/lib
	} else {
		target.path = /usr/lib
	}
	INSTALLS += target
}
