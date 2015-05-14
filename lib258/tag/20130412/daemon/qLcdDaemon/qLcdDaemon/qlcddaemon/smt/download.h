
#pragma once

#include "tablewidget.h"
#include "downloading.h"
#include "../gui/baseForm.h"

class Download : public BaseForm
{
    Q_OBJECT
    
public:
    explicit Download(int usb = 0, QWidget *parent = 0);
    ~Download();

/*protected:
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
*/
private:
    void getFile(QDir dir);
    void checkApp();
    tableWidget *listApp;
    QListWidget *listWidget;
    QString currentDir;
    char *conffile; // "/tmp/pln/conf.ini"
    int USB;
    QMessageBox msgBox;
    QPushButton *btnCover;
    QPushButton *btnUpdate;

private slots:
    void slot_btnOk_clicked();
    void slot_listWidget_clicked(QModelIndex);
};


