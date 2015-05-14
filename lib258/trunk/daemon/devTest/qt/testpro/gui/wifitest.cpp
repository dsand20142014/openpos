#include "wifitest.h"
#include <QtDebug>
extern "C"{
#include "oslib.h"
#include "toolslib.h"
#include "sand_network.h"
#include "test/test.h"
#include "devSignal.h"
}
#define  Hw_addr        0xff&buf[intrface].ifr_hwaddr.sa_data

extern void _print(char *fmt,...);

WifiTest::WifiTest(bool flag,QWidget *parent) :
    BaseForm(tr(" "))
{
    label = new QLabel(tr("IFCONFIG:"));
    label_2 = new QLabel(tr("HOST IP:"));
    label_3 = new QLabel(tr("PORT:"));
    label_5 = new QLabel();

    lineEdit_2 = new QLineEdit("21");
    lineEdit_2->setFixedHeight((30*screenHeight)/320);

    mt = new MyIpAddrEdit();
    mt->setFixedHeight((30*screenHeight)/320);
    mt->settext("172.16.1.1");

    lineEdit_2->installEventFilter(this);
    mt->installEventFilter(this);

    QTextEdit *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(10);
    vbox->addWidget(label);
    vbox->addWidget(label_2);
    vbox->addWidget(mt);
    vbox->addWidget(label_3);
    vbox->addWidget(lineEdit_2);
    vbox->addWidget(label_5);

    vbox->setContentsMargins(20,10,20,10);


    textEdit->setLayout(vbox);

    layout->addWidget(textEdit);

    mt->setFocus();

    btnlayout->addWidget(btnOk);

    btnOk->show();

    okflag = false;

    wifiFlag = flag;

    if(wifiFlag)
        title->setText(tr("WIFI CONNECT"));
    else
        title->setText(tr("GSM CONNECT"));

    connect(btnOk, SIGNAL(clicked()), this, SLOT(btnOk_clicked()));

    connect(&timer,SIGNAL(timeout()),this,SLOT(check_ip_port()));

    connect(btnExit,SIGNAL(clicked()),&timer,SLOT(stop()));

    getResult();
}

WifiTest::~WifiTest()
{

}

bool WifiTest::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==mt||obj==lineEdit_2)
    {
        if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *keyEve = static_cast<QKeyEvent*>(event) ;
            switch(keyEve->key())
            {
            case Qt::Key_Enter:
                if(mt->ip1Hasfocus())
                    mt->setip2focus();
                else if(mt->ip2Hasfocus())
                    mt->setip3focus();
                else if(mt->ip3Hasfocus())
                    mt->setip4focus();
                else if(mt->ip4Hasfocus())
                    lineEdit_2->setFocus();
                else if(lineEdit_2->hasFocus()){
                    if(btnOk->isEnabled())
                            btnOk->click();

                }

                return true;

            default:
                return false;
            }

        }
        else
            return false;
    }
    else
        return QObject::eventFilter(obj,event);

}

void WifiTest::getResult()
{
    struct _if ifr;
    char ip[52];
    int ret = 0;

    memset(&ifr, 0 ,sizeof(struct _if));
    if(wifiFlag){
        strcpy(ifr.name, "wlan0");

        int rec = Os__get_device_signal(DEV_WIFI);
        //qDebug()<<"rec==="<<rec;
        if(rec==0){
            label->setText(tr("WIFI DEVICE DID NOT OPEN!"));
             btnOk->setEnabled(false);

            return;
        }

    }
    else
        strcpy(ifr.name, "ppp0");


    int net = net_interface(&ifr);

    if(!net)
    {
        memset(ip,0,sizeof(ip));
        if(strlen(ifr.ip)==0){
            btnOk->setEnabled(false);

            if(!wifiFlag)
                label->setText(tr("GSM DEVICE DID NOT OPEN!"));
            return;
        }

        if(wifiFlag)
            ret=activeDev(DEV_COMM_WIFI);
        else
            ret = activeDev(DEV_COMM_GSM);

        sprintf(ip,"IFCONFIG: %s", ifr.ip);

        label->setText(ip);
    }
    else
    {

        label_5->setText(tr("GSM NOT READY,PLEASE CHECK GSM CARD!"));
        btnOk->setEnabled(false);

    }
}

void WifiTest::check_ip_port()
{
    btnOk->setEnabled(false);

    timer.stop();
    unsigned char ucResult;
    unsigned char aucIP[5];

    int iTCPHandle=0;



    ucResult=TCP_Open(&iTCPHandle);

    if(ucResult!=0x00){
        label_5->setText(tr("CONNECT FAILED!"));
    }

    QStringList ipList = mt->text().split(".");

    aucIP[0] = ipList.at(0).toInt();
    aucIP[1] = ipList.at(1).toInt();
    aucIP[2] = ipList.at(2).toInt();
    aucIP[3] = ipList.at(3).toInt();


    if(ucResult==0x00)
    {
        ucResult = TCP_Connect(iTCPHandle,tab_long(aucIP,4),lineEdit_2->text().toInt());
        if(ucResult!=0x00)
            label_5->setText(tr("CONNECTION TIMED OUT!"));      
        else
            label_5->setText(tr("CONNECT SUCCESS!"));
    }

    TCP_Close(iTCPHandle);

    btnOk->setEnabled(true);

}


void WifiTest::btnOk_clicked()
{
    if(timer.isActive()) return;

    label_5->setText(tr("CONNECT..."));

    timer.start(500);

}


int WifiTest::net_interface(struct _if *if_info)
{
    unsigned int intrface;//, i = 1;
    int fd;

    struct ifreq buf[16];
    struct ifconf ifc;
    //struct ifreq  ifr;

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error\n");
        return -1;
    }

    ifc.ifc_len = sizeof(buf) ;
    ifc.ifc_buf = (caddr_t) buf;

    if (ioctl (fd, SIOCGIFCONF, (char *) &ifc) < 0){
        perror("SIOCGIFCONF error\n");
        goto error ;
    }

    intrface = ifc.ifc_len / sizeof (struct ifreq);
    while (intrface-- > 0) {
        if (strcmp(if_info->name, buf[intrface].ifr_name))
            continue;

        if (ioctl (fd, SIOCGIFFLAGS, (char *) &buf[intrface]) < 0){
            perror("SIOCGIFFLAGS error\n");
            goto error;
        }

        if ( buf[intrface].ifr_flags & IFF_PROMISC)
            _print("the interface is PROMISC\n");

        if (buf[intrface].ifr_flags & IFF_UP){
            strcpy(if_info->state," up");
            _print("status is UP\n");
        }

        if ( ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface]) < 0) {
            perror("SIOCGIFADDR error\n");
            goto error;
        }
        //puts(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
        strcpy(if_info->ip , inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));


        if ( ioctl (fd, SIOCGIFBRDADDR, (char *) &buf[intrface]) < 0) {
            perror("SIOCGIFADDR error\n");
            goto error;
        }
        //puts(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_broadaddr))->sin_addr));
        strcpy(if_info->br ,inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_broadaddr))->sin_addr));


        if ( ioctl (fd, SIOCGIFNETMASK, (char *) &buf[intrface]) < 0) {
            perror("SIOCGIFADDR error\n");
            goto error;
        }
        //puts(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_netmask))->sin_addr));
        strcpy(if_info->netmsk , inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_netmask))->sin_addr));


        if ( ioctl (fd, SIOCGIFHWADDR, (char *) &buf[intrface]) < 0) {
            perror("SIOCGIFADDR error\n");
            goto error;
        }
        sprintf(if_info->mac,"%02x:%02x:%02x:%02x:%02x:%02x",Hw_addr[0],Hw_addr[1],Hw_addr[2],Hw_addr[3],Hw_addr[4],Hw_addr[5]);
    }

   ::close(fd);
    return 0;
error:
   ::close(fd);
    return -1;
}


