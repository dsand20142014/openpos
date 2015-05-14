#ifndef IMFRAME_H
#define IMFRAME_H

#include <QWSInputMethod>
#include <QTimer>
#include <QSignalMapper>

typedef enum
{
    NUM,
    ABC,
    CAPABC,
    PINYIN,
    IMEMAX///仅用来表示输入法的数量
}
ImeType;

class InputWidget;
class PinyinDialog;
class IMFrame:public QWSInputMethod
{
Q_OBJECT
public:
    IMFrame();
    ~IMFrame();
    void setMicroFocus(int, int);
    void updateHandler(int);
    bool filter(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
    bool inputNum(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
    bool inputAbc(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
    bool inputCapAbc(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
    bool inputPinyin(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat);
    void sendPreString(const QString & str);
    void sendComString(const QString & str);


private:
    ImeType imeType;
    InputWidget* inputwidget;
    QString content;
    QTimer* abcTimer;
    PinyinDialog* pinyinDialog;

signals:
    void imeTypeChanged(ImeType t);

public slots:
    void changeImeType(void);
    void setImeType(ImeType t);

private slots:
    void sendContent(const QString&);
    void confirmContent();
};

#endif
