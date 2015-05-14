#ifndef LCDDAEMONMSGQUEUE_H
#define LCDDAEMONMSGQUEUE_H

#include <QThread>
#include <QMetaType>
#include <QVariant>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "sand_lcd.h"

#define ID_UP_FONT      20
#define ID_TF_DETECT    21
#define ID_CHRG_DETECT  22
#define ID_CHAG_BAR     23
#define ID_BATT_VALUE   24
#define ID_PRN_HOT      25
#define ID_USBDISK_DETECT    26
#define ID_SIGN_RT      27

#define WIDGET_LABEL        0
#define WIDGET_BUTTON       1
#define WIDGET_ICONBUTTON	2
#define MSG_LEN         100
#if 0
struct message_data
{
    int api_id;

    int line;
    int colum;

    int gprs_d;
    int gprs_s;
    int modem_dial;
    int r1;
//    int r2;
//    int r3;
//    int r4;

    int widget_type;
    int widget_key;
    int widget_info;

    char mtext[MSG_LEN];
};



struct lcd_message
{
    long  mtype;
    struct message_data msg_data;
};
#endif
Q_DECLARE_METATYPE(lcd_message)

class LcdDaemonMsgQueue : public QThread
{
    Q_OBJECT
public:
    void run();



private:
    key_t lcdKey;
    int createMsgQue(void);
signals:
    void newMessage(QVariant msgVar);
};

#endif // LCDDAEMONMSGQUEUE_H
