#ifndef SCREENSETTING_H
#define SCREENSETTING_H

#include <QWidget>
#include <QtGui>
namespace Ui {
class ScreenSetting;
}

class ScreenSetting : public QWidget
{
    Q_OBJECT
    
public:
    explicit ScreenSetting(QWidget *parent = 0);
    ~ScreenSetting();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

   // void on_pushButton_3_clicked();

private:
    Ui::ScreenSetting *ui;
    QSettings *set;
    void setValue();
    void getValue();
    QStringList keyList;
    QStringList noteList;

    void keyPressEvent(QKeyEvent *);
    int analysisDesc(QString desc, QString *brief, QString *valueList);

};

#endif // SCREENSETTING_H
