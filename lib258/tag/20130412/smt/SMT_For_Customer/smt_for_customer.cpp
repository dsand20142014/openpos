#include <memory>
#include <QMessageBox>
#include <QPixmap>
#include <QFileDialog>
#include "smt_for_customer.h"
#include "ui_smt_for_customer.h"

SMT_For_Customer::SMT_For_Customer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SMT_For_Customer)
{
    QString _com = "COM1";
    QStringList portListItems;

    readTimer = new QTimer(this);
    msgPartL=new QLabel;
    msgTty=new QLabel;
    msgLable=new QLabel;
    msgSpace=new QLabel;
    msgPos=new QLabel;
    msgPosType=new QLabel;
    msgDis=new QLabel;
    msgProBar = new QProgressBar;

    ui->setupUi(this);

    portListItems<<"COM1"<<"COM2"<<"COM3"<<"COM4"<<"COM5"<<"COM6"<<"COM7"<<"COM8"<<"COM9"<<"COM10"
                <<"COM11"<<"COM12"<<"COM13"<<"COM14"<<"COM15"<<"COM16"<<"COM17"<<"COM18"<<"COM19"<<"COM20"
               <<"COM21"<<"COM22"<<"COM23"<<"COM24"<<"COM25"<<"COM26"<<"COM27"<<"COM28"<<"COM29"<<"COM30";
    ui->comboBoxCom->addItems(portListItems);
    ui->comboBoxCom->setEditText("COM1");

    readTimer->setInterval(100);
    PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    serialPort = new QextSerialPort("COM1", settings, QextSerialPort::Polling);


    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setDisabled(true);
    msgTty->setMinimumSize(msgLable->sizeHint());
    QPixmap icon(":resource/ico/conico.png");
    msgTty->setPixmap(icon);
    msgPos->setMinimumSize(msgLable->sizeHint());
    QPixmap type(":resource/ico/pos.png");
    msgPos->setPixmap(type);

    msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
    msgLable->setMinimumWidth(120);
    msgLable->setText("  "+_com);

    msgPosType->setMinimumWidth(50);
    msgPosType->setText("");

    msgDis->setText(" ");
    msgDis->setAlignment(Qt::AlignRight);

    msgSpace->setMinimumWidth(10);

    msgProBar->setStyleSheet("QProgressBar:horizontal {border: 1px solid gray;border-radius: 3px;background: white;padding: 1px;text-align:center;}QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #a3cf62);}");
    msgProBar->setMaximumSize(150,15);
    msgProBar->setMinimumSize(150,15);
    msgProBar->setValue(0);
    _com = "-" + _com + QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + _com);

    statusBar()->addWidget(msgPartL);
    statusBar()->addWidget(msgTty);
    statusBar()->addWidget(msgLable);
    //statusBar()->addWidget(msgSpace);
    //statusBar()->addWidget(msgPos);
    //statusBar()->addWidget(msgPosType);
    statusBar()->addWidget(msgDis,1);
    statusBar()->addWidget(msgProBar);

    connect(readTimer, SIGNAL(timeout()), SLOT(onReadyRead()));
    connect(serialPort, SIGNAL(readyRead()), SLOT(onReadyRead()));

    tools.getSerialPort(serialPort);
    tools.getProgressBar(msgProBar);
    tools.getStatusLabel(msgDis);
    tools.getAppParam(&appID,&appName,&appVersion,&appSize);
    tools.getErrMsg(&errMsg);

    clear();

    lock = false;
}

SMT_For_Customer::~SMT_For_Customer()
{
    delete ui;
}

void SMT_For_Customer::onReadyRead(void)
{
    int readCount = 0;
    unsigned char readBuf[1024];
    memset(readBuf,0x00,sizeof(readBuf));

    if (serialPort->bytesAvailable())
    {
        readCount = serialPort->read((char *)readBuf,sizeof(readBuf));
    }
}


void SMT_For_Customer::on_actionConnect_triggered()
{
    QString port;
    port = ui->comboBoxCom->currentText();
    msgLable->setText("  "+port);
    serialPort->setPortName(port);
    serialPort->setBaudRate((BaudRateType)115200);
    serialPort->setDataBits((DataBitsType)8);
    serialPort->setParity((ParityType)0);
    serialPort->setStopBits((StopBitsType)0);
    if (!serialPort->isOpen())
    {
        serialPort->open(QIODevice::ReadWrite);
    }

    if (serialPort->isOpen())
    {

        if (serialPort->queryMode() == QextSerialPort::Polling)
        {
            readTimer->start();
        }
        ui->actionConnect->setDisabled(true);
        ui->actionDisconnect->setEnabled(true);
        ui->comboBoxCom->setDisabled(true);
        msgLable->setStyleSheet("QLabel { background-color : #a3cf62;}");
        port = "-" + port + QString::fromLocal8Bit("-已连接");
        this->setWindowTitle(SMT_VERSION + port);
    }
}


void SMT_For_Customer::on_actionDisconnect_triggered()
{
    QString strBuf;
    if(serialPort->isOpen())
        serialPort->close();
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setDisabled(true);
    ui->comboBoxCom->setEnabled(true);
    msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
    strBuf = "-" + ui->comboBoxCom->currentText() + QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + strBuf);
    lock = false;
}


void SMT_For_Customer::on_actionSync_triggered()
{
    QMessageBox msgBox;
    if(lock || !serialPort->isOpen() || appID == 0 || appName.isEmpty())
        return;
    lock = true;
    msgDis->setText("");
    msgProBar->setValue(0);
    ui->pushButtonAppPath->setDisabled(true);
    if(tools.sync(appPath))
    {
        msgBox.setWindowTitle(QString::fromLocal8Bit("恭喜"));
        msgBox.setIconPixmap(QPixmap(":resource/ico/success.png"));
        msgBox.setText(QString::fromLocal8Bit("下载成功！"));
        msgBox.exec();
        msgDis->setText(QString::fromLocal8Bit("完成!"));
    }
    else
    {
        msgBox.setWindowTitle(QString::fromLocal8Bit("警告"));
        msgBox.setIconPixmap(QPixmap(":resource/ico/warning.png"));
        msgBox.setText(errMsg);
        msgBox.exec();
        msgDis->setText(QString::fromLocal8Bit("失败!"));
    }
    ui->pushButtonAppPath->setEnabled(true);
    lock = false;
}


void SMT_For_Customer::clear(void)
{
    ui->lineEditAppID->clear();
    ui->lineEditAppName->clear();
    ui->lineEditAppPath->clear();
    ui->lineEditAppSize->clear();
    ui->lineEditAppVersion->clear();
    appID = 0;
    appName.clear();
    appVersion.clear();
    appSize = 0;
    appPath.clear();
}

void SMT_For_Customer::on_actionVersion_triggered()
{
    QMessageBox msgBox;
    QString msg;

    msgBox.setWindowTitle(QString::fromLocal8Bit("版本"));
    msgBox.setIconPixmap(QPixmap(":resource/ico/logo.png"));
    msg = QString::fromLocal8Bit("<html><B>SMT For Customer</B>: 杉德应用下载工具(客户版)<br><B>Version</B>: ");
    msg += SMT_VERSION;
    msg += "<br>Copyright 2012 Shanghai SAND.All rights reserved.";
    msg += "<br><a href=http://www.sand.com.cn>www.sand.com.cn</a></html>";
    msgBox.setText(msg);
    msgBox.exec();
}

void SMT_For_Customer::on_actionExit_triggered()
{
    SMT_For_Customer::close();
}


void SMT_For_Customer::on_pushButtonAppPath_clicked()
{
    QString strTmp;
    QMessageBox msgBox;
    strTmp = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开文件"),
                                                    ".",
                                                    QString::fromLocal8Bit("DWN文件 (*.dwn);;所有文件 (*.*)"));
    if(strTmp !="")
    {
        appPath = strTmp;
        ui->lineEditAppPath->setText(appPath);
        if(tools.getFileInfo(appPath))
        {
            ui->lineEditAppID->setText(QString::number(appID));
            ui->lineEditAppName->setText(appName);
            ui->lineEditAppVersion->setText(appVersion);
            strTmp = QString::number((double)appSize/1024.00,'f',2) + " KB";
            ui->lineEditAppSize->setText(strTmp);
        }
        else
        {
            clear();
            ui->lineEditAppPath->setText(strTmp);
            //提示文件不可用
            msgBox.setWindowTitle(QString::fromLocal8Bit("警告"));
            msgBox.setIconPixmap(QPixmap(":resource/ico/warning.png"));
            msgBox.setText(errMsg);
            msgBox.exec();
        }
    }
}

void SMT_For_Customer::on_comboBoxCom_currentIndexChanged(const QString &arg1)
{
    QString strBuf;
    msgLable->setText("  " + arg1);
    strBuf = "-" + arg1 + QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + strBuf);
}
