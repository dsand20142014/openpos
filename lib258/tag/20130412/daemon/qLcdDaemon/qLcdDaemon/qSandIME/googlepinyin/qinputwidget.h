#ifndef INPUTWIDGET_H
#define INPUTWIDGET_H

#include <QWidget>
#include "qimeframe.h"

namespace Ui {
class InputWidget;
}

class QLineEdit;
class QWSInputMethod;
class InputWidget:public QWidget
{
Q_OBJECT
public:
    InputWidget(IMFrame* im);
    ~InputWidget();

public slots:
    void showImeType(ImeType t);

private:
    Ui::InputWidget *ui;
};

#endif
