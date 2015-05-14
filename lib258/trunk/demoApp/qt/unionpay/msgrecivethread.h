#ifndef MSGREVIVETHREAD_H
#define MSGREVIVETHREAD_H

#if 0
#include <QThread>
#include "new_drv.h"
//#include "osevent.h"
//#include "osdrv.h"

#define TRANS_ISO2LEN    38
#define TRANS_ISO2MINLEN 16
#define TRANS_ISO3LEN   128

#define ERR_MAG_TRACKDATA 02
#define  ERR_SANDMAGCARD  03

class MsgReciveThread:public QThread
{
    Q_OBJECT
public:
    MsgReciveThread();
    void stop();
protected:
    void run();
private:
    void reciveMsg(void);
    unsigned char MAG_GetTrackInfo(unsigned char *pucTrack);
};
#endif

#endif // MSGREVIVETHREAD_H
