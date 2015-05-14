#ifndef SAMTEST_H
#define SAMTEST_H

#include <QWidget>
#include <QKeyEvent>
#include "button.h"
#include "baseForm.h"
//namespace Ui {
//class SamTest;
//}

class SamTest : public BaseForm
{
    Q_OBJECT
    
public:
    explicit SamTest(QWidget *parent = 0);
    ~SamTest();
    
private slots:
    void getResult();

private:
//    typedef struct
//    {
//        unsigned short	uiLen;
//        unsigned char	ucStatus;
//        unsigned char	ucSW1;
//        unsigned char	ucSW2;
//        unsigned char	aucData[255];
//    }ISO7816OUT;

   QRadioButton *radBtn1;
   QRadioButton *radBtn2;
   QRadioButton *radBtn3;
   QTextEdit *textEdit;
   QComboBox *comboBox;

protected:
   void keyPressEvent(QKeyEvent *);


};

#endif // SAMTEST_H
