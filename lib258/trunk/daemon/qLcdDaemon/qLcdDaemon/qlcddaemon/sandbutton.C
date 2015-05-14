#include "sandbutton.h"
#include <QToolButton>
#include <sys/ipc.h>
#include <sys/shm.h>

extern "C"
{
#include "sand_key.h"
}

#define CURRENT_APP_SHM_ID  67835

SandButton::SandButton(QWidget *parent) :
    QToolButton(parent)
{
    connect(this,SIGNAL(pressed()),this,SLOT(send_key()));

}

void SandButton::send_key()
{
    char c;
    int * shm_addr;
    int shmid;
    int curret_app;
    c = this->key_value+'0';

    shmid=shmget( CURRENT_APP_SHM_ID,8096,0666|IPC_CREAT );

    if(shmid < 0)
    {
        qDebug("### shmget failed.");
    }
    shm_addr = (int *)shmat(shmid,NULL,0);
    curret_app = 1;
    memcpy(shm_addr, &curret_app, sizeof(int));
//    Os__set_key(c);
}
