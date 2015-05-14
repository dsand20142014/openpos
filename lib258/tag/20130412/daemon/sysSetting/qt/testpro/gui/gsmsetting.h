#ifndef GSMSETTING_H
#define GSMSETTING_H

#include <QWidget>
#include <QtGui>

namespace Ui {
class GsmSetting;
}

class GsmSetting : public QWidget
{
    Q_OBJECT
    
public:
    explicit GsmSetting(QWidget *parent = 0);
    ~GsmSetting();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::GsmSetting *ui;
   // QSettings set;
    void setValue();
    void getValue();
    QStringList keyList;
    QStringList noteList;
    void copy_from_tmpfile(char *file);

    int config_pap_chap();
    int analysisDesc(QString desc, QString *brief, QString *valueList);
    void keyPressEvent(QKeyEvent *);

};

#endif // GSMSETTING_H
