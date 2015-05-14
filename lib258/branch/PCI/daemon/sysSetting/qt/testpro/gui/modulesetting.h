#ifndef MODULESETTING_H
#define MODULESETTING_H

#include <QWidget>
#include <QSettings>
#include <QtGui>
namespace Ui {
class ModuleSetting;
}

#define MAXCOUNT 10


class ModuleSetting : public QWidget
{
    Q_OBJECT
    
public:
    explicit ModuleSetting(QWidget *parent = 0);
    ~ModuleSetting();
    
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::ModuleSetting *ui;

    void setValue();
    void getValue();
    QStringList keyList;
    QStringList noteList;

    QLabel* label[MAXCOUNT];
    QCheckBox* box[MAXCOUNT];
    QCheckBox* box2[MAXCOUNT];

    QStringList strList;

    int analysisDesc(QString desc, QString *brief, QString *valueList);
    void keyPressEvent(QKeyEvent *);
    void splitStr(QMap<QString ,int>& set,QStringList list,QString beginStr,QString flagStr );
    void readFile();
    void writeFile();
};

#endif // MODULESETTING_H
