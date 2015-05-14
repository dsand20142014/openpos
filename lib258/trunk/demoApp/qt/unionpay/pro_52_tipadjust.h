#ifndef PRO_52_TIPADJUST_H
#define PRO_52_TIPADJUST_H

#include <QDialog>

namespace Ui {
class Pro_52_TipAdjust;
}

class Pro_52_TipAdjust : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_52_TipAdjust(QWidget *parent = 0);
    ~Pro_52_TipAdjust();
    
private slots:
    void on_cancelButton_clicked();

private:
    Ui::Pro_52_TipAdjust *ui;
};

#endif // PRO_52_TIPADJUST_H
