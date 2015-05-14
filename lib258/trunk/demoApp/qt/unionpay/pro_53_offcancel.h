#ifndef PRO_53_OFFCANCEL_H
#define PRO_53_OFFCANCEL_H

#include <QDialog>

namespace Ui {
class Pro_53_OffCancel;
}

class Pro_53_OffCancel : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_53_OffCancel(QWidget *parent = 0);
    ~Pro_53_OffCancel();
    
private slots:
    void on_cancelButton_clicked();

private:
    Ui::Pro_53_OffCancel *ui;
};

#endif // PRO_53_OFFCANCEL_H
