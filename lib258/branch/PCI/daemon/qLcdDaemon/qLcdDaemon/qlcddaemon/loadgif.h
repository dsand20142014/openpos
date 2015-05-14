#ifndef LOADGIF_H
#define LOADGIF_H

#include <QtGui>
#include <QWidget>
#include <QLabel>
#include "socketserver.h"


class LoadGif : public QWidget
{
    Q_OBJECT
    
public:
    explicit LoadGif(QWidget *parent = 0);
    ~LoadGif();
    void gif_show(struct gif_loading_struct *content);
    
private:
    QLabel *label_gif;
    QLabel *label_text;
    QMovie *loading;

    QWidget *centerWidget;
    QBoxLayout *centerLayout;
    QGridLayout *gridLayout;
    QSpacerItem *spacer1, *spacer2, *spacer3, *spacer4;

    int screenWidth;
    int screenHeight;
};

#endif // LOADGIF_H
