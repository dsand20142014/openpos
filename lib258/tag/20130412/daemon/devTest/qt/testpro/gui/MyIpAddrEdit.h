#include <QWidget>
#include <QFrame>
class QLineEdit;
class QLabel;
class MyIpPartLineEdit;

class MyIpAddrEdit : public QFrame
{
    Q_OBJECT
public:
    MyIpAddrEdit(QWidget* pParent = 0);
    ~MyIpAddrEdit();

    void settext(const QString &text);
    QString text();
    bool ip1Hasfocus();
    bool ip2Hasfocus();
    bool ip3Hasfocus();
    bool ip4Hasfocus();
    void setip2focus();
    void setip3focus();
    void setip4focus();


signals:
    void textchanged(const QString& text);
    void textedited(const QString &text);
    void endEdit();

private slots:
    void textchangedslot(const QString& text);
    void texteditedslot(const QString &text);


private:
    MyIpPartLineEdit *ip_part1_;
    MyIpPartLineEdit *ip_part2_;
    MyIpPartLineEdit *ip_part3_;
    QLineEdit *ip_part4_;

    QLabel *labeldot1_;
    QLabel *labeldot2_;    
    QLabel *labeldot3_;
};
