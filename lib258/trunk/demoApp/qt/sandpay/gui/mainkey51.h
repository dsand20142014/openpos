#ifndef MAINKEY51_H
#define MAINKEY51_H

#include <QDialog>

namespace Ui {
class MainKey51;
}

class MainKey51 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey51(QWidget *parent = 0);
    ~MainKey51();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainKey51 *ui;
    void setInitValue();
};

#endif // MAINKEY51_H
