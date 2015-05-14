#ifndef MAINKEY81_H
#define MAINKEY81_H

#include <QDialog>

namespace Ui {
class mainkey81;
}

class mainkey81 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey81(QWidget *parent = 0);
    ~mainkey81();
    
private slots:
    void on_pushButton_5_clicked();

private:
    Ui::mainkey81 *ui;
};

#endif // MAINKEY81_H
