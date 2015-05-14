#ifndef SCREEN_H
#define SCREEN_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QString>
#include "readsetting.h"

namespace Ui {
class Screen;
}

class Screen : public QDialog
{
    Q_OBJECT
    
public:
    explicit Screen(QWidget *parent = 0);
    ~Screen();
    
private slots:
    void on_pbCancel_clicked();
    void on_pbEnter_clicked();

private:
    Ui::Screen *ui;
    QSettings *settings;

    void displayValue(void);
};

#endif // SCREEN_H
