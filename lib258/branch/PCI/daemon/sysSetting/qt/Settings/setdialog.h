#ifndef SETDIALOG_H
#define SETDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QPushButton>
#include <QLabel>
#include <QTextBrowser>
#include <QLineEdit>

class SetDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SetDialog(QWidget *parent = 0);
    void setTitle(QString title);
    void setNote(QString note);
    void setValue(QString value);
    void displayDialog();
signals:
    
public slots:

private:
    QString slTitle,slNote;
    QString sValue;
    QLabel *lTitle;
    QTextBrowser *tbNote;
    QLineEdit *leValue;
    QPushButton *cancelButton;
    QPushButton *enterButton;
};

#endif // SETDIALOG_H
