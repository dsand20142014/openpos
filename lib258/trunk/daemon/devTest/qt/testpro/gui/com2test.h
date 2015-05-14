#ifndef COM2TEST_H
#define COM2TEST_H

#include <QWidget>
#include "baseForm.h"

class COM2Test : public BaseForm
{
    Q_OBJECT
    
public:
    explicit COM2Test(QWidget *parent = 0);
    ~COM2Test();
    
private:
    QTextEdit *textEdit;
    void get_Result(void);
};

#endif // COM2TEST_H
