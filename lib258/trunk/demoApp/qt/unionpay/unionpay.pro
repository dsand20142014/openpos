FORMS += \
    frame.ui \
    cardnumdisplay.ui \
    mainmenu.ui \
    mainkey9.ui \
    mainkey91.ui \
    mainkey92.ui \
    mainkey93.ui \
    mainkey94.ui \
    mainkey8.ui \
    mainkey81.ui \
    mainkey82.ui \
    mainkey7.ui \
    mainkey6.ui \
    mainkey5.ui \
    mainkey4.ui \
    mainkey931.ui \
    pro_debit.ui \
    msginfo.ui \
    pro_cancel.ui \
    pro_preauth.ui \
    pro_authfinish.ui \
    pro_cancelauth.ui \
    pro_cancelauthfinish.ui \
    pro_51_authfinishoff.ui \
    pro_authfinishmsg.ui \
    pro_52_tipadjust.ui \
    pro_53_offcancel.ui \
    pro_refund.ui \
    pro_quarybal.ui \
    pro_reprint.ui \
    pro_settle.ui \
    mainkey73.ui

HEADERS += \
    frame.h \
    cardnumdisplay.h \    
    emv/appinc/xdata.h \
    emv/appinc/util.h \
    emv/appinc/smart.h \
    emv/appinc/rs232yth.h \
    emv/appinc/py_input_config.h \
    emv/appinc/py_input.h \
    emv/appinc/print.h \
    emv/appinc/pc_download.h \
    emv/appinc/newsync.h \
    emv/appinc/msg.h \
    emv/appinc/masapp.h \
    emv/appinc/loadkey.h \
    emv/appinc/iso8583.h \
    emv/appinc/include.h \
    emv/appinc/global.h \
    emv/appinc/file.h \
    emv/appinc/emvparam.h \
    emv/appinc/emvinterface.h \
    emv/appinc/display.h \
    emv/appinc/digital.h \
    emv/appinc/commsencrypt.h \
    emv/appinc/asc_table_config.h \
    emv/appinc/asc_table.h \           
    mainmenu.h \
    mainkey9.h \
    mainkey91.h \
    mainkey92.h \
    mainkey93.h \
    mainkey94.h \
    mainkey8.h \
    mainkey81.h \
    mainkey82.h \
    mainkey7.h \
    mainkey6.h \
    mainkey5.h \
    mainkey4.h \
    mainkey931.h \
    emv/interfaceinc/interfaceparam.h \
    emv/interfaceinc/interface.h \
    emv/qemvinc/qvsdc.h \
    emv/qemvinc/qverify.h \
    emv/qemvinc/QTypeDef.h \
    emv/qemvinc/qtranstag.h \
    emv/qemvinc/qtermriskmanage.h \
    emv/qemvinc/qtermconfig.h \
    emv/qemvinc/qtermactanalysis.h \
    emv/qemvinc/qtag.h \
    emv/qemvinc/qreadapp.h \
    emv/qemvinc/qprocessrestrict.h \
    emv/qemvinc/qpboc.h \
    emv/qemvinc/qlib.h \
    emv/qemvinc/QInitApp.h \
    emv/qemvinc/qicc.h \
    emv/qemvinc/qerror.h \
    emv/qemvinc/qdef.h \
    emv/qemvinc/qddfstack.h \
    emv/qemvinc/qdataelement.h \
    emv/qemvinc/qdataauth.h \
    emv/qemvinc/qdata.h \
    emv/qemvinc/qcardswdef.h \
    emv/qemvinc/qappselect.h \
    emv/qemvinc/qappcrypt.h \
    emv/qemvinc/msd.h \
    pro_debit.h \
    mythread.h \
    msginfo.h \
    pro_cancel.h \
    pro_preauth.h \
    pro_authfinish.h \
    pro_cancelauth.h \
    pro_cancelauthfinish.h \
    pro_51_authfinishoff.h \
    pro_authfinishmsg.h \
    pro_52_tipadjust.h \
    pro_53_offcancel.h \
    pro_refund.h \
    pro_quarybal.h \
    pro_reprint.h \
    pro_settle.h \
    mainkey73.h \
    mainentrythread.h


SOURCES += \
    main.cpp \
    frame.cpp \
    cardnumdisplay.cpp \    
    mainmenu.cpp \
    mainkey9.cpp \
    mainkey91.cpp \
    mainkey92.cpp \
    mainkey93.cpp \
    mainkey94.cpp \
    mainkey8.cpp \
    mainkey81.cpp \
    mainkey82.cpp \
    mainkey7.cpp \
    mainkey6.cpp \
    mainkey5.cpp \
    mainkey4.cpp \
    mainkey931.cpp \
    emv/appsrc/xdata.c \
    emv/appsrc/util.c \
    emv/appsrc/trans.c \
    emv/appsrc/syscash.c \
    emv/appsrc/smart.c \
    emv/appsrc/serv.c \
    emv/appsrc/sel.c \
    emv/appsrc/sav.c \
    emv/appsrc/rs232yth.c \
    emv/appsrc/py_input.c \
    emv/appsrc/print.c \
    emv/appsrc/port.c \
    emv/appsrc/pinpad.c \
    emv/appsrc/pc_download.c \
    emv/appsrc/pboc.c \
    emv/appsrc/msg.c \
    emv/appsrc/menu.c \
    emv/appsrc/masapp.c \
    emv/appsrc/manager.c \
    emv/appsrc/mag.c \
    emv/appsrc/logon.c \
    emv/appsrc/loadkey.c \
    emv/appsrc/key.c \
    emv/appsrc/iso8583.c \
    emv/appsrc/interfaceparam.c \
    emv/appsrc/interface.c \
    emv/appsrc/global.c \
    emv/appsrc/file.c \
    emv/appsrc/extprint.c \
    emv/appsrc/evecom.c \
    emv/appsrc/emvparam.c \
    emv/appsrc/emvinterface.c \
    emv/appsrc/digital.c \
    emv/appsrc/des.c \
    emv/appsrc/comms.c \
    emv/appsrc/cfg.c \
    emv/appsrc/cash.c \
    emv/appsrc/asc_table.c \        
    emv/exsrc/exkey.c \
    emv/exsrc/extrans.c \
    emv/exsrc/OWE_GetCardinfo.c \
    pro_debit.cpp \
    mythread.cpp \
    msginfo.cpp \
    pro_cancel.cpp \
    pro_preauth.cpp \
    pro_authfinish.cpp \
    pro_cancelauth.cpp \
    pro_cancelauthfinish.cpp \
    pro_51_authfinishoff.cpp \
    pro_authfinishmsg.cpp \
    pro_52_tipadjust.cpp \
    pro_53_offcancel.cpp \
    pro_refund.cpp \
    pro_quarybal.cpp \
    pro_reprint.cpp \
    pro_settle.cpp \
    mainkey73.cpp \
    msgrecivethread.cpp \
    mainentrythread.C

unix:!macx:!symbian: LIBS += -L$$PWD/oslib/lib/ -lemv

INCLUDEPATH += $$PWD/emv/emvinc
DEPENDPATH += $$PWD/emv/emvinc

unix:!macx:!symbian: LIBS += -L$$PWD/oslib/lib/ -lvisawave

INCLUDEPATH += $$PWD/emv/qemvinc
DEPENDPATH += $$PWD/emv/qemvinc

unix:!macx:!symbian: LIBS += -L$$PWD/oslib/lib/ -lcommsencrypt

INCLUDEPATH += $$PWD/oslib
DEPENDPATH += $$PWD/oslib

unix:!macx:!symbian: LIBS += -L$$PWD/../../../../appLib/rel/ -lapp

INCLUDEPATH += $$PWD/oslib/osinc
DEPENDPATH += $$PWD/oslib/osinc
#INCLUDEPATH += /mnt/hgfs/qtwork/unionpay/emv/appinc
#INCLUDEPATH += /mnt/hgfs/qtwork/unionpay/emv/interfaceinc
#INCLUDEPATH += /mnt/hgfs/qtwork/unionpay/emv/qemvinc
#INCLUDEPATH += /home/ubuntu/Desktop/unionpay/unionpay/emv/appinc
#INCLUDEPATH += /home/ubuntu/Desktop/unionpay/unionpay/emv/interfaceinc
#INCLUDEPATH += /home/ubuntu/Desktop/unionpay/unionpay/emv/qemvinc
INCLUDEPATH += $$PWD/emv/appinc
INCLUDEPATH += $$PWD/emv/interfaceinc
INCLUDEPATH += $$PWD/emv/qemvinc

unix:!macx:!symbian: LIBS += -L$$PWD/oslib/lib/ -liconv

INCLUDEPATH += $$PWD/oslib
DEPENDPATH += $$PWD/oslib
