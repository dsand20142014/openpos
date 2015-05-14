#ifndef MAINKEY4_H
#define MAINKEY4_H

#include <QDialog>

namespace Ui {
class MainKey4;
}

class MainKey4 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey4(QWidget *parent = 0);
    ~MainKey4();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainKey4 *ui;
};

#endif // MAINKEY4_H
