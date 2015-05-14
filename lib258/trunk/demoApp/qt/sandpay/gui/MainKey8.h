#ifndef MAINKEY8_H
#define MAINKEY8_H

#include <QDialog>

namespace Ui {
class MainKey8;
}

class MainKey8 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey8(QWidget *parent = 0);
    ~MainKey8();
    
private slots:
    void on_pushButton_5_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainKey8 *ui;
    void setData();
};

#endif // MAINKEY8_H
