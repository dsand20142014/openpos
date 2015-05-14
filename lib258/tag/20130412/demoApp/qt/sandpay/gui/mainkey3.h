#ifndef MAINKEY3_H
#define MAINKEY3_H

#include <QDialog>

namespace Ui {
class MainKey3;
}

class MainKey3 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey3(QWidget *parent = 0);
    ~MainKey3();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainKey3 *ui;

};

#endif // MAINKEY3_H
