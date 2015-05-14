#ifndef MAINKEY931_H
#define MAINKEY931_H

#include <QDialog>

namespace Ui {
class mainkey931;
}

class mainkey931 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey931(QWidget *parent = 0);
    ~mainkey931();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();


    void on_checkBox_3_stateChanged(int arg1);

    void on_radioButton_5_clicked();

    void on_radioButton_7_clicked();

    void on_radioButton_4_clicked();

private:
    Ui::mainkey931 *ui;
};

#endif // MAINKEY931_H
