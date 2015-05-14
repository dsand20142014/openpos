#ifndef MAINKEY8_H
#define MAINKEY8_H

#include <QDialog>

namespace Ui {
class mainkey8;
}

class mainkey8 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey8(QWidget *parent = 0);
    ~mainkey8();
    
private slots:


    void on_pushButton_1_pressed();

    void on_pushButton_2_pressed();
    void on_pushButton_3_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_5_clicked();

private:
    Ui::mainkey8 *ui;
};

#endif // MAINKEY8_H
