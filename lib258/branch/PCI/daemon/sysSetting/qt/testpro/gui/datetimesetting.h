#ifndef DATETIMESETTING_H
#define DATETIMESETTING_H

#include <QWidget>
#include <QtGui>

namespace Ui {
class DateTimeSetting;
}

class DateTimeSetting : public QWidget
{
    Q_OBJECT
    
public:
    explicit DateTimeSetting(QWidget *parent = 0);
    ~DateTimeSetting();
    
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::DateTimeSetting *ui;
    void setValue();
    void getValue();
    void keyPressEvent(QKeyEvent *);

};

#endif // DATETIMESETTING_H
