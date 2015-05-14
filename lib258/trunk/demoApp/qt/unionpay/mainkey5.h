#ifndef MAINKEY5_H
#define MAINKEY5_H

#include <QDialog>

namespace Ui {
class mainkey5;
}

class mainkey5 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey5(QWidget *parent = 0);
    ~mainkey5();
    
private slots:
    void on_pushButton_4_clicked();

    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::mainkey5 *ui;
};

#endif // MAINKEY5_H
