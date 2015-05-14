#ifndef WIFIPSWDIALOG_H
#define WIFIPSWDIALOG_H

#include <QDialog>
#include <QDesktopWidget>
#include <QTimer>

namespace Ui {
class WifiPswDialog;
}

class WifiPswDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit WifiPswDialog(QString title,QString strength,QWidget *parent = 0);
    ~WifiPswDialog();
    QString curPsw;
//    void hidePswLine();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::WifiPswDialog *ui;

};

#endif // WIFIPSWDIALOG_H
