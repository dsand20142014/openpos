#ifndef PRO_51_AUTHFINISHOFF_H
#define PRO_51_AUTHFINISHOFF_H

#include <QDialog>

namespace Ui {
class Pro_51_AuthFinishOff;
}

class Pro_51_AuthFinishOff : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_51_AuthFinishOff(QWidget *parent = 0);
    ~Pro_51_AuthFinishOff();
    
private slots:
    void on_cancelButton_clicked();

private:
    Ui::Pro_51_AuthFinishOff *ui;
};

#endif // PRO_51_AUTHFINISHOFF_H
