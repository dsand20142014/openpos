#include "parallaxhome.h"
#include <QDebug>
#include "sand_main.h"

extern "C"
{
#include "sand_key.h"
#include "sysparam.h"
}

#include "fsync_drvs.h"
#include "osevent.h"
#define thread 0
#ifdef thread

#endif

ParallaxHome::ParallaxHome(QStringList tNames, QStringList tIcons, QWidget *parent) :
    QGraphicsView(parent),
    m_pageOffset(-1),
    pageIndex(0)
{
    setFrameShape(QFrame::NoFrame);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   

    dragFlag = Flag_Slide;
    pItem = NULL;
    pressFlag = false;

    testNames = QStringList(tNames);    
    testIcons = QStringList(tIcons);
    oldpage = 0;

    pageCount = ceil(static_cast<double>(testNames.count())/9); //pageCountÏòÉÏÉáÈëfd
	STOP_SLEEP = false;

    setDragMode(RubberBandDrag);
    setAcceptDrops(true);
    setupScene(); //load item
    setScene(&m_scene);

    connect(&m_pageAnimator, SIGNAL(frameChanged(int)), SLOT(shiftPage(int)));
    m_pageAnimator.setDuration(200);
    m_pageAnimator.setFrameRange(0, 100);
    m_pageAnimator.setCurveShape(QTimeLine::EaseInCurve);

    choosePage(0);
    setSleep();
}

/*
 * Get sleep time and set sleep when timeout.
 * Add from linuxpos by xiangliu 20130318.
 */
void ParallaxHome::setSleep()
{
    SYS_SETTING setting;

    sleepTime = 10000;

    memset(&setting, 0, sizeof(SYS_SETTING));
    strcpy(setting.section, "SCREEN_SETTING");
    strcpy(setting.key, "SLEEP_TIME");

    if(PARAM_setting_get(&setting) >= 0) {
        QString v = setting.value;
        if (!v.isNull() && !v.isEmpty()) {
            sleepTime = v.toInt() * 1000;
        }
    }

    if (sleepTime < 10000) {
        sleepTime = 10000; //10s
    }

    connect(&sleepTimer, SIGNAL(timeout()), this, SLOT(slot_sleep()));    
    sleepTimer.start(sleepTime);
}

void ParallaxHome::slot_sleep()
{
    if (this->parentWidget() && this->parentWidget()->isActiveWindow() && this->isVisible()) {
        sleepThread.start();
    }
}

void ParallaxHome::restartTimer()
{
	if (STOP_SLEEP)
		return;
    if (sleepTimer.isActive()) {
        sleepTimer.stop();
    }
    sleepTimer.start(sleepTime);
}

void ParallaxHome::stopTimer()
{
    if (sleepTimer.isActive()) {
        sleepTimer.stop();
    }
	STOP_SLEEP = true;
}

void ParallaxHome::setEnableSleep()
{
	STOP_SLEEP = false;
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
//    printf("          choosePage   :   %d\n", page);
    if (m_pageAnimator.state() != QTimeLine::NotRunning)
    {
        usleep(200000);
        m_pageAnimator.start();
        if(oldpage != page)
        {
            if (m_pageAnimator.state() != QTimeLine::NotRunning)
            {
 //               printf("---------errorerrorerror m_pageAnimator.state  :  %d!!!\n", page);
                return;
            }
            oldpage = page;
            slideBy(-page - m_pageOffset);
        }
        else
        {
//            printf("     !!!!error       choosePage   :   %d!!!!!!!!!!!!!!\n", page);
            return;
        }
    }
    oldpage = page;
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
        int yp = 38; //xl

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
#if 1
    QPixmap pixmap("/daemon/image/surfacing.png");
    //TODO
//    pixmap = pixmap.scaled(240,320);

    m_scene.addPixmap(pixmap); //ÉèÖÃ³¡¾°±³¾°
    pixmap = pixmap.scaled(width()*(pageCount == 0 ? 1 : pageCount), height() + m_naviBar->rect().height() + 50);
#endif
    m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
}

void ParallaxHome::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    layoutScene();
}

void ParallaxHome::mousePressEvent(QMouseEvent *event)
{
    restartTimer();
    if (event->button() != Qt::LeftButton)
          event->ignore();

    pressFlag = true;//Êó±êµãÔÚ×ÀÃæÄÚ
    oldPoint = event->pos();//È¡µÃ°´ÏÂÊ±µÄ×ø±ê

    if (oldPoint.y() >= 280){// && oldPoint.x() >= 94 && oldPoint.x() <= 140) {
        dragFlag = Flag_Slide;
        event->accept();
        return;
    }

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
    restartTimer();
    if (event->button() != Qt::LeftButton)
          event->ignore();

    curPoint = event->pos();

    if (curPoint.y() >= 280 && oldPoint.x() >= 100 && curPoint.x() >= 100 && curPoint.x() <= 138) {
        choosePage(pageIndex);
        event->accept();
        return;
    }

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
            {
                //Os__set_key_ensure(200); //xl for sleep
                return;
            }

            pressFlag = false;
            setDisabled(true);
            pItem->setBackgroundColor(Qt::transparent);
            QString name = pItem->text();
            testByItemName(name);
            pItem = NULL;
            setDisabled(false);
        }
        else if (dragFlag == Flag_Slide) //????
        {
            if (pItem != NULL)
            {
                pItem->setBackgroundColor(Qt::transparent);
                pItem = NULL;
            }

            //»¬¶¯¾àÀë³¬¹ý´°¿Ú¿í¶ÈµÄÒ»°ë²Å»á»»Ò³
            if((dValue >= width()/6) && (pageIndex < 0))
            {
                if (m_pageAnimator.state() == QTimeLine::NotRunning)
                if(Os__set_key_ensure(KEY_UP) == 0)//不支持按键翻页
                {
//                    Os__set_key_ensure(KEY_UP);
//                    Os__set_key_ensure(KEY_DOWN);
                    pageIndex++;
                }
            }
            else if((dValue <= -width()/6) && (pageIndex > -pageCount+1))
            {
                if (m_pageAnimator.state() == QTimeLine::NotRunning)
                if(Os__set_key_ensure(KEY_DOWN) == 0)//不支持按键翻页
                {

                    pageIndex--;
                }
            }

            choosePage(pageIndex);
        }
        event->accept();
    }
 //   choosePage(pageIndex);
}

void ParallaxHome::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
          event->ignore();

    if (pressFlag)
    {
        int value = event->pos().x() - oldPoint.x();

        if (qAbs(value) >= 8) {
            dragFlag = Flag_Slide;
            restartTimer();//Os__set_key_ensure(200); //xl for sleep
        }
//        else
//            dragFlag = Flag_Start;

        shiftPage(value + pageIndex * width());
        event->accept();
    }
}

void ParallaxHome::layoutScene()
{
    m_scene.setSceneRect(0, 0, (pageCount == 0 ? 1 : pageCount) * width() - 1, height() - 1);

//    m_scene.setSceneRect(0, 0, pageCount * width() - 1, height() - 1);//ÉèÖÃ³¡¾°µÄ´óÐ¡
    centerOn(/*width() / 2, height() / 2*/0, 0); //¶¨Î»³¡¾°µÄ×ø±êÔ­µã

    int nw = m_naviBar->rect().width();
    int nh = m_naviBar->rect().height();
    m_naviBar->setPos((width() - nw) / 2, height() - nh);

    shiftPage(m_pageOffset * width());
}

void ParallaxHome::testByItemName(const QString &itemName) //¡ä¨°?a2a¨º?????
{
    int i;

    choosePage(pageIndex);
    for(i = 0; i < m_items.count(); i++)
    {
        if(itemName == testNames.at(i))
        {
            if(Os__set_key_ensure(i%9 + '1') == 0)
            {
//                setDisabled(true);
//                printf("setDisabled123  [%d]  [%d]  [%d]  [%d]\n", __LINE__, pageIndex, m_items.count(),i);
            }

            break;
        }
    }
}

ParallaxHome::~ParallaxHome()
{
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
}

