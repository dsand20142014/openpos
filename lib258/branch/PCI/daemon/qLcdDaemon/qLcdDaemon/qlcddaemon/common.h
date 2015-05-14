#ifndef COMMON_H
#define COMMON_H

#include <QtGui>

class messageBox : public QMessageBox
{
    Q_OBJECT

public:
    explicit messageBox(const QString & text="", QWidget *parent=0);
    ~messageBox();

};

#endif // COMMON_H
