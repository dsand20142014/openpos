#ifndef MAINKEY94_H
#define MAINKEY94_H

#include <QDialog>

namespace Ui {
class mainkey94;
}

class mainkey94 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey94(QWidget *parent = 0);
    ~mainkey94();
    
private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::mainkey94 *ui;
};

#endif // MAINKEY94_H
