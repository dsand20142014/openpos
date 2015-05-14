#ifndef PinForm_H
#define PinForm_H

#include <QWidget>
#include <QLabel>
#include <QKeyEvent>
#include <QLineEdit>

class PinForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit PinForm(QString text, QWidget *parent = 0);
    ~PinForm();

protected:
    void keyPressEvent(QKeyEvent *);
    //void keyReleaseEvent(QKeyEvent *);

signals:
    void getPin(QString);

private:
    QLineEdit *line;
    QLabel *label;    
};

#endif // PinForm_H
