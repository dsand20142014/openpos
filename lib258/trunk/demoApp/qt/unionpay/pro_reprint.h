#ifndef PRO_REPRINT_H
#define PRO_REPRINT_H

#include <QDialog>

namespace Ui {
class Pro_Reprint;
}

class Pro_Reprint : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_Reprint(QWidget *parent = 0);
    ~Pro_Reprint();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void destroyWindow(int);
private:
    Ui::Pro_Reprint *ui;
};

#endif // PRO_REPRINT_H
