#include "parallaxhome.h"
#include <QDebug>
#include "pswdialog.h"
//#include "mainentrythread.h"
//#include "wpagui.h"
//#include "scanresults.h"
#include "modulesetting.h"
#include "datetimesetting.h"
#include "screensetting.h"
#include "gsmsetting.h"
#include "keyclearsetting.h"
#include "activewin.h"
#include "correctscreen.h"
#include "wifisetwin.h"
#include "lowpowerwin.h"
extern "C"{
#include "new_drv.h"
#include "osevent.h"
#include "sand_main.h"
#include "devSignal.h"
#include "sand_key.h"
#include "osdriver.h"
}

//#define thread 1
#ifdef thread

MainEntryThread *mainEntryThread;
#endif

ParallaxHome::ParallaxHome(int argc, char *argv[], QWidget *parent) :
    QGraphicsView(parent),
    m_pageOffset(-1),
    pageIndex(0)
{
    setFrameShape(QFrame::NoFrame);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    dragFlag = Flag_Slide;
    pItem = NULL;
    pressFlag = false;

    enterFlag = false;
//    testNames << tr("1.MODULE") << tr("2.DATETIME") << tr("3.SCREEN") << tr("4.GSM")<<tr("5.MODULE \nACTIVE")<<tr("6.KEY CLEAR")<<tr("7.WIFI")
//              <<tr("8.SCREEN \nADJUST")<<tr("9.WIFI \nSUSPEND")<<tr("1.EXIT");

//    testIcons << tr("/daemon/image/icon/1.png") << tr("/daemon/image/icon/2.png")
//              << tr("/daemon/image/icon/3.png") << tr("/daemon/image/icon/4.png") <<tr("/daemon/image/icon/active.png")
//              << tr("/daemon/image/icon/keyclear.png")<< tr("/daemon/image/icon/5.png")
//              <<tr("/daemon/image/icon/screenAdjust.png")<<tr("/daemon/image/icon/5.png")<<tr("/daemon/image/icon/6.png");


    testNames << tr("1.MODULE") << tr("2.DATETIME") << tr("3.SCREEN") << tr("4.GSM")<<tr("5.KEY CLEAR")<<tr("6.SCREEN \nADJUST")
              <<tr("7.MODULE \nACTIVE")<<tr("8.WIFI")
              <<tr("9.WIFI \nSUSPEND")<<tr("1.EXIT");

    testIcons << tr("/daemon/image/icon/1.png") << tr("/daemon/image/icon/2.png")
              << tr("/daemon/image/icon/3.png") << tr("/daemon/image/icon/4.png") <<tr("/daemon/image/icon/keyclear.png")
              << tr("/daemon/image/icon/screenAdjust.png")
              <<tr("/daemon/image/icon/active.png")
             << tr("/daemon/image/icon/5.png")<<tr("/daemon/image/icon/5.png")<<tr("/daemon/image/icon/6.png");

    fristFLag = true;

    pageCount = ceil(static_cast<double>(testNames.count())/9); //pageCountÏòÉÏÉáÈëfd
    //qDebug() << "pageCount = " << pageCount;

    setDragMode(RubberBandDrag);

    setAcceptDrops(true);
    setupScene(); //load item

    setScene(&m_scene);
    connect(&m_pageAnimator, SIGNAL(frameChanged(int)), SLOT(shiftPage(int)));
    m_pageAnimator.setDuration(500);
    m_pageAnimator.setFrameRange(0, 100);
    m_pageAnimator.setCurveShape(QTimeLine::EaseInCurve);

    choosePage(0);
#ifdef thread
    qt_main_entry_init(argc, argv);

    mainEntryThread = new MainEntryThread(argc,argv,this);
    connect(mainEntryThread,SIGNAL(sysEventSignal(int)),this,SLOT(sysEventSlot(int)));
    mainEntryThread->start();
#endif

}

void ParallaxHome::sysEventSlot(int sysEvent)
{
#ifdef thread
    ((struct seven *)sysEvent)->pt_eve_out->treatment_status = NOK;
    switch ((((struct seven *)sysEvent)->event_nr + ((struct seven *)sysEvent)->event_type))
    {
    case OSEVENTTYPE_SEL+0xff:
    case OSEVENTTYPE_SEL+0xfe:
        //qDebug("qtest show");
        ((struct seven *)sysEvent)->pt_eve_out->treatment_status = NOK;
        this->show();
        break;

//    case OSEVENTTYPE_CIR + OSEVENTID_MFCCRD:
//    {
//        //qDebug("SIGNAL : OSEVENTTYPE_CIR + OSEVENTID_MFCCRD signal get from APP");
//        ((struct seven *)sysEvent)->pt_eve_out->treatment_status = OK;
//        mainEntryThread->wakeUp();
//    }
//    break;

    default:
        mainEntryThread->wakeUp();
        break;
    }
#endif
}

void ParallaxHome::slideBy(qreal dx)
{
    m_pageOffset = static_cast<qreal>(pageIndex);
    int start = -(m_pageOffset + dx) * width();
    int end = m_pageOffset * width();
    if(fristFLag){
         fristFLag = false;
    }
    else
    {
        m_pageAnimator.setFrameRange(start, end);
        m_pageAnimator.start();
    }


    m_naviBar->setIndexPage(-pageIndex);
}

void ParallaxHome::choosePage(int page)
{
    if (m_pageAnimator.state() != QTimeLine::NotRunning)
        return;
    slideBy(-page - m_pageOffset);
}

void ParallaxHome::shiftPage(int frame) //¶¨Î»¸÷¸öitemµÄ×ø±ê
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
    for (int i = 0; i < pageCount; i++) //Ò³Êý
    {
        int yp = 55;//xl 50;

        for (int j = 0; j < 3; j++) //ÐÐÊý
        {
            int xp = 40;
            for (int k = 0; k < 3; k++) //ÁÐÊý
            {
                if (!m_items.isEmpty() && m_items.count() >= testNames.count())
                    break;

                Node *node = new Node;

                int index = i*9 + j*3 + k;
                node->setText(testNames.at(index));//ÉèÖÃitemµÄÃû³Æ
                QPixmap pix(testIcons.at(index));
                pix.scaled(QSize(59, 50), Qt::IgnoreAspectRatio);
                node->setItemPixmap(pix); //ÉèÖÃitemµÄicon

                m_scene.addItem(node);
                m_items << node;

                qreal qx = static_cast<qreal>(xp)/qreal(240) + static_cast<qreal>(i);
                qreal qy = static_cast<qreal>(yp)/qreal(320-70);//xl

                node->setItemPoint(QPointF(qx, qy));

                m_positions << QPointF(qx, qy);
                node->setZValue(1);
                xp = xp + 80; //ÁÐ¼ä¾à
            }
            yp = yp + 80-2; //ÐÐ¼ä¾à
        }
    }

    m_naviBar = new NaviBar(pageCount);
    m_scene.addItem(m_naviBar);
    m_naviBar->setIndexPage(-pageCount);
    m_naviBar->setZValue(2);

    QPixmap pixmap("/daemon/image/surfacing.png");
    //TODO
    //pixmap = pixmap.scaled(width()*pageCount, height() + m_naviBar->rect().height() + 50);
    m_scene.addPixmap(pixmap); //ÉèÖÃ³¡¾°±³¾°

    m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
}

void ParallaxHome::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    layoutScene();
}


void ParallaxHome::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
          event->ignore();

    pressFlag = true;//Êó±êµãÔÚ×ÀÃæÄÚ
    oldPoint = event->pos();//È¡µÃ°´ÏÂÊ±µÄ×ø±ê

    QList<QGraphicsItem *> pitems = QGraphicsView::items(event->pos());
    isHasItem = pitems.count();

    if (isHasItem == 2) //background and item, select one item.
    {
        pItem = dynamic_cast<Node*>(QGraphicsView::itemAt(event->pos()));
        if (pItem!= NULL)
        {
            //qDebug()<<"is ining ***********";
            pItem->setBackgroundColor(QColor(159, 159, 159));
            dragFlag = Flag_Start;//????
        }
    }
    else //only background, no item is selected.
    {
        dragFlag = Flag_Slide; //????
        //qDebug()<<"not  ining ***********";

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
            //qDebug()<<"aaaaaaa ***********";

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
           // //qDebug()<<"bbbbbbbbbb ***********";


            //»¬¶¯¾àÀë³¬¹ý´°¿Ú¿í¶ÈµÄÒ»°ë²Å»á»»Ò³
            if((dValue >= width()/6) && (pageIndex < 0))
                pageIndex++;
            else if((dValue <= -width()/6) && (pageIndex > -pageCount+1))
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
        //qDebug()<<"mouse move *******";
        int value = event->pos().x() - oldPoint.x();
       // //qDebug()<<"curpos   oldpox  value  width()/6"<<event->pos().x()<<oldPoint.x()<<value<<width()/6;
        if (qAbs(value) >= 8)
            dragFlag = Flag_Slide;
//        else
//            dragFlag = Flag_Start;

       // //qDebug()<<"move dragFlag=========="<<dragFlag;
        shiftPage(value + pageIndex * width());
        event->accept();
    }
}

void ParallaxHome::layoutScene()
{
    m_scene.setSceneRect(0, 0, pageCount * width() - 1, height() - 1);//ÉèÖÃ³¡¾°µÄ´óÐ¡
    centerOn(/*width() / 2, height() / 2*/0, 0); //¶¨Î»³¡¾°µÄ×ø±êÔ­µã

    int nw = m_naviBar->rect().width();
    int nh = m_naviBar->rect().height();
    m_naviBar->setPos((width() - nw) / 2, height() - nh);

    shiftPage(m_pageOffset * width());
}

void ParallaxHome::setEnterFlag()
{
  enterFlag = false;
//  qDebug() << "~~~~~enterFlag "<< enterFlag;

}

void ParallaxHome::testByItemName(const QString &itemName) //´ò¿ª²âÊÔ½çÃæ
{
//   qDebug() << "enterFlag "<< enterFlag;
    if((!enterFlag) && (itemName == testNames.at(0)))
    {
       moduleFunc();

    }
    else if((!enterFlag) && (itemName == testNames.at(1)))
    {
       datetimeFunc();


    }
    else if((!enterFlag) && (itemName == testNames.at(2)))
    {
       screenFunc();

    }
    else if((!enterFlag) && (itemName == testNames.at(3)))
    {
      gsmFunc();
    }

    else if((!enterFlag) && (itemName == testNames.at(4)))
    {
        keyclearFunc();

    }
    else if((!enterFlag) && (itemName == testNames.at(5)))
    {
       correctScreenFunc();

    }
    else if((!enterFlag) && (itemName == testNames.at(6)))
    {
       activemoduleFunc();

    }
    else if((!enterFlag) && (itemName == testNames.at(7)))
    {
        wifisetFunc();
    }
    else if((!enterFlag) && (itemName == testNames.at(8)))
    {
        wifiModuleFunc();

    }
    else if((!enterFlag) && (itemName == testNames.at(9)))
    {
      exitFunc();

    }



}

ParallaxHome::~ParallaxHome()
{
   // //qDebug()<<"IN ~ParallaxHome()";
    if(pItem!=NULL)
    {
        delete pItem;
        pItem = NULL;
    }

    if( m_naviBar!=NULL)
    {
        delete m_naviBar;
        m_naviBar = NULL;
    }



    qDeleteAll(m_items.begin(),m_items.end());
    m_items.clear();

//    for(QList<Node*>::iterator
//        i = m_items.begin();i<m_items.end();i++)
//    {
//        if(*i!=NULL)
//        {
//            delete *i;
//            *i = NULL;
//        }
//    }
   // //qDebug()<<"22222IN ~ParallaxHome()";

//    while(!m_items.isEmpty())
//        delete m_items.takeFirst();

   // m_items.clear();
   // //qDebug()<<"333333333IN ~ParallaxHome()";


}


void ParallaxHome::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_1:{
        if((pageIndex==0)&&(!enterFlag)){
            moduleFunc();

        }
        else if((pageIndex==-1)&&(!enterFlag) ){

            exitFunc();
        }
    }
        break;
    case Qt::Key_2:
    {
        if(enterFlag) return;
        datetimeFunc();

    }
        break;
    case Qt::Key_3:
    {
        if(enterFlag) return;
        screenFunc();

    }
        break;
    case Qt::Key_4:
    {
        if(enterFlag) return;
        gsmFunc();

    }
        break;
    case Qt::Key_5:
    {
        if(enterFlag) return;
        keyclearFunc();
    }
        break;

    case Qt::Key_6:
    {
        if(enterFlag) return;
        correctScreenFunc();

    }
        break;
    case Qt::Key_7:
    {
        if(enterFlag) return;
        activemoduleFunc();

    }
        break;


    case Qt::Key_8:
    {
        if(enterFlag) return;
        wifisetFunc();
    }
        break;

    case Qt::Key_9:{
        if(enterFlag) return;
        wifiModuleFunc();
    }

    case Qt::Key_Escape:
    case Qt::Key_Backspace:
    {
        if(enterFlag) return;
        exitFunc();
    }

        break;


    }

}

void ParallaxHome::moduleFunc()
{
    enterFlag = true;
    ModuleSetting *ms = new ModuleSetting();
    ms->show();
    connect(ms,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
}

void ParallaxHome::datetimeFunc()
{
    enterFlag = true;
    DateTimeSetting *ds = new DateTimeSetting();
    ds->show();
    connect(ds,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
}

void ParallaxHome::screenFunc()
{
    enterFlag = true;
    ScreenSetting *ss = new ScreenSetting();
    ss->show();
    connect(ss,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
}

void ParallaxHome::gsmFunc()
{
    enterFlag = true;
    GsmSetting *gs = new GsmSetting();
    gs->show();
    connect(gs,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
}

void ParallaxHome::activemoduleFunc()
{
    enterFlag = true;
    ActiveWin *aw = new ActiveWin();
    aw->show();
    connect(aw,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
}

void ParallaxHome::keyclearFunc()
{
    enterFlag = true;

    PswDialog *pswDialog = new PswDialog();
    int rec =  pswDialog->exec();
    if(!rec){//reject is 0 ,accept is 1!
        enterFlag = false;
        return;
    }

    KeyClearSetting *ks = new KeyClearSetting();
    ks->show();
    connect(ks,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
}

void ParallaxHome::wifisetFunc()
{
    enterFlag = true;

    if(Os__get_device_signal(DEV_WIFI)==0)
    {
        QMessageBox box(QMessageBox::Information,tr("  title "),tr("WIFI NOT OPEN,PLEASE CHECK!"));
        box.setWindowFlags(Qt::CustomizeWindowHint);

        QPushButton *okBtn = new QPushButton(tr("Ok"));
        okBtn->setFixedWidth(80);
        okBtn->setFixedHeight(25);
        box.addButton(okBtn,QMessageBox::YesRole);

        box.exec();

        enterFlag = false;
    }else{

        WifiSetWin *wpa = new WifiSetWin();
//        wpa->setGeometry(0, 20, 240, 320-20);
        wpa->show();
        connect(wpa,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
    }
}

void ParallaxHome::correctScreenFunc()
{
    enterFlag = true;
    PswDialog *pswDialog = new PswDialog();
    int rec =  pswDialog->exec();
    if(!rec){//reject is 0 ,accept is 1!
        enterFlag = false;
        return;
    }

    CorrectScreen *cs = new CorrectScreen;
    cs->show();

    connect(cs,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
}

void ParallaxHome::wifiModuleFunc()
{
    enterFlag = true;
    if(Os__get_device_signal(DEV_WIFI)==0)
    {
        QMessageBox box(QMessageBox::Information,tr("  title "),tr("WIFI NOT OPEN,PLEASE CHECK!"));
        box.setWindowFlags(Qt::CustomizeWindowHint);

        QPushButton *okBtn = new QPushButton(tr("Ok"));
        okBtn->setFixedWidth(80);
        okBtn->setFixedHeight(25);
        box.addButton(okBtn,QMessageBox::YesRole);

        box.exec();

        enterFlag = false;
    }else{
        LowPowerWin *lpw = new LowPowerWin();
        lpw->show();
        connect(lpw,SIGNAL(destroyed()),this,SLOT(setEnterFlag()));
    }
}

void ParallaxHome::exitFunc()
{
    enterFlag = true;
#ifdef thread
    mainEntryThread->wakeUp();
    hide();
#else
    close();
    Os__set_key_ensure(KEY_SYS_EXIT);
    //        Os__set_key_ensure(0xfe);
#endif
    enterFlag = false;
}
