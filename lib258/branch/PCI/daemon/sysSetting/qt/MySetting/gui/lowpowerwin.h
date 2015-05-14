#ifndef LOWPOWERWIN_H
#define LOWPOWERWIN_H

#include <QtGui>

#include "baseForm.h"

class LowPowerWin : public BaseForm
{
    Q_OBJECT
    
public:
    explicit LowPowerWin(QWidget *parent = 0);
    ~LowPowerWin();
    
private slots:
    void slot_btnOK();

private:
    QRadioButton *radio1;
    QRadioButton *radio2;
    QGroupBox *groupBox;
};

#endif // LOWPOWERWIN_H
