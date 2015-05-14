#ifndef MAINKEY7_H
#define MAINKEY7_H

#include <QDialog>

namespace Ui {
class mainkey7;
}

class mainkey7 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey7(QWidget *parent = 0);
    ~mainkey7();
    
private slots:
    void on_pushButton_9_clicked();


    void on_pushButton_1_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_2_clicked();


    void on_pushButton_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::mainkey7 *ui;
};

#endif // MAINKEY7_H
