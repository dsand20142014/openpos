#ifndef Downloading_H
#define Downloading_H

#include <QtGui>
#include <QTime>
#include "../gui/baseForm.h"
#include "common.h"

extern "C"{
#include "param.h"
#include "applist.h"
#include "updateService.h"
#define MALLOC_CHECK_ 1
}
/*
#define malloc(s) debug_malloc(s, __FILE__, __LINE__, __func__)
#define free(p)  do {\
        printf("%s:%d:%s:free(0x%lx)\n", __FILE__, __LINE__, __func__, (unsigned long)p);\
        if (p) {\
          free(p);\
          p = NULL;\
       }\
} while (0)
}*/

class Downloading : public BaseForm
{
    Q_OBJECT
    
public:
    explicit Downloading(bool process = false, QWidget *parent = 0);
    ~Downloading();
    
private:
    QLabel *label_gif;
    QMovie *loading;
    QTimer timer;
    QProgressBar *progressBar;
    unsigned int etcFile;
    unsigned int etcSection1;

    int msqid;
    UP_SER_MSG *msg;

    void start();
    void stop();
    void stop_error();

private slots:
    void slot_process();
};

#endif // Download_H
