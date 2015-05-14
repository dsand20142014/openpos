#ifndef RETYPEBYTRACE_H
#define RETYPEBYTRACE_H

#include <QDialog>

namespace Ui {
class ReTypeByTrace;
}

class ReTypeByTrace : public QDialog
{
    Q_OBJECT
    
public:
    explicit ReTypeByTrace(QWidget *parent = 0);
    ~ReTypeByTrace();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::ReTypeByTrace *ui;
};

#endif // RETYPEBYTRACE_H
