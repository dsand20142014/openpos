#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>

class Button : public QPushButton
{
    Q_OBJECT
public:
    /* type=0: blue; type=1: red; type=2: yellow; type=3: green */
    explicit Button(const QString & text, int type=0, QWidget *parent = 0);
    void loadStyleSheet(const QString &sheetName, QWidget *obj = 0);
};

#endif // BUTTON_H
