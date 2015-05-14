#ifndef CHANGEMANAGERPSW_H
#define CHANGEMANAGERPSW_H

#include <QDialog>

namespace Ui {
class ChangeManagerPsw;
}

class ChangeManagerPsw : public QDialog
{
    Q_OBJECT
    
public:
    explicit ChangeManagerPsw(QWidget *parent = 0);
    ~ChangeManagerPsw();
    
private:
    Ui::ChangeManagerPsw *ui;

private slots:
    void slot_finished1();
    void slot_finished2();

    void on_pushButton_clicked();
};

#endif // CHANGEMANAGERPSW_H
