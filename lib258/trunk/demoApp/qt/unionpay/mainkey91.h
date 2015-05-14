#ifndef MAINKEY91_H
#define MAINKEY91_H

#include <QDialog>

namespace Ui {
class mainkey91;
}

class mainkey91 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey91(QWidget *parent = 0);
    ~mainkey91();
    
private slots:
    void on_pushButton_7_clicked();

private:
    Ui::mainkey91 *ui;
};

#endif // MAINKEY91_H
