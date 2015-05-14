#ifndef WIFIENSUREDIALOG_H
#define WIFIENSUREDIALOG_H

#include <QDialog>
#include <QDesktopWidget>
namespace Ui {
class WifiEnsureDialog;
}

class WifiEnsureDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit WifiEnsureDialog(QString title,QString strength,QWidget *parent = 0);
    ~WifiEnsureDialog();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::WifiEnsureDialog *ui;
};

#endif // WIFIENSUREDIALOG_H
