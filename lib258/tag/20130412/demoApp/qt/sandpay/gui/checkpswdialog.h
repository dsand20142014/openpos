#ifndef CHECKPSWDIALOG_H
#define CHECKPSWDIALOG_H

#include <QDialog>

namespace Ui {
class CheckPswDialog;
}

class CheckPswDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CheckPswDialog(QWidget *parent = 0);
    ~CheckPswDialog();
    
private slots:
    void on_pushButton_clicked();
    void slot_inputpsw();

   // void on_pushButton_2_clicked();

private:
    Ui::CheckPswDialog *ui;
};

#endif // CHECKPSWDIALOG_H
