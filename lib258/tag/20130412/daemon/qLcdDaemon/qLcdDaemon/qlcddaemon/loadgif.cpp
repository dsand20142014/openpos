#include "loadgif.h"

#include <QMovie>

LoadGif::LoadGif(QWidget *) :
    QWidget(0, Qt::Tool| Qt::WindowStaysOnTopHint| Qt::FramelessWindowHint)
{
    setStyleSheet("background-color: rgba(255, 255, 255, 0)");
    label_gif = new QLabel(this);
    label_gif->setStyleSheet("background-color: rgba(255, 255, 255, 0)");
    loading = new QMovie();
}

void LoadGif::gif_show(gif_loading_struct *content)
{

    if(content->stop == 1)
    {
        loading->stop();
        this->hide();
    }
    else
    {
		loading->setFileName(content->name);
//        loading->setScaledSize(QSize(content->w,content->h));
        label_gif->setGeometry(content->x,content->y,content->w,content->h);
        label_gif->setMovie(loading);

        loading->start();
        //setGeometry(content->x,content->y,content->w,content->h);
        this->showMaximized();
    }
}

LoadGif::~LoadGif()
{
}
