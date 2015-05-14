#ifndef MAINKEY9_H
#define MAINKEY9_H

#include <QDialog>

namespace Ui {
class mainkey9;
}

class mainkey9 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey9(QWidget *parent = 0);
    ~mainkey9();
    
private slots:

    void on_pushButton_1_pressed();

    void on_pushButton_2_pressed();

    void on_pushButton_3_pressed();

    void on_pushButton_Close_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::mainkey9 *ui;
};

#endif // MAINKEY9_H
