#ifndef ICCWIDONW_H
#define ICCWIDONW_H

#include <QWidget>
#include "TypeDef.h"
#include "EMVDef.h"

namespace Ui {
class ICCWidonw;
}

class ICCWidonw : public QWidget
{
    Q_OBJECT
    
public:
    explicit ICCWidonw(QWidget *parent = 0);
    ~ICCWidonw();

    void setText(QString text,TRANSTYPE	enTransType);
    unsigned long	uiAmount;
    unsigned long uiOtherAmount;

private slots:
    void enter_slot();

private:
    Ui::ICCWidonw *ui;

    TRANSTYPE	enTransType;
    UCHAR ucResult;
    UCHAR	EMVTransProcess(TRANSTYPE enTransType,UINT uiAmount,UINT uiOtherAmount);
    UCHAR SelectAccountType(void);

};

#endif // ICCWIDONW_H
