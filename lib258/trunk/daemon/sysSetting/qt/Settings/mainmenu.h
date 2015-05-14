#ifndef MAINMENU_H
#define MAINMENU_H

#include <QDialog>
#include <QSettings>
#include <QStringList>
#include <QToolButton>
#include <QString>
#include <QDialog>
#include "readsetting.h"
#include "gsm.h"
#include "date.h"
#include "module.h"
#include "screen.h"
#include "wifi.h"
#include "ethernet.h"
#include "gps.h"

namespace Ui {
class MainMenu;
}

class MainMenu : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainMenu(QWidget *parent = 0);
    ~MainMenu();
    
private slots:
    void on_pbGsm_clicked();
    void on_pbDateTime_clicked();
    void on_pbModle_clicked();
    void on_pbScreen_clicked();
    void on_pbWifi_clicked();
    void on_pbCancel_clicked();
    void on_pbEthernet_clicked();
    void on_pbGps_clicked();
    void on_pbModen_clicked();

private:
    Ui::MainMenu *ui;
    QSettings *settings;
    QStringList groups,gpKeys,gpKeysValue;
    //ReadSetting readSetting;
    QString strKey,strValue;
};

#endif // MAINMENU_H
