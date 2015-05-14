#ifndef MAINKEY6_H
#define MAINKEY6_H

#include <QDialog>

namespace Ui {
class mainkey6;
}

class mainkey6 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey6(QWidget *parent = 0);
    ~mainkey6();
    
private slots:
    void on_pushButton_1_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
private:
    Ui::mainkey6 *ui;
};

#endif // MAINKEY6_H
