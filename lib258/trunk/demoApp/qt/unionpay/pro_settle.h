#ifndef PRO_SETTLE_H
#define PRO_SETTLE_H

#include <QDialog>

namespace Ui {
class Pro_Settle;
}

class Pro_Settle : public QDialog
{
    Q_OBJECT
    
public:
    explicit Pro_Settle(QWidget *parent = 0);
    ~Pro_Settle();
    unsigned char ucTransFlag;
    QTimer *sandtimer;
private slots:
    void slottimer1GetCardInfo();
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();
    void destroyWindow(int);
private:
    Ui::Pro_Settle *ui;
    int gb2312toutf8(char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
    int GetSettleInfo(void);
};

#endif // PRO_SETTLE_H
