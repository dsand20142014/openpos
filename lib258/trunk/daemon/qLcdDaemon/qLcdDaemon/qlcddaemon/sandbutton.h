#ifndef SANDBUTTON_H
#define SANDBUTTON_H

#include <QToolButton>

class SandButton : public QToolButton
{
    Q_OBJECT
public:
    explicit SandButton(QWidget *parent = 0);

signals:


public slots:
    void send_key();

public:
    int key_value;

};

#endif // SANDBUTTON_H
