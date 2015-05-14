/**************************************************
 * lcddaemon.C
 * Main window for QLCDdaemon including the Topbar,
 *     Sliding view, SandDisplay and Gif window.
 * Created by wyl 2012, modified by xl 20130507
 **************************************************/

#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
//#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <googlepinyin/pinyindialog.h>
#include <googlepinyin/pinyinime.h>
#include "lcddaemon.h"
#include "authorization.h"

#define MAX_APP_COUNT 36
#define PRINTER_IOC_MAGIC       'S'
#define PRINTER_FEED_PAPER      _IOW(PRINTER_IOC_MAGIC,  1, int)
#define STATUS_CNT_MAX  40
#define CURRENT_APP_SHM_ID  67835
#define sandpos

/*
 * MSG API ID define for qlcddaemon
 * Add by xiangliu 2013.01.24
 */
#define MSG_ID_GPS      5
#define MSG_ID_GPRS     10
#define MSG_ID_LOW_BATTERY    11
#define MSG_ID_PRINTER  30
#define MSG_ID_WIFI     15
#define MSG_ID_SETDATE  16
#define MSG_ID_APP_RUN  40
#define MSG_ID_APP_EXIT  9
#define MSG_ID_APP_PIN  99
#define MSG_ID_APP_TIMEOUT  100
#define MSG_ID_AUTHORIZE  88
#define MSG_ID_ATTACK  77
#define ID_UP_FONT      20
#define ID_TF_DETECT    21
#define ID_CHRG_DETECT  22
#define ID_CHAG_BAR     23
#define ID_BATT_VALUE   24
#define ID_PRN_HOT      25
#define ID_USBDISK_DETECT    26

extern "C"
{
#include "sand_key.h"
#include "applist.h"
#include "param.h"

int manager(void *p); // for old api
int manager(void *p){Q_UNUSED(p);return 0;}

/* xl add for smt */
int msqId;
UP_SER_MSG msgSMT;
extern int mqId;
}

using namespace ime_pinyin;

LcdDaemon::LcdDaemon(QWidget *parent, QStringList *argv) :
    QWidget(parent)

{
    int i,j;
    start=NULL;
    myloadgifWin = NULL;   
    pinForm = NULL;
    STOP_SLEEP = false;

    sysArgv = new QStringList(*argv);

    if ((msqId=msgget(SPD_SMG_KEY ,0777|IPC_CREAT))<0)
    {
        qDebug(" %s msgget SPD_SMG_KEY fail.\n",__func__);
        exit(EXIT_FAILURE);
    } else {
        memset(&msgSMT, 0, sizeof(msgSMT));
    }

    paral_show = false;
    keyEnable = true;
    this->slideScreen = NULL;

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);

    isStart = false;
    isStart_timer = new QTimer(this);
    connect(isStart_timer, SIGNAL(timeout()), this, SLOT(start_win()));

    showFullScreen();

    top_form = new TopForm();
    top_form->setGeometry(0, 0, 240, 25);
    top_form->label_power->setPixmap(QPixmap("/daemon/image/battery0.png"));//xl

    APPINFOLIST applist[MAX_APP_COUNT];
    g_app_cnt = Os__applist_get(applist, MAX_APP_COUNT);//APPLIST_list_load();//loadAppList();

    j = 0;
    for(i = 0; i < g_app_cnt; i++)
    {        
        if((applist[i].hidden[0] == '0')&&atoi((char *)applist[i].id) > 9)
        {
            j++;
        }
    }
    g_app_cnt = j;
//    printf("                qt g_app_cnt %d\n", g_app_cnt);
    g_app_curret = 0;

    QPushButton *btnOk = new QPushButton(tr("Ok"));
    btnOk->setMinimumSize(70, 30);

    printer_state_box.setWindowFlags(Qt::FramelessWindowHint);
    printer_state_box.setGeometry(0, 100, 240, 320-200);
    printer_state_box.setAlignment(Qt::AlignCenter);
    printer_state_box.setStyleSheet("color: red; background-color: rgb(151, 205, 246); border-style: solid; border-width: 2px; border-color: gray;");
    printer_state_box.hide();

//    battery_state_box.addButton(btnOk, QMessageBox::YesRole);
//    battery_state_box.setIcon(QMessageBox::Information);
//    battery_state_box.setWindowFlags(Qt::CustomizeWindowHint);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeupdate()));
    timer->start(1000);

    message = new QLabel(this);
    message->setText(tr("请进入SETTING设置日期和时间!"));
    message->setAlignment(Qt::AlignCenter);
    message->setGeometry(0,25,240,320-25);
    message->hide();

    powerForm = new PowerOff(); //xl add for poweroff form 030325
    powerForm->hide();
    connect(&(powerForm->sleepThread), SIGNAL(disableAll(bool)), this, SLOT(slot_setDisabled(bool)));
    connect(powerForm->btn_exit, SIGNAL(clicked()), this, SLOT(slot_syswin_exit()));

    qsocketserverthread = new SocketServerThread();
    connect(qsocketserverthread,SIGNAL(gifLoadingSign(struct gif_loading_struct *)), this, SLOT(gifloadopt(struct gif_loading_struct *)));
    qsocketserverthread->start();


#ifdef sandpos
    QWSInputMethod* im = new IMFrame();
    QWSServer::setCurrentInputMethod(im);
#endif

    myloadgifWin = new LoadGif();
    myloadgifWin->hide();

    sandDisp = new SandDisplay(this);
    sandDisp->setGeometry(0, 25, 240, 320-25);
    sandDisp->hide();
}

void LcdDaemon::gifloadopt(gif_loading_struct *gif_content)
{
    myloadgifWin->gif_show(gif_content);
}

void LcdDaemon::start_win()
{
    isStart = false;

    isStart_timer->stop();
}

void LcdDaemon::keyReleaseEvent(QKeyEvent *e)
{
    if (!keyEnable)
        return;

    restartSleepTimer();

    if(e->key() == Qt::Key_Escape)
    {
        isStart = false;
    }

    switch(e->key())
    {
//    case Qt::Key_Enter:
//        Os__set_key_ensure(KEY_ENTER);
//        break;
    case Qt::Key_1:
        Os__set_key_ensure('1');
        break;
    case Qt::Key_2:
        Os__set_key_ensure('2');
        break;
    case Qt::Key_3:
        Os__set_key_ensure('3');
        break;
    case Qt::Key_4:
        Os__set_key_ensure('4');
        break;
    case Qt::Key_5:
        Os__set_key_ensure('5');
        break;
    case Qt::Key_6:
        Os__set_key_ensure('6');
        break;
    case Qt::Key_7:
        Os__set_key_ensure('7');
        break;
    case Qt::Key_8:
        Os__set_key_ensure('8');
        break;
    case Qt::Key_9:
        Os__set_key_ensure('9');
        break;
    default:
        //Os__set_key_ensure(200);
        break;
    }
}

void LcdDaemon::keyPressEvent(QKeyEvent *e)
{   
    if (!keyEnable)
        return;

    restartSleepTimer();

    switch(e->key())
    {
    case Qt::Key_Escape:
        //if(paral_show)
        {
            isStart = true;
        }
        break;
    case Qt::Key_0:
        if(isStart)
        //if(paral_show && isStart)
        {
            SysWindow *starttmp;
            isStart = false;
            isStart_timer->stop();

            starttmp=new SysWindow(0, sysArgv);
            starttmp->showMaximized();

            Os__set_key_ensure(KEY_SYS_SET); //to tell linuxpos not to wait event
            connect(starttmp, SIGNAL(destroyed()), this, SLOT(slot_syswin_exit()));//restartSleepTimer()));
        }

        break;

    case Qt::Key_PowerOff:
        if (powerForm->isHidden()) {
            powerForm->showMaximized();
            Os__set_key_ensure(KEY_SYS_SET);
        }
        break;

    case Qt::Key_F9://feed
        {
            int fd ;
            int line=80;
            fd = ::open("/dev/printer", O_RDWR) ;
            if (fd < 0 )
            {
                qDebug()<<"Feed Key can't open printer.";
            }
            ::ioctl(fd,PRINTER_FEED_PAPER,&line);
            ::close(fd);
        }
        break;
    default:
        break;
    }

}

/*
 * If enabled, send msg to linuxpos to wait event.
 */
void LcdDaemon::slot_setDisabled(bool disable)
{
    setDisabled(disable);

    if (!disable) {
        struct lcd_message msg;
        msg.msg_data.gprs_d = 3;
        sendmsg2linuxpos(&msg);
    }
}

void LcdDaemon::slot_syswin_exit()
{
    struct lcd_message msg;

    restartSleepTimer();
    msg.msg_data.gprs_d = 3;
    sendmsg2linuxpos(&msg);
}

void LcdDaemon::sendmsg2linuxpos(struct lcd_message *msg)
{
    msg->mtype = 49;
    if (msgsnd(mqId, msg, 140, 0) < 0) {
        perror("<sendmsg2linuxpos> msgsnd error");
    }
}

void LcdDaemon::timeupdate()
{
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm");
//    if ((time.second() % 2) == 0)
//        text[2] = ' ';

    top_form->label_time->setText(text);
}


LcdDaemon::~LcdDaemon()
{
    im_close_decoder();
}

void LcdDaemon::lcdMsgProc(QVariant msgVar)
{    
    struct lcd_message msg;
    int * shm_addr;
    int shmid;
    int curret_app;    
    int ret = 0;

    shmid=shmget( CURRENT_APP_SHM_ID,8096,0666|IPC_CREAT );

    if(shmid < 0)
    {
        qDebug("### shmget failed.");
    }
    shm_addr = (int *)shmat(shmid,NULL,0);
    if(this->isActiveWindow())
        curret_app = 1;
    else
        curret_app = -1;

    memcpy(shm_addr, &curret_app, sizeof(int));

    shmdt(shm_addr);

    msg = msgVar.value<struct lcd_message>();
    //    qDebug("%d,%d,%d,%s,[type]%d,[key]%d",msg.msg_data.api_id,
    //           msg.msg_data.line,msg.msg_data.colum,msg.msg_data.mtext,msg.msg_data.widget_type,msg.msg_data.widget_key);
    //    qDebug("###########%d###########\n", msg.msg_data.api_id);

    switch(msg.msg_data.api_id)
    {
    case 0:
        //qDebug("\n       GET 0!");
        showDisp();        
        sandDisp->display_GB2312(msg.msg_data.line, 0, (char *)msg.msg_data.mtext);
        break;

    case 1:
        //qDebug("\n       GET 1! %s", msg.msg_data.mtext);
        showDisp();
        sandDisp->display_GB2312(msg.msg_data.line, msg.msg_data.colum, (char *)msg.msg_data.mtext);
        break;

    case 2:
        //qDebug("\n       GET 2!");
        showDisp();
        sandDisp->display_text(msg.msg_data.line, msg.msg_data.colum, (char *)msg.msg_data.mtext);
        break;

#ifdef TEST
    case 8:
        if (sandDisp->isHidden())
            sandDisp->show();
        sandDisp->display_text(msg.msg_data.line, msg.msg_data.colum, msg.msg_data.mtext);
        sandDisp->display_text(msg.msg_data.line, 8, msg.msg_data.mtext);
        break;
#else
    case 8:
#endif
    case 20:
        setWidgetText((char *)msg.msg_data.mtext, msg.msg_data.widget_type,msg.msg_data.widget_info);
        break;
    case 3:
        //qDebug("\n       GET 3!");
        if (sandDisp->isVisible()) {
            //emit display(3);
            sandDisp->display_clear(msg.msg_data.line);
        }
        break;
    case 4:
 //       Os__light_on();
        break;

    case 6:
        this->update();
        break;

    case 7:
    case 13:
    case 14:
        //qDebug("\n       GET 7/13/14!");
        showDisp();
        sandDisp->display_image(msg.msg_data.line, msg.msg_data.colum, (char *)msg.msg_data.mtext);
        break;

    case MSG_ID_GPS:
        //GPS
        ret = msg.msg_data.gprs_d;

        if (ret)            
            top_form->label_gps->setPixmap(QPixmap("image/gps.png"));
        else
            top_form->label_gps->clear();
        break;

    case MSG_ID_GPRS:
    {   //GPRS
        ret = msg.msg_data.gprs_d;

        if (ret == -1) { // no sim
            top_form->label_signal->setPixmap(QPixmap("image/nosim.png"));
            top_form->label_gprs->clear();
        }
        else if (ret == 0) {
            top_form->label_gprs->clear();
            top_form->label_signal->setPixmap(QPixmap("image/signal0.png"));
        }
        else if (ret >= 1 && ret <= 5)//1表示无信号，2-5表示信号
        {
            char imageName[20];
            memset(imageName, 0, 20);
            sprintf(imageName,"image/signal%d.png", ret-1);//xl

            top_form->label_gprs->clear();            
            top_form->label_signal->setPixmap(QPixmap(imageName));
        }
		else if (ret >= 11 && ret <= 15)//2-5表示信号
        {
            char image[20];
            memset(image, 0, 20);
            sprintf(image,"image/signal%d.png", ret-11);//xl

            top_form->label_gprs->setPixmap(QPixmap("image/g.png"));
            top_form->label_signal->setPixmap(QPixmap(image));
        }
    }
        break;

    case MSG_ID_LOW_BATTERY:

        /*1表示锂电池,2表示纽扣电池*/
        if (msg.msg_data.gprs_d == 1) {
            top_form->label_power->setPixmap(QPixmap("image/battery_red.png"));
            /*if (!battery_state_box.isActiveWindow()) {
                battery_state_box.setText(tr("Low battery!\nPlease insert the power supply!"));
                battery_state_box.exec();
            }*/
        }
        else if (msg.msg_data.gprs_d == 2) {
            top_form->label_power->setPixmap(QPixmap("image/battery_red.png"));
            /*if (printer_state_box.isHidden()) {
                printer_state_box.setText(tr("Low battery!\nPlease replace the backup battery!"));
                printer_state_box.show();
                printer_state_box.exec();
            }*/
        }

        break;

    case ID_TF_DETECT:
        ret = msg.msg_data.gprs_d;

        if (msg.msg_data.gprs_d)
            //qt_display_image(0,0,"sd.png");
            top_form->label_sd->setPixmap(QPixmap("image/sd.png"));
        else
            top_form->label_sd->clear();
        break;

    case ID_CHRG_DETECT: // no use

        if (msg.msg_data.gprs_d)
            top_form->label_connect->setPixmap(QPixmap("image/connect.png"));
        else
            top_form->label_connect->clear();
        break;

    case 23:
    case ID_BATT_VALUE:
    {        
        ret = msg.msg_data.gprs_d;

        char jpg_filename[20];
        memset(jpg_filename, 0, 20);
        if (ret>=0 && ret<=4)
        {
            sprintf(jpg_filename,"image/battery%d.png", ret);
            top_form->label_power->setPixmap(QPixmap(jpg_filename));
        }
        break;
    }
#if 0
    case ID_PRN_HOT: // no use
        if (msg.msg_data.gprs_d == 0)
            qt_display_image(0,0,"logo.png");
        else if (msg.msg_data.gprs_d & 0x01) // no paper
            qt_display_image(0,0,"prn_nop.png");
        else if (msg.msg_data.gprs_d & 0x02) // hot
            qt_display_image(0,0,"prn_hot.png");
        break;
#endif

    case ID_USBDISK_DETECT:
        ret = msg.msg_data.gprs_d;

        if (ret)
            top_form->label_usb->setPixmap(QPixmap("usb.png"));
        else
            top_form->label_usb->clear();
        break;

    case MSG_ID_PRINTER:
        printer_state_box.hide();

        ret = msg.msg_data.gprs_d;

        if (ret == 0)
        {            
            top_form->label_nopaper->clear();
        } else {
            if (ret == 1 || ret == 3)
            {
                top_form->label_nopaper->setPixmap(QPixmap("image/nopaper.png"));
            }
            if (ret == 2 || ret == 3)
            {
                printer_state_box.setText("PRINTER HOT !");
                printer_state_box.show();
            }
        }
        break;

    case MSG_ID_WIFI:
    {
        ret = msg.msg_data.gprs_d;

        //0表示wifi模块关闭,1表示wifi模块打开但未连接,2~5表示wifi信号强度(devSignal.h)
        if (ret && ret >= 1 && ret <= 5)
        {            
            if (ret == 1) {//正在拨号
                top_form->label_wifi->setPixmap(QPixmap("image/wifi0.png"));
            } else {
                char wifi_filename[20];
                sprintf(wifi_filename, "image/wifi%d.png", ret-1);
                top_form->label_wifi->setPixmap(QPixmap(wifi_filename));
            }
        } else
        {
            top_form->label_wifi->clear();
        }
        break;
    }

    case MSG_ID_APP_EXIT: // this is a curious msg received when app exit but used for sleep now
        //qDebug("\nGet 9!!!!!!!!!!");
        restartSleepTimer();
        if(!sandDisp->isHidden()) {
            sandDisp->hide();

            if(slideScreen && slideScreen->isHidden())
                slideScreen->show();

            keyEnable = true;
//            setDisabled(false);
            sandDisp->clearAll();
        }

    	break;

    case MSG_ID_APP_RUN:
        ret = msg.msg_data.gprs_d;
        if (ret) {
            restartSleepTimer(false); //stop timer
            STOP_SLEEP = true;
        }
        else {
            STOP_SLEEP = false;
            restartSleepTimer(); //restart timer            
        }
        break;

    case MSG_ID_SETDATE:
        message->show();
        break;

    case MSG_ID_APP_TIMEOUT:
    {                
        struct lcd_message msgtimeout;

        //msgtimeout.mtype = 49; // send msg to linuxpos to remove app

        QMessageBox box;
        QPushButton *btnOk = new QPushButton(tr("Ok"));
        QPushButton *btnCancel = new QPushButton(tr("Cancel"));
        btnOk->setMinimumSize(70, 30);
        btnCancel->setMinimumSize(70, 30);

        box.addButton(btnOk, QMessageBox::YesRole);
        box.addButton(btnCancel, QMessageBox::NoRole);
        box.setIcon(QMessageBox::Information);
        box.setWindowFlags(Qt::CustomizeWindowHint);
        box.setText(tr("APP no response!\nRemove it?"));
        box.exec();

        if (box.clickedButton() == btnOk) {
            msgtimeout.msg_data.gprs_d = 1;
        } else {
            msgtimeout.msg_data.gprs_d = 0;
        }

        sendmsg2linuxpos(&msgtimeout);
//        if (msgsnd(mqId, &msgtimeout, 140, 0) < 0) {
//            perror("MSG_ID_APPTIMEOUT msgsnd error");
//        }
    }
        break;

    case MSG_ID_APP_PIN: //show pin input form
    {
        //show pin pad and get pin
        showPin(msg.msg_data.mtext, msg.msg_data.gprs_d);
    }
        break;

    case MSG_ID_AUTHORIZE:
    {
        qDebug("\nGet MSG_ID_AUTHORIZE!!");
        AuthorizationForm *af = new AuthorizationForm();
        af->showMaximized();
    }
        break;

    case MSG_ID_ATTACK: //under attack!
    {
        QLabel *label_attack = new QLabel(tr("Under attack!!"), this);
        setDisabled(true);
        label_attack->setGeometry(0, 0, 240, 320);
        while (1) { //dead!
            label_attack->setStyleSheet("color: red; font: bold 20px");
            sleep(1);
            label_attack->setStyleSheet("color: gray; font: bold 20px");
            sleep(1);
        }
    }
        break;

    default:
        //qDebug("%d,%d,%d,%s",msg.msg_data.api_id,
        //       msg.msg_data.line,msg.msg_data.colum,msg.msg_data.mtext);
        break;
    }
}


void LcdDaemon::startGif()
{
    setDisabled(true);
    loading = new QMovie("image/apploading.gif");
    giflabel = new QLabel(this);
    giflabel->setGeometry(20, (320-75)/2, 200, 75);
    giflabel->setMovie(loading);
    giflabel->show();
    loading->start();
}

void LcdDaemon::stopGif()
{
    loading->stop();
    giflabel->hide();
}

void LcdDaemon::setupWindow()
{
    stopGif();
    top_form->show();

    if (slideScreen == NULL) {

        /*if (!testNames.isEmpty()) {
            testNames.clear();
        }
        if (!testIcons.isEmpty()) {
            testIcons.clear();
        }*/
        slideScreen = new ParallaxHome(testNames, testIcons, this);
        slideScreen->setGeometry(0, top_form->height(), top_form->width(), 320-top_form->height());
        slideScreen->show();

        setDisabled(false);
        paral_show = true;
        /*
         *因为关屏到真正休眠，按键唤醒到真正唤醒 还有1到2秒的时间（如果wifi打开则时间更长），
         *所以为了避免这段时间的误操作把所有事件disable -- xl 20130419
         */
        connect(&(slideScreen->sleepThread), SIGNAL(disableAll(bool)), this, SLOT(slot_setDisabled(bool)));
    }
}

void LcdDaemon::setWidgetText(char *text, int widget_type, int widget_info)
{
    switch(widget_type)
    {
    case WIDGET_LABEL:
    {
        if(strncmp(text,"NO APP ! ...",12) == 0)
        {            
            setupWindow();
        }
        else if(strncmp(text,"APP LOADING",11) == 0)
        {
            startGif();
            break;
        }

        else { // show error message
            battery_state_box.setText(text);
            battery_state_box.exec();
        }

        if(paral_show)
        {
            if(slideScreen->isHidden())
            {
                if(strncmp(text,"WELCOME",7) == 0)
                {
                    slideScreen->show();
                }
            }
        }

        break;
    }
    /*case WIDGET_BUTTON:
    {
        sandbutton[line]->setText(tr(text));
        sandlabel[line]->hide();
        sandbutton[line]->key_value = widget_key;
        sandbutton[line]->show();
        break;
    }*/
    case WIDGET_ICONBUTTON:
    {
        int i, j;
        char path[20];
        char path_tmp[20];

        if(this->slideScreen != NULL)
            this->slideScreen->setDisabled(false);

        memset(path_tmp, 0, 20);
        memset(path, 0, 20);
        strcat(path, "/app/");
        j = widget_info;
        for(i = 4; i >= 0; i--)
        {
            path[5 + i] = j%10 + '0';
            j = (int)(j/10);
        }

        strcat(path_tmp, path);
        strcat(path_tmp, "/app.png");
        if(0 == access(path_tmp, F_OK))
            strcat(path, "/app.png");
        else
            strcat(path, "/app.png");
//        qDebug("app %d ico : %s  ;  path_tmp  :  %s", widget_info,path, path_tmp);
        path[19] = 0;

        for(i = 0; text[i] != 0; i++)//NULL
        {
            if(text[i] == 13)
                text[i] = 0;//NULL
        }

        testNames.append(tr(text));
        testIcons.append(tr(path));

        g_app_curret++;
        if(g_app_curret == g_app_cnt)
        {
            setupWindow();
        }
        break;
    }
    default:
        break;
    }
    return;
}

void LcdDaemon::restartSleepTimer(bool start)
{
    if (slideScreen) {
        if (start){
            if (!STOP_SLEEP) {
                slideScreen->setEnableSleep();
                slideScreen->restartTimer();
            }
        }
        else
            slideScreen->stopTimer();
    }
}

void LcdDaemon::showDisp()
{
    if(sandDisp->isHidden())
        sandDisp->show();
    if(slideScreen)
        slideScreen->hide();

    keyEnable = false;
}

void LcdDaemon::showPin(QString text, int timeout)
{
    if (timeout < 10 || timeout > 60)
        timeout = 30;
    pinForm = new PinForm(text);
    pinForm->setGeometry(0, 25, 240, 320-25);
    connect(pinForm, SIGNAL(getPin(QString)), this, SLOT(slot_get_pin(QString)));
    pinForm->show();

    timerPin.setSingleShot(true);
    connect(&timerPin, SIGNAL(timeout()), this, SLOT(slot_pin_timeout()));
    timerPin.start(timeout*1000);
}

void LcdDaemon::slot_get_pin(QString pin)
{
    lcd_message msg;

    memset(&msg, 0, sizeof(msg));
    msg.mtype = 51; //msg send to app

    if(!pin.isEmpty()) {
        if (pin == "-") {
            msg.msg_data.gprs_s = -1; //cancel
        } else {
            strncpy(msg.msg_data.mtext, pin.toLatin1().data(), pin.length());
            msg.msg_data.gprs_s = pin.length();
        }
    } else {
        msg.msg_data.gprs_s = 0;
    }

    timerPin.stop();
    if (pinForm) {
        pinForm->close();
        pinForm = NULL;
    }

    if (msgsnd(mqId, &msg, 140, 0) < 0) {
        perror("<sendMsg> msgsnd error");
    }
}

void LcdDaemon::slot_pin_timeout()
{
    lcd_message msg;

    memset(&msg, 0, sizeof(msg));
    msg.mtype = 51;
    msg.msg_data.gprs_s = -2;

    if (pinForm) {
        pinForm->close();
        //pinForm->deleteLater();
        pinForm = NULL;
    }

    if (msgsnd(mqId, &msg, 140, 0) < 0) {
        perror("<sendMsg> msgsnd error");
    }
}
