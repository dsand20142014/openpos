#ifndef PINYINDIALOG_H
#define PINYINDIALOG_H

#include <QDialog>
#include <QLabel>
#include "qimeframe.h"
#include "qclickablelabel.h"

namespace Ui {
class PinyinDialog;
}

class PinyinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PinyinDialog(IMFrame* im);
    ~PinyinDialog();
    bool keyPush(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);

private:
    Ui::PinyinDialog *ui;
    int cadidateNum;
    int page;
    int maxPage;
    QClickableLabel* label[5];
    IMFrame* imf;
    QTimer* timer;
    QString mycontent;
    QString oldcontent;
    bool signFlag;
    QStringList signList;
    ImeType imeType_temp;

    void on_pagedown_clicked();
    void on_pageup_clicked();
signals:
    void alreadyGetCandidate(int candidateNum);

public slots:
    void getCandidate(QString pinyinStr);
    void showCandidate(int pageIndex);

    void label1clicked();
    void label2clicked();
    void label3clicked();
    void label4clicked();
    void label5clicked();

private slots:
    void sendMyContent(const QString&);
    void confirmMyContent();
    void changeImeType(ImeType);
};

#endif // PINYINDIALOG_H
