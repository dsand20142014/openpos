#include "lcddaemonmsgqueue.h"
#include <QDebug>

void LcdDaemonMsgQueue::run()
{
    qRegisterMetaType<QVariant>("QVariant");
    struct lcd_message msg;
    QVariant msgVar;
    lcdKey=1300;
    mqId=createMsgQue();
    while(1)
    {
        if (msgrcv(mqId, &msg, 140, 50, 0)<0 )
        {
            perror("&&&& message receive &&&&\n");
            exit(-1);
        }
        msgVar.setValue(msg);
        emit newMessage(msgVar);
    }
}

int LcdDaemonMsgQueue::createMsgQue()
{
    int id;
    if ((id=msgget(lcdKey ,0777|IPC_CREAT))<0)
    {
        qDebug(" %s msgget1 fail.\n",__func__);
        exit(EXIT_FAILURE);
    }

    /*firstly remove it */
    if (msgctl(id,IPC_RMID,NULL)<0)
    {
        qDebug(" %s msgctl fail.\n",__func__);
        exit(EXIT_FAILURE);
    }

    /*create it again */
    if ((id=msgget(lcdKey ,0777|IPC_CREAT))<0)
    {
        qDebug(" %s msgget2 fail.\n",__func__);
        exit(EXIT_FAILURE);
    }

/*    if ((msqId=msgget(SPD_SMG_KEY ,0777|IPC_CREAT))<0)
    {
        qDebug(" %s msgget SPD_SMG_KEY fail.\n",__func__);
        exit(EXIT_FAILURE);
    } else {

        memset(&msgSMT, 0, sizeof(msgSMT));
    }
*/
    return id;
}
