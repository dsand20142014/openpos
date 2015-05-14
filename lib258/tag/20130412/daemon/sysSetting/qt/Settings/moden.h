#ifndef MODEN_H
#define MODEN_H

#include <QDialog>

namespace Ui {
class Moden;
}

class Moden : public QDialog
{
    Q_OBJECT
    
public:
    explicit Moden(QWidget *parent = 0);
    ~Moden();
    
private slots:
    void on_pbCancel_clicked();

    void on_pbEnter_clicked();

private:
    Ui::Moden *ui;
};

#endif // MODEN_H
