#include "apntest.h"

extern "C"{
#include "sand_gprs.h"
#include "devSignal.h"
#include "ostime.h"
#include "sysparam.h"
#include "sand_network.h"
#include "test/test.h"
#include "gsm.h"
}
#define  Hw_addr        0xff&buf[intrface].ifr_hwaddr.sa_data

extern void _print(char *fmt,...);

ApnTest::ApnTest(QWidget *parent) :
    BaseForm(tr("GSM STATUS TEST"),parent)
{
    flag = false;
    textEdit = new QTextEdit;
    layout->addWidget(textEdit);

    textEdit->setText(tr("PRESS OK TO TEST!"));

    textEdit->setReadOnly(true);

    btnOk->show();
    btnOk->setFocus();
    btnlayout->addWidget(btnOk);

    memset(apn,0,sizeof(apn));
    memset(band,0,sizeof(band));

    getapnband();

    connect(btnOk,SIGNAL(clicked()),this,SLOT(getResult()));

}

void ApnTest::getapnband()
{
    int i = 0;
    int cn = 0;

    SYS_SETTING sysSetting;

    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"DEFAULT_APN");
    cn = PARAM_setting_get(&sysSetting);

    if(!QString(sysSetting.value).isEmpty())
    {
        for(i=0;i<cn;i++)
        {
            if(strcmp(sysSetting.value,sysSetting.briefList[i]) == 0)
                break;
        }
        if(i >= cn)
            memcpy(apn,sysSetting.briefList[atoi(sysSetting.value)-1],50);
        else
          memcpy(apn,sysSetting.value,50);

     }


    memset(&sysSetting,0,sizeof(SYS_SETTING));
    strcpy(sysSetting.section,"GSM_SETTING");
    strcpy(sysSetting.key,"DEFAULT_BAND");
    PARAM_setting_get(&sysSetting);
    memcpy(band,sysSetting.briefList[atoi(sysSetting.value) -1],50);

}


//void ApnTest::keyPressEvent(QKeyEvent *e)
//{
//    switch(e->key())
//    {
//    case Qt::Key_Enter:
//        getResult();
//        break;
//    case Qt::Key_Escape:
//        close();
//        break;
//    }
//}




ApnTest::~ApnTest()
{
    GSM_set_APN((uchar*)apn);
}


void ApnTest::apnConnect()
{
    uchar ucResult = 0;

    while(1)
    {
        int ret=access("/var/lock/LCK..ttymxc1",F_OK);

        if(ret==0)
        {
            if (!GSM__dial_check(1))
            {
                system("/etc/ppp/ppp-off");
            }
        }
        else
        {
           break;
        }
        Os__xdelay(50);
    }


    if(!flag)
        ucResult =  GSM_set_APN((uchar*)"CMNET");
    else
       ucResult =  GSM_set_APN((uchar*)"MPOS-CUPSH.SH");


    if( !ucResult )
    {
      Os__xdelay(100);
      ucResult = GSM__dial_check(1200);
    }


    if(!ucResult){
        struct _if ifr;
        char ip[52];

        memset(&ifr, 0 ,sizeof(struct _if));

        strcpy(ifr.name, "ppp0");

        int net = net_interface(&ifr);

        _print("net***=%d\n",net);

        if(!net)
        {
            memset(ip,0,sizeof(ip));
            sprintf(ip,"%s", ifr.ip);
            _print("ifr.ip***=%s\n",ifr.ip);
            _print("ip***=%s\n",ip);
             textEdit->append("IP:"+QString(ip));
        }

       textEdit->append(tr("TEST SUCCESS!"));

       if(flag){
           showInforText(tr("TEST SUCCESS!"),false);

           QTimer::singleShot(2000,informationBox,SLOT(hide()));
           btnOk->setEnabled(true);

           return;
       }

       flag = true;

       textEdit->append(tr("APN : MPOS-CUPSH.SH"));
       textEdit->append(tr("BAND : %1").arg(band));
       textEdit->append(tr("REDIAL..."));

       QTimer::singleShot(3000,this,SLOT(apnConnect()));

    }
    else{
        textEdit->append(tr("TEST FAILED!"));
        btnOk->setEnabled(true);
    }

}


int ApnTest::net_interface(struct _if *if_info)
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

void ApnTest::getResult()
{
    btnOk->setEnabled(false);
    flag = false;
    textEdit->setText(tr("APN : CMNET"));
    textEdit->append(tr("BAND : %1").arg(band));
    textEdit->append(tr("REDIAL..."));

    QTimer::singleShot(10,this,SLOT(apnConnect()));

}
