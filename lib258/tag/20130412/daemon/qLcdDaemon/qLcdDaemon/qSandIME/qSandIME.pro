#-------------------------------------------------
#
# Project created by QtCreator 2012-04-26T09:25:44
#
#-------------------------------------------------

TARGET = qSandIME
TEMPLATE = lib

DEFINES += QSANDIME_LIBRARY

SOURCES += \
    googlepinyin/utf16reader.cpp \
    googlepinyin/utf16char.cpp \
    googlepinyin/userdict.cpp \
    googlepinyin/sync.cpp \
    googlepinyin/splparser.cpp \
    googlepinyin/spellingtrie.cpp \
    googlepinyin/spellingtable.cpp \
    googlepinyin/searchutility.cpp \
    googlepinyin/qinputwidget.C \
    googlepinyin/qimeframe.C \
    googlepinyin/pinyinime.cpp \
    googlepinyin/pinyindialog.C \
    googlepinyin/ngram.cpp \
    googlepinyin/mystdlib.cpp \
    googlepinyin/matrixsearch.cpp \
    googlepinyin/lpicache.cpp \
    googlepinyin/dicttrie.cpp \
    googlepinyin/dictlist.cpp \
    googlepinyin/dictbuilder.cpp \
    qclickablelabel.C

HEADERS += \
    googlepinyin/utf16reader.h \
    googlepinyin/utf16char.h \
    googlepinyin/userdict.h \
    googlepinyin/sync.h \
    googlepinyin/splparser.h \
    googlepinyin/spellingtrie.h \
    googlepinyin/spellingtable.h \
    googlepinyin/searchutility.h \
    googlepinyin/qinputwidget.h \
    googlepinyin/qimeframe.h \
    googlepinyin/pinyinime.h \
    googlepinyin/pinyindialog.h \
    googlepinyin/ngram.h \
    googlepinyin/mystdlib.h \
    googlepinyin/matrixsearch.h \
    googlepinyin/lpicache.h \
    googlepinyin/dicttrie.h \
    googlepinyin/dictlist.h \
    googlepinyin/dictdef.h \
    googlepinyin/dictbuilder.h \
    googlepinyin/atomdictbase.h \
    qclickablelabel.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE7A34E05
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = qSandIME.dll
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

OTHER_FILES += \
    googlepinyin/dict_pinyin.dat

FORMS += \
    googlepinyin/qinputwidget.ui \
    googlepinyin/pinyindialog.ui
