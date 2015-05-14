#include <QtGui/QApplication>

#include "QTextCodec"
#include "msginfo.h"

extern "C"{
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <errno.h>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
//#include "osdrv.h"
//#include "sand_time.h"
//#include "smart.h"
//#include "sand_lcd.h"
//#include "sand_modem.h"
//#include "sand_gprs.h"
//#include "oscfg_param.h"


//#include "tools.h"

/*********以下三个头文件有问题*******/
//#include "toolslib.h"
//#include "osicc.h"
//#include "oslib.h"

//    void manager()
//    {

//    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    MsgInfo msg(1);
    msg.show();

//    Widget w;
//    w.show();
    
    return a.exec();
}
