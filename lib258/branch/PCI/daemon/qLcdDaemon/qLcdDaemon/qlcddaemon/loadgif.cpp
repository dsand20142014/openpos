#include "loadgif.h"
#include <QtGui>

LoadGif::LoadGif(QWidget *) :
    QWidget(0,  Qt::Tool|Qt::WindowStaysOnTopHint| Qt::FramelessWindowHint)
{
    setStyleSheet("background-color: rgba(255, 255, 255, 0)");
    label_gif = new QLabel();
    label_gif->setStyleSheet("background-color: rgba(255, 255, 255, 0)");
    loading = new QMovie();

    label_text = new QLabel();
    label_text->setWordWrap(true);

    screenWidth = qApp->desktop()->screen(0)->width();
    screenHeight = qApp->desktop()->screen(0)->height();

    centerWidget = new QWidget(this);
    centerLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    gridLayout = new QGridLayout;
    spacer1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    spacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    spacer3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    spacer4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void LoadGif::gif_show(struct gif_loading_struct *content)
{
    if(content->stop == 1)
    {
        loading->stop();
        this->hide();
    }
    else
    {
        if(content->tposition)
        {
            label_gif->setParent(centerWidget);
            label_text->setParent(centerWidget);

            loading->setFileName(content->name);
            label_gif->setMovie(loading);
            loading->start();

            if (content->text) {
                label_text->setText(content->text);
                if (content->style)
                    label_text->setStyleSheet(content->style);
            }


            centerLayout->addWidget(label_gif);
            centerLayout->addWidget(label_text);
            centerWidget->setLayout(centerLayout);
            /*
            Qt::AlignLeft	0x0001	Aligns with the left edge.
            Qt::AlignRight	0x0002	Aligns with the right edge.
            Qt::AlignHCenter	0x0004	Centers horizontally in the available space.
            Qt::AlignJustify	0x0008	Justifies the text in the available space.
            The vertical flags are:

            Constant	Value	Description
            Qt::AlignTop	0x0020	Aligns with the top.
            Qt::AlignBottom	0x0040	Aligns with the bottom.
            Qt::AlignVCenter
*/
            switch(content->tposition)
            {
                case 4:
                    centerLayout->setDirection(QBoxLayout::RightToLeft);
                    label_gif->setMinimumWidth(loading->currentPixmap().width());
                    label_gif->setMinimumHeight(loading->currentPixmap().height());
                    label_gif->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                    label_text->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    break;
                case 3:
                    centerLayout->setDirection(QBoxLayout::TopToBottom);
                    label_gif->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
                    label_text->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
                    break;
                case 2:
                    centerLayout->setDirection(QBoxLayout::LeftToRight);
                    label_gif->setMinimumWidth(loading->currentPixmap().width());
                    label_gif->setMinimumHeight(loading->currentPixmap().height());
                    label_gif->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                    label_text->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                    break;
                case 1:
                default:
                    centerLayout->setDirection(QBoxLayout::BottomToTop);
                    label_gif->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
                    label_text->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
                    break;
            }

            gridLayout->addWidget(centerWidget, 1, 1, 1, 1);
            gridLayout->addItem(spacer1, 0, 1, 1, 1);
            gridLayout->addItem(spacer2, 1, 2, 1, 1);
            gridLayout->addItem(spacer3, 1, 0, 1, 1);
            gridLayout->addItem(spacer4, 2, 1, 1, 1);

            setLayout(gridLayout);
            this->showMaximized();
        }
        else
        {
            label_gif->setParent(this);
            label_text->setParent(this);

            loading->setFileName(content->name);
            label_gif->setMovie(loading);
            loading->start();

            int width = loading->currentPixmap().width();
            int height = loading->currentPixmap().height();

            if (content->w && content->h)
                label_gif->setGeometry(content->x,content->y,content->w,content->h);
            else //center
                label_gif->setGeometry((screenWidth-width)/2, (screenHeight-height)/2, width, height);

            if (content->text) {
                label_text->setText(content->text);
                if (content->style)
                    label_text->setStyleSheet(content->style);
                /*
                 * (tx,ty) = (0, 1): text under gif
                 * (tx,ty) = (1, 0): text on the right
                 */
                if (content->tw && content->th)
                    label_text->setGeometry(content->tx,content->ty,content->tw,content->th);
                else{
                    if ((content->tx == 0) && (content->ty == 1)) {
                        label_text->setGeometry(label_gif->x(), label_gif->y()+label_gif->height()+10,
                                                label_gif->width(), screenHeight-label_gif->y()-label_gif->height());
                        label_text->setAlignment(Qt::AlignTop);
                    }
                    else if ((content->tx == 1) && (content->ty == 0))
                        label_text->setGeometry(label_gif->x()+label_gif->width()+5, label_gif->y(),
                                                screenWidth-label_gif->width(), label_gif->height());
                    else {
                        label_text->setGeometry(0, label_gif->y()-50, screenWidth, 50);
                        label_text->setAlignment(Qt::AlignCenter);
                    }
                }
            }
            this->showMaximized();
        }
    }
}

LoadGif::~LoadGif()
{
}
