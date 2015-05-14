#include "syswindow.h"
#include "gui/button.h"
#include <QPalette>
#include <QPixmap>
#include <QKeyEvent>
#include <QLayout>

extern "C"
{
#include "sand_key.h"
}


IButton::IButton(QString icon, QWidget *parent) : QPushButton(parent)
{
    //setFocusPolicy(Qt::NoFocus);
    setFixedSize(52, 52);
    setIconSize(QSize(48, 48));
    setIcon(QIcon(icon));
}

About::About(QWidget *parent) : BaseForm(tr("IPS420 About"), parent)
{
    textedit = new QTextEdit();
    textedit->setReadOnly(true);
    textedit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layout->setMargin(0);
    layout->addWidget(textedit);
    btnExit->hide();
    btnOk->show();

    connect(btnOk, SIGNAL(clicked()), this, SLOT(hide()));
}

void About::keyPressEvent(QKeyEvent *e)
{
    Os__set_key_ensure(200);

    switch(e->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Escape:
            hide();
        break;
    }
    e->accept();
}

void About::mousePressEvent(QMouseEvent *)
{
    Os__set_key_ensure(200);
}

About::~About()
{

}

SysWindow::SysWindow(QWidget *parent, QStringList *argv) : QWidget(parent, Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(QPixmap("./image/surfacing.png")));
    setPalette(palette);
    setAutoFillBackground(true);

    label = new QLabel(tr("Please input the password:"));
    label->setFont(QFont("", 14));

    lineEdit = new QLineEdit();
    lineEdit->setEchoMode(QLineEdit::Password);
    lineEdit->setFocus();

    pushButton_enter = new Button(tr("Ok"));
    pushButton_reset = new Button(tr("Clear"), 1);

    IButton *pushButton_setting = new IButton("image/setting.png");
    IButton *pushButton_test = new IButton("image/test.png");
    IButton *pushButton_download = new IButton("image/download.png");
    IButton *pushButton_about = new IButton("image/about.png");
    IButton *pushButton_cancel = new IButton("image/exit.png");

    QLabel *label_set = new QLabel(tr("SETTING"));
    label_set->setAlignment(Qt::AlignCenter|Qt::AlignTop);
    QLabel *label_test = new QLabel(tr("TEST"));
    label_test->setAlignment(Qt::AlignCenter|Qt::AlignTop);
    QLabel *label_smt = new QLabel(tr("SMT"));
    label_smt->setAlignment(Qt::AlignCenter|Qt::AlignTop);
    QLabel *label_about =  new QLabel(tr("ABOUT"));
    label_about->setAlignment(Qt::AlignCenter|Qt::AlignTop);
    QLabel *label_exit =  new QLabel(tr("EXIT"));
    label_exit->setAlignment(Qt::AlignCenter|Qt::AlignTop);

    btnwin = new QWidget();
    QGridLayout *glayout = new QGridLayout(btnwin);
    glayout->setSpacing(5);
    glayout->setVerticalSpacing(10);
    glayout->addWidget(pushButton_setting, 0, 0, Qt::AlignCenter);
    glayout->addWidget(pushButton_test, 0, 1, Qt::AlignCenter);
    glayout->addWidget(pushButton_download, 0, 2, Qt::AlignCenter);
    glayout->addWidget(pushButton_about, 2, 0, Qt::AlignCenter);
    glayout->addWidget(pushButton_cancel, 2, 1, Qt::AlignCenter);
    glayout->addWidget(label_set, 1, 0);
    glayout->addWidget(label_test, 1, 1);
    glayout->addWidget(label_smt, 1, 2);
    glayout->addWidget(label_about, 3, 0);
    glayout->addWidget(label_exit, 3, 1);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(pushButton_reset, Qt::AlignBottom);
    hlayout->addWidget(pushButton_enter, Qt::AlignBottom);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addSpacing(15);
    vlayout->addWidget(btnwin);
    vlayout->addSpacing(20);
    vlayout->addWidget(label);
    vlayout->addWidget(lineEdit);
    vlayout->addSpacerItem(new QSpacerItem(50, 100));
    vlayout->addLayout(hlayout);
    setLayout(vlayout);

    aboutWin = new About();

    QFile file("/daemon/readme");
    if (file.open(QFile::ReadOnly)) {
        aboutWin->textedit->setText(file.readAll());
        file.close();
    }else {
        aboutWin->textedit->setText("NO INFORMATION!");
    }

    loading = new QMovie("image/loading.gif");
    giflabel = new QLabel();
    giflabel->setWindowFlags(Qt::FramelessWindowHint);
    giflabel->setAlignment(Qt::AlignCenter);
    giflabel->setStyleSheet("background-color: transparent");
    giflabel->setGeometry(0, 25, 240, 320-25);
    giflabel->setMovie(loading);
    giflabel->hide();

    myProcess = new QProcess();

    aboutWin->hide();
    btnwin->hide();

    sysArgv = argv;
    //timerGif.setSingleShot(true);

    connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(pushButton_reset, SIGNAL(clicked()), lineEdit, SLOT(clear()));
    connect(pushButton_enter, SIGNAL(clicked()), this, SLOT(on_pushButton_enter_clicked()));
    connect(pushButton_setting, SIGNAL(clicked()), this, SLOT(on_pushButton_setting_clicked()));
    connect(pushButton_test, SIGNAL(clicked()), this, SLOT(on_pushButton_test_clicked()));
    connect(pushButton_about, SIGNAL(clicked()), this, SLOT(on_pushButton_about_clicked()));
    connect(pushButton_download, SIGNAL(clicked()), this, SLOT(on_pushButton_download_clicked()));
    //connect(myProcess, SIGNAL(started()), this, SLOT(slot_process_start()));
    connect(myProcess, SIGNAL(finished(int)), this, SLOT(slot_process_finish(int)));
    //connect(&timerGif, SIGNAL(timeout()), loading, SLOT(stop()));
}

SysWindow::~SysWindow()
{
    //Os__set_key_ensure(KEY_SYSWIN_EXIT); // for Os_Wait_Event/Os_Wait_Event_NoKey conflict in MAG/MFR event bug
}

void SysWindow::keyPressEvent(QKeyEvent *e)
{    
    switch(e->key())
    {
    case Qt::Key_Escape:
        //Os__set_key_ensure(200);
        this->close();
        break;
    case Qt::Key_Enter:
        //Os__set_key_ensure(KEY_ENTER);
        on_pushButton_enter_clicked();
        break;
    case Qt::Key_1:
        on_pushButton_setting_clicked();
        break;
    case Qt::Key_2:
        on_pushButton_test_clicked();
        break;
    case Qt::Key_3:
        on_pushButton_download_clicked();
        break;
    case Qt::Key_4:
        on_pushButton_about_clicked();
        break;
    case Qt::Key_5:
        close();
        break;
    default:
        //Os__set_key_ensure(200);
        break;
    }
}

void SysWindow::on_pushButton_enter_clicked()
{
    if(lineEdit->text()=="258036")
    {
        pushButton_reset->hide();
        pushButton_enter->hide();
        lineEdit->clear();
        label->hide();
        lineEdit->hide();
        btnwin->show();
    } else if (lineEdit->text()=="031014") { //add 130325 for smt
        on_pushButton_download_clicked();
    }
    else
    {                
        lineEdit->clear();
        lineEdit->setFocus();
    }
}

void SysWindow::enableMyself(bool enable)
{
    if (enable) {
        giflabel->hide();
        loading->stop();
        setDisabled(false);
    } else {
        setDisabled(true);
        giflabel->show();
        loading->start();
        //timerGif.start(3000);
        QTimer::singleShot(3000, this, SLOT(slot_process_start()));
    }
}

void SysWindow::slot_process_start()
{
    //qDebug("process started.");
    loading->stop();
    giflabel->hide();
    setDisabled(false);
}

void SysWindow::slot_process_finish(int s)
{
    enableMyself(true);
    if (s == QProcess::CrashExit) {
        qDebug("\nThe process crashed!!");
    }
}

/*void SysWindow::slot_process_change(QProcess::ProcessState a)
{
    switch(a)
    {
        case QProcess::Starting:
            //qDebug("process_change: Starting");
            break;
        case QProcess::Running:
            //qDebug("process_change: Running");
            break;
        case QProcess::NotRunning:
            //qDebug("process_change: NotRunning");
            break;
        default:
            break;
    }
}*/

void SysWindow::on_pushButton_setting_clicked()
{
//    Os__set_key_ensure(KEY_SYS_SET);
    enableMyself(false);

    for(int i=0; i<sysArgv->size(); i++) {
        if(sysArgv->value(i) == "dbg=1") {
            //重定向信息输出
            myProcess->setStandardOutputFile("/dev/console");
            myProcess->setStandardErrorFile("/dev/console");
        }
    }

    if (myProcess->state() == QProcess::NotRunning) {
        myProcess->start("/app/00001/00001", NULL);
    }
}

void SysWindow::on_pushButton_test_clicked()
{
    //Os__set_key_ensure(KEY_SYS_TEST);
    enableMyself(false);

    for(int i=0; i<sysArgv->size(); i++) {
        if(sysArgv->value(i) == "dbg=1") {
            //重定向信息输出
            myProcess->setStandardOutputFile("/dev/console");
            myProcess->setStandardErrorFile("/dev/console");
        }
    }

    if (myProcess->state() == QProcess::NotRunning) {
        myProcess->start("/app/00002/00002", NULL);
    }
}

void SysWindow::on_pushButton_download_clicked()
{
    //Os__set_key_ensure(200);
    smt = new SecondMenu(tr("SMT DOWNLOAD"));
    smt->showMaximized();
}

void SysWindow::on_pushButton_about_clicked()
{        
    //Os__set_key_ensure(200);
    aboutWin->show();
}
