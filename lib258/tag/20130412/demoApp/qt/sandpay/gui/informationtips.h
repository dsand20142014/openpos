#ifndef INFORMATIONTIPS_H
#define INFORMATIONTIPS_H

#include <QDialog>

namespace Ui {
class InformationTips;
}

class InformationTips : public QDialog
{
    Q_OBJECT
    
public:
    explicit InformationTips(QWidget *parent = 0);
    ~InformationTips();

    void setText_UI(QString );
    void setButton(bool,int ms = 0);
    
private slots:
    void on_pushButton_clicked();

private:
    Ui::InformationTips *ui;

};

#endif // INFORMATIONTIPS_H
