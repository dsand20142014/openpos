#ifndef MAINKEY6_H
#define MAINKEY6_H

#include <QDialog>

namespace Ui {
class MainKey6;
}

class MainKey6 : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainKey6(QWidget *parent = 0);
    ~MainKey6();
    
private slots:
    void on_pushButton_clicked();

private:
    Ui::MainKey6 *ui;
};

#endif // MAINKEY6_H
