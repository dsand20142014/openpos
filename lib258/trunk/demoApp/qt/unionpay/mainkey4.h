#ifndef MAINKEY4_H
#define MAINKEY4_H

#include <QDialog>

namespace Ui {
class mainkey4;
}

class mainkey4 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey4(QWidget *parent = 0);
    ~mainkey4();
    
private slots:
    void on_pushButton_7_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_1_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::mainkey4 *ui;
};

#endif // MAINKEY4_H
