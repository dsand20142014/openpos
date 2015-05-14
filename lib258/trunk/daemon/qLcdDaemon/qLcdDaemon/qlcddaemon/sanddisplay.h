#ifndef SANDDISPLAY_H
#define SANDDISPLAY_H

#include <QtGui>

#define MAX_LINE    8
#define MAX_COLUMN  8
#define GB2312PIX   20 //pixel width per hanzi
#define MARGIN_LEFT 20 //margins of left/right
#define MARGIN_TOP  40
#define SPACE       10
#define WIDTH       200 // width per line
#define HEIGHT      280 // total height
#define backgroundColor Qt::gray

class SandDisplay : public QWidget
{
    Q_OBJECT
    
public:
    explicit SandDisplay(QWidget *parent = 0);
    ~SandDisplay();
    
    void display_image(int x, int y, char *fname);
    void display_text(int line, int col, QString text);
    void display_GB2312(int line, int col, char * text);
    void display_clear(int line);
    void display_graphic(int line, int col, char *ucimage, int size);
    void clearAll();

protected:
    void paintEvent(QPaintEvent *);

private:
    int pEvent;
    int px;
    int py;
    int pw;
    int ph;
    QString strText;
    QString fileName;
    QFont strFont;
    QFontMetrics *fm;
    QImage *qimg;
};

#endif // SANDDISPLAY_H
