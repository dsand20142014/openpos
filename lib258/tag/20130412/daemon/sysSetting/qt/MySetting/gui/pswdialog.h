#ifndef PSWDIALOG_H
#define PSWDIALOG_H

#include <QDialog>

namespace Ui {
class PswDialog;
}

class PswDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PswDialog(QWidget *parent = 0);
    ~PswDialog();
    
private:
    Ui::PswDialog *ui;

private slots:
    void slotPsw();
};

#endif // PSWDIALOG_H
