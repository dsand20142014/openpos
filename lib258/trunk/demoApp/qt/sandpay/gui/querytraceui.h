#ifndef QUERYTRACEUI_H
#define QUERYTRACEUI_H

#include <QDialog>

namespace Ui {
class QueryTraceUI;
}

class QueryTraceUI : public QDialog
{
    Q_OBJECT
    
public:
    explicit QueryTraceUI(QWidget *parent = 0);
    ~QueryTraceUI();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::QueryTraceUI *ui;

    unsigned short	uiIndex,uioldIndex;
    void setData();
};

#endif // QUERYTRACEUI_H
