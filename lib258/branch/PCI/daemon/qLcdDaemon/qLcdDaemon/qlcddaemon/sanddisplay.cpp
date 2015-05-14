#include "sanddisplay.h"

SandDisplay::SandDisplay(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint)
{    
    pEvent = -1;
    //strFont.setBold(true);
    strFont.setPixelSize(16);
    fm = new QFontMetrics(strFont);
    ph = fm->height();
    qimg = NULL;
}

/*
 * Display image in (x, y).
 * Add by XiangLiu 2013.4.18 for old App.
 */
void SandDisplay::display_image(int x,int y,char *fname)
{
    pEvent = 0;
    fileName = fname;
    px = x;
    py = y;
    repaint(x, y, 240-x, 320-y);
}

/*
 * Display text in line, column.
 * Add by XiangLiu 2013.4.18 for old App.
 */
void SandDisplay::display_text(int line, int col, QString text)
{
    pEvent = 1;
    px = GB2312PIX * col/2 + MARGIN_LEFT;
    py = (GB2312PIX + SPACE) * line + MARGIN_TOP;
    strText = text;
    pw = fm->width(strText);
    //update(px, py, pw, ph);
    repaint(px, py, pw, ph);
}

/*
 * Display GB2312 text in line, column.
 * Add by XiangLiu 2013.4.18 for old App.
 */
void SandDisplay::display_GB2312(int line, int col, char *text)
{
    pEvent = 1;
    px = GB2312PIX * col + MARGIN_LEFT;
    py = (GB2312PIX + SPACE) * line + MARGIN_TOP;
    strText = text;
    pw = fm->width(strText);

    repaint(px, py, pw, ph);
}

/*
 * Display uchar image in line, column.
 * Add by XiangLiu 2013.4.18 for old App.
 */
void SandDisplay::display_graphic(int line, int col, char *ucimage, int size)
{
    pEvent = 4;
    px = GB2312PIX * col/2 + MARGIN_LEFT;
    py = (GB2312PIX + SPACE) * line + MARGIN_TOP;
    qimg = new QImage((const uchar *)ucimage, 16, 16, QImage::Format_Mono); //FIXME
    update();
}

void SandDisplay::slot_display(int c)
{
    switch (c) {
    case 0:
        display_text(0, 0, "1. APP TEST");
        break;
    case 1:
        display_GB2312(0, 1, "1. 测试1");
        break;
    case 2:
        display_text(1, 0, "1. APP TEST");
        break;
    case 3:
        display_text(1, 1, "1. APP TEST");
        break;
    case 4:
        display_clear(0);
        break;
    default:
        break;
    }
}

void SandDisplay::display_clear(int line)
{
    pEvent = 2;
    px = MARGIN_LEFT;
    py = (GB2312PIX + SPACE) * line;
    repaint(px, py, WIDTH, GB2312PIX + SPACE);
}

void SandDisplay::clearAll()
{
    pEvent = 3;
    repaint();
}

void SandDisplay::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    switch(pEvent) {
    case 0:
        painter.drawPixmap(px, py, QPixmap(fileName));
        break;

    case 1:
        painter.setFont(strFont);
        painter.drawText(px, py, pw, ph, Qt::AlignVCenter, strText);
        //qDebug("\n   drawText: x=%d, y=%d, w=%d", px, py, pw);
        break;

    case 2:
        painter.eraseRect(MARGIN_LEFT, py, WIDTH, GB2312PIX);
        //painter.fillRect(MARGIN_LEFT, py, WIDTH, GB2312PIX, backgroundColor);
        break;

    case 3:
        painter.fillRect(0, 0, 240, 320-25, backgroundColor);
        break;

    case 4:
        if (qimg) {
            painter.drawImage(px, py, *qimg);
        }
        break;

    default:
        break;
    }
}

SandDisplay::~SandDisplay()
{

}
