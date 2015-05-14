#ifndef MSGINFOUI_H
#define MSGINFOUI_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class MsgInfoUI;
}

class MsgInfoUI : public QDialog
{
    Q_OBJECT
    
public:
    explicit MsgInfoUI(QWidget *parent = 0);
    ~MsgInfoUI();
    
private slots:
    void slottimer1Done();
    void slot_changeValue(uchar*);

//signals:
//    void signal_msg();
    void on_pushButton_clicked();

private:
    Ui::MsgInfoUI *ui;
  //  QTimer sandtimer;
};

#endif // MSGINFOUI_H
