#include "sanddisplay.h"

SandDisplay::SandDisplay(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint)
{
    setGeometry(0, 25, 240, 320-25);
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
    repaint(x, y, WIDTH-x, HEIGHT-y);
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
        qDebug("\n   drawText: x=%d, y=%d, w=%d", px, py, pw);
        break;

    case 2:
        painter.fillRect(MARGIN_LEFT, py, WIDTH, GB2312PIX, backgroundColor);
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
