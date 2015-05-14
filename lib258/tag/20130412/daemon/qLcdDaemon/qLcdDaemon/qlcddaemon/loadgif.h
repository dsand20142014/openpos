#ifndef LOADGIF_H
#define LOADGIF_H

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
    QMovie *loading;
};

#endif // LOADGIF_H
