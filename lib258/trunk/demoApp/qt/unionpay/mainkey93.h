#ifndef MAINKEY93_H
#define MAINKEY93_H

#include <QDialog>

namespace Ui {
class mainkey93;
}

class mainkey93 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey93(QWidget *parent = 0);
    ~mainkey93();
    
private slots:

    void on_pushButton_9_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::mainkey93 *ui;
};

#endif // MAINKEY93_H
