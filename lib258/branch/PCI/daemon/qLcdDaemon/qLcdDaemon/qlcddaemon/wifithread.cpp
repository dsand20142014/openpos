#include "wifithread.h"
//#include <QDebug>
extern "C"
{
#include "sand_key.h"
#include "sysglobal.h"
#include "modules.h"
#include "devSignal.h"
//#include "sysparam.h"
}
void WifiThread::run()
{
    /* set main form disabled */
    emit disableAll(true);
    Os__set_key_ensure(KEY_SYS_SET);

    SYSGLOBAL_sys_suspend();

    //Os__set_key_ensure(KEY_SYSWIN_EXIT);
    /* set main form enabled */
    emit disableAll(false);

    /*因为唤醒后打开wifi所需时间太长所以放在单独的线程里做*/
    SYSGLOBAL_sys_awake();
}
