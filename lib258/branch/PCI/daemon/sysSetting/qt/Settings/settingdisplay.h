#ifndef SETTINGDISPLAY_H
#define SETTINGDISPLAY_H

#include <QDialog>

namespace Ui {
class SettingDisplay;
}

class SettingDisplay : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingDisplay(QWidget *parent = 0);
    ~SettingDisplay();
    
private:
    Ui::SettingDisplay *ui;
};

#endif // SETTINGDISPLAY_H
