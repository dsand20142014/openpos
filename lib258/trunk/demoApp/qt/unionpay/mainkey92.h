#ifndef MAINKEY92_H
#define MAINKEY92_H

#include <QDialog>

namespace Ui {
class mainkey92;
}

class mainkey92 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey92(QWidget *parent = 0);
    ~mainkey92();
    
private slots:
    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

private:
    Ui::mainkey92 *ui;
};

#endif // MAINKEY92_H
