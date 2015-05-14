#ifndef KEYCLEARSETTING_H
#define KEYCLEARSETTING_H

#include "baseForm.h"
#include "button.h"
#include <QtGui>

class KeyClearSetting : public BaseForm
{
    Q_OBJECT
    
public:
    explicit KeyClearSetting(QWidget *parent = 0);
    ~KeyClearSetting();
    
private:
    Button *btnAuto;
    QLabel *labelA;

    QStringList childGroups;
    int id[30];
    int size;


private slots:
    void slotAuto();
};

#endif // KEYCLEARSETTING_H
