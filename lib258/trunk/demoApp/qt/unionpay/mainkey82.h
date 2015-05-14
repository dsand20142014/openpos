#ifndef MAINKEY82_H
#define MAINKEY82_H

#include <QDialog>

namespace Ui {
class mainkey82;
}

class mainkey82 : public QDialog
{
    Q_OBJECT
    
public:
    explicit mainkey82(QWidget *parent = 0);
    ~mainkey82();
    
private slots:
    void on_pushButton_4_clicked();

private:
    Ui::mainkey82 *ui;
};

#endif // MAINKEY82_H
