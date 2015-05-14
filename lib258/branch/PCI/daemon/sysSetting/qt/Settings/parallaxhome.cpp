#include "parallaxhome.h"
#include "mainentrythread.h"
#include <QDebug>

#include "gps.h"
#include "module.h"
#include "gsm.h"
#include "wifi.h"
#include "screen.h"
#include "date.h"
#include "ethernet.h"
#include "moden.h"
#include "gsmscroll.h"
#include "wpagui.h"

#include "osevent.h"
#include "sand_main.h"


#include "oslib.h"
#include "sand_time.h"


MainEntryThread *mainEntryThread;

ParallaxHome::ParallaxHome(int argc, char *argv[], QWidget *parent) :
    QGraphicsView(parent),
    m_pageOffset(-1),
    pageIndex(0)
{
    setFrameShape(QFrame::NoFrame);
    setWindowFlags(Qt::FramelessWindowHint);
   // setWindowTitle("Parallax Home");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainEntryThread = new MainEntryThread(argc,argv,this);
    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->start();

    dragFlag = Flag_Slide;
    pItem = NULL;
    pressFlag = false;

    testNames << tr("MODULE") << tr("DATETIME") << tr("SCREEN") << tr("GSM")
              << tr("WIFI") << tr("ETHERNET") << tr("GPS") << tr("MODEN")<<tr("CANCEL");

    testIcons << tr("./image/Currency.png") << tr("./image/Engadget.png")
              << tr("./image/IMDb.png") << tr("./image/MobileRSS.png")
              << tr("./image/PS Mobile.png") << tr("./image/Games.png")
              << tr("./image/Attic.png") << tr("./image/MobileRSS.png")
              << tr("./image/Videos.png");

    pageCount = ceil(static_cast<double>(testNames.count())/9); //pageCount向上舍入

    setDragMode(RubberBandDrag);
    setAcceptDrops(true);
    setupScene(); //load item
    setScene(&m_scene);

    connect(&m_pageAnimator, SIGNAL(frameChanged(int)), SLOT(shiftPage(int)));
    m_pageAnimator.setDuration(500);
    m_pageAnimator.setFrameRange(0, 100);
    m_pageAnimator.setCurveShape(QTimeLine::EaseInCurve);

    choosePage(0);
}


void ParallaxHome::slideBy(qreal dx)
{
    m_pageOffset = static_cast<qreal>(pageIndex);
    int start = -(m_pageOffset + dx) * width();
    int end = m_pageOffset * width();
    m_pageAnimator.setFrameRange(start, end);
    m_pageAnimator.start();

    m_naviBar->setIndexPage(-pageIndex);
}

void ParallaxHome::choosePage(int page)
{
    if (m_pageAnimator.state() != QTimeLine::NotRunning)
        return;
    slideBy(-page - m_pageOffset);
}

void ParallaxHome::shiftPage(int frame) //定位各个item的坐标
{
    int ww = width();
    int hh = height() - m_naviBar->rect().height();
    m_pageOffset = static_cast<qreal>(frame) / qreal(width()) ;
    int ofs = m_pageOffset * ww;
    for (int i = 0; i < m_items.count(); ++i)
    {
        QPointF pos = m_positions[i];
        QPointF xy(pos.x() * ww, pos.y() * hh);
        m_items[i]->setPos(xy + QPointF(ofs, 0));
    }
}

void ParallaxHome::setupScene()
{
    for (int i = 0; i < pageCount; i++) //页数
    {
        int yp = 50;

        for (int j = 0; j < 3; j++) //行数
        {
            int xp = 40;
            for (int k = 0; k < 3; k++) //列数
            {
                if (!m_items.isEmpty() && m_items.count() >= testNames.count())
                    break;

                Node *node = new Node;

                int index = i*9 + j*3 + k;
                node->setText(testNames.at(index));//设置item的名称
                QPixmap pix(testIcons.at(index));
                pix.scaled(QSize(59, 50), Qt::IgnoreAspectRatio);
                node->setItemPixmap(pix); //设置item的icon

                m_scene.addItem(node);
                m_items << node;

                qreal qx = static_cast<qreal>(xp)/qreal(240) + static_cast<qreal>(i);
                qreal qy = static_cast<qreal>(yp)/qreal(320-50);

                node->setItemPoint(QPointF(qx, qy));

                m_positions << QPointF(qx, qy);
                node->setZValue(1);
                xp = xp + 80; //列间距
            }
            yp = yp + 80-2; //行间距
        }
    }

    m_naviBar = new NaviBar;
    m_scene.addItem(m_naviBar);
    m_naviBar->setIndexPage(-pageCount);
    m_naviBar->setZValue(2);

    QPixmap pixmap("./image/surfacing.png");
    //TODO
    pixmap = pixmap.scaled(width()*pageCount, height() + m_naviBar->rect().height() + 50);
    m_scene.addPixmap(pixmap); //设置场景背景

    m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
}

void ParallaxHome::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    layoutScene();
}

void ParallaxHome::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

void ParallaxHome::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
          event->ignore();

    pressFlag = true;//鼠标点在桌面内
    oldPoint = event->pos();//取得按下时的坐标

    QList<QGraphicsItem *> pitems = QGraphicsView::items(event->pos());
    isHasItem = pitems.count();

    if (isHasItem == 2) //background and item, select one item.
    {
        pItem = (Node*)QGraphicsView::itemAt(event->pos());
        if (!pItem->isNull())
        {
            pItem->setBackgroundColor(QColor(159, 159, 159));
            dragFlag = Flag_Start;//????
        }
    }
    else //only background, no item is selected.
    {
        dragFlag = Flag_Slide; //????
    }

    event->accept();
}

void ParallaxHome::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
          event->ignore();

    curPoint = event->pos();
    int dValue = curPoint.x() - oldPoint.x();
    if (qAbs(dValue) >= 8)
        dragFlag = Flag_Slide;
    else
        dragFlag = Flag_Start;

    if (pressFlag)
    {
        if (dragFlag == Flag_Start)  //????
        {
            if (pItem == NULL)
                return;

            pressFlag = false;
            pItem->setBackgroundColor(Qt::transparent);
            QString name = pItem->text();
            testByItemName(name);
            pItem = NULL;
        }
        else if (dragFlag == Flag_Slide) //????
        {
            if (pItem != NULL)
            {
                pItem->setBackgroundColor(Qt::transparent);
                pItem = NULL;
            }

            //滑动距离超过窗口宽度的一半才会换页
            //modify the range of pageIndex to limit the page count of display
            if((dValue >= width()/6) && (pageIndex < 0))
                pageIndex++;
            else if((dValue <= -width()/6) && (pageIndex > 0))
                pageIndex--;

            choosePage(pageIndex);
        }
        event->accept();
    }
}

void ParallaxHome::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
          event->ignore();

    if (pressFlag)
    {
        int value = event->pos().x() - oldPoint.x();
        if (qAbs(value) >= 8)
            dragFlag = Flag_Slide;
//        else
//            dragFlag = Flag_Start;

        shiftPage(value + pageIndex * width());
        event->accept();
    }
}

void ParallaxHome::layoutScene()
{
    m_scene.setSceneRect(0, 0, pageCount * width() - 1, height() - 1);//设置场景的大小
    centerOn(/*width() / 2, height() / 2*/0, 0); //定位场景的坐标原点

    int nw = m_naviBar->rect().width();
    int nh = m_naviBar->rect().height();
    m_naviBar->setPos((width() - nw) / 2, height() - nh);

    shiftPage(m_pageOffset * width());
}

void ParallaxHome::testByItemName(const QString &itemName) //打开测试界面
{
    if (itemName == testNames.at(0))
    {
        qDebug() << itemName;

        Module *module = new Module(this);
        module->showFullScreen();
        //module->show();
    }

    if (itemName == testNames.at(1))
    {
        qDebug() << itemName;

        Date *date = new Date(this);
        date->showFullScreen();
        //date->show();
    }
    else if (itemName == testNames.at(2))
    {
        qDebug() << itemName;

        Screen *screen = new Screen(this);
        screen->showFullScreen();
        //screen->show();
    }
    else if (itemName == testNames.at(3))
    {
        qDebug() << itemName;

//        Gsm *gsm = new Gsm(this);
//        gsm->showFullScreen();
        //gsm->show();
        GsmScroll *gsmScroll = new GsmScroll(this);
        gsmScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gsmScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gsmScroll->showFullScreen();
        //gsmScroll->show();
    }
    else if (itemName == testNames.at(4))
    {
        qDebug() << itemName;

        //Wifi *wifi = new Wifi(this);
        //wifi->showFullScreen();
        WpaGui *wpa = new WpaGui(qApp);
        wpa->showFullScreen();
        //wpa->show();
    }
    else if (itemName == testNames.at(5))
    {
        qDebug() << itemName;

        Ethernet *ethernet = new Ethernet(this);
        ethernet->showFullScreen();
        //ethernet->show();
    }
    else if (itemName == testNames.at(6))
    {
        qDebug() << itemName;

        Gps *gps = new Gps(this);
        gps->showFullScreen();
        //gps->show();
    }
    else if (itemName == testNames.at(7))
    {
        qDebug() << itemName;

        Moden *moden = new Moden(this);
        moden->showFullScreen();
        //moden->show();
    }
    else if(itemName == testNames.at(8))
    {
        qDebug() << itemName;

        //close();
        mainEntryThread->wakeUp();
        WriteDateTime();
        hide();
    }
}

void ParallaxHome::sysEventSlot(int sysEvent)
{

    //     * ????void manager(struct seven *EventInfo)?????
    //     * ?????????????????????

    ((struct seven *)sysEvent)->pt_eve_out->treatment_status = NOK;

    switch ((((struct seven *)sysEvent)->event_nr + ((struct seven *)sysEvent)->event_type))
    {
    case OSEVENTTYPE_SEL+0xff:
    case OSEVENTTYPE_SEL+0xfe:
        ((struct seven *)sysEvent)->pt_eve_out->treatment_status = OK;
        show();
        //mainEntryThread->wakeUp();
        break;

    default:
        mainEntryThread->wakeUp();
        break;
    }
}


int ParallaxHome::WriteDateTime(void)
{
    unsigned char dateTime[17];
    QSettings *settings = new QSettings("setting.ini",QSettings::IniFormat);
    settings->setIniCodec("UTF-8");

    readSetting.clear();
    readSetting.readKeys(settings,"DATE_TIME_SETTING");
    QString strDate,strTime;
    unsigned char *dateTimeTmp;
    for(int i=0; i<readSetting.keys.size(); i++)
    {
        if(readSetting.keys[i] == "01.YYMMDD")
        {
            strDate = readSetting.keysValue[i];
            dateTimeTmp = (unsigned char *)(strDate.toAscii().constData());

            dateTime[0] = dateTimeTmp[4];
            dateTime[1] = dateTimeTmp[5];
            dateTime[2] = dateTimeTmp[2];
            dateTime[3] = dateTimeTmp[3];
            dateTime[4] = dateTimeTmp[0];
            dateTime[5] = dateTimeTmp[1];

            Os__write_date(dateTime);

            memset(dateTime,0,sizeof(dateTime));
        }
        if(readSetting.keys[i] == "02.MMDD")
        {
            strTime = readSetting.keysValue[i];
            dateTimeTmp = (unsigned char *)(strTime.toAscii().constData());

            dateTime[0] = dateTimeTmp[0];
            dateTime[1] = dateTimeTmp[1];
            dateTime[2] = dateTimeTmp[2];
            dateTime[3] = dateTimeTmp[3];

            Os__write_time(dateTime);
        }
    }

    settings->endGroup();
}

unsigned char writeGprs(char* pucPtr)
{

    FILE *fp;
    int i = 0 , j;
    int tem = 0;
    char apn_buf[64];
    char rbuff[1024];
    char wbuff[1024];
    char APN[30];
    char *chat_script_file = "/etc/ppp/chat/gprs-chat-script";

    strcpy(APN, pucPtr);

    fp = fopen(chat_script_file, "r+");

    if(NULL == fp) {
        printf("[%s:%d] file open failed",__func__,__LINE__);
        return 0x01;
    }

    memset(rbuff, 0x00, sizeof(rbuff));
    memset(wbuff, 0x00, sizeof(wbuff));
    memset(apn_buf, 0x00, sizeof(apn_buf));
    fread(rbuff, 1024, 1, fp);
    while( i < 1024){
        if(rbuff[i]=='I' &&rbuff[i+1]=='P'){
            tem = i;
            for(j = 0; j < 64; j++, tem++){
                if(rbuff[tem+5] == 0x22) break;
                //printf("%d\n", rbuff[tem+5]);
                apn_buf[j] = rbuff[tem+5];
            }
            break;
        }
         i++;
    }

    if (strcmp(apn_buf,  pucPtr) == 0x00){
        fclose(fp);
        return 0;
    }
    //printf("apn_buf=%s\n", apn_buf);
    //printf("pucPtr =%s\n", pucPtr);
    i += 5;
    tem = i;
    memcpy(wbuff, rbuff, i);
    memcpy(wbuff+i, APN, strlen(APN));

    while(i < 1024){
        if(rbuff[i]=='"') break;
         i ++;
    }

    memcpy(wbuff+tem+strlen(APN), rbuff+i,strlen(rbuff)-i);

    ftruncate(fileno(fp), 0);
    fseek(fp, 0, SEEK_SET);
    fwrite(wbuff,strlen(wbuff), 1, fp);
    fclose(fp);

    //OSGSM_GprsHangupDial_PR();

    return 0;

}

}

