#ifndef ONCARDUI_H
#define ONCARDUI_H

#include <QDialog>

namespace Ui {
class OnCardUI;
}

class OnCardUI : public QDialog
{
    Q_OBJECT
    
public:
    explicit OnCardUI(QWidget *parent = 0);
    ~OnCardUI();
    
public slots:
    void on_pushButton_5_clicked();
signals:
    void signal_oncard();
private slots:
    void on_pushButton_8_clicked();

private:
    Ui::OnCardUI *ui;
};

#endif // ONCARDUI_H
