#include <memory>
#include <QMessageBox>
#include <QPixmap>
#include <QFileDialog>
#include "codegenerator.h"
#include "smt_mini.h"
#include "ui_smt_mini.h"

SMT_Mini::SMT_Mini(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SMT_Mini)
{
    ui->setupUi(this);

    QString _com = "  ";
    _com += tools.getPrivateProfileString("PortSettings","port","COM1",NULL);

    readTimer = new QTimer(this);
    writeTimer = new QTimer(this);
    msgPartL=new QLabel;
    msgTty=new QLabel;
    msgLable=new QLabel;
    msgSpace=new QLabel;
    msgPos=new QLabel;
    msgPosType=new QLabel;
    msgDis=new QLabel;
    msgProBar = new QProgressBar;

    appName.clear();
    appID = 0;
    appVersion = 0;
    cpuType = 1;
    binPath.clear();
    dwnFileName.clear();
    dwnFilePath.clear();
    convertOption = 0;

    arm7FilePath.clear();
    arm7AppName.clear();
    arm7AppID = 0;

    arm9FilePath.clear();
    arm9AppName.clear();
    arm9AppID = 0;

    mergeFilePath.clear();
    mergeFileName.clear();
    mergeOption = 0;

    errMsg.clear();
    lock = false;


    readTimer->setInterval(10);
    PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    serialPort = new QextSerialPort("com1", settings, QextSerialPort::Polling);

    //为plnEditSerialPort安装过滤器
    ui->plnEditSerialPort->installEventFilter(this);

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setDisabled(true);

    msgTty->setMinimumSize(msgLable->sizeHint());
    QPixmap icon(":resource/ico/conico.png");
    msgTty->setPixmap(icon);
    msgPos->setMinimumSize(msgLable->sizeHint());
    QPixmap type(":resource/ico/pos.png");
    msgPos->setPixmap(type);

    msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
    msgLable->setMinimumWidth(150);
    msgLable->setText(_com);

    msgPosType->setMinimumWidth(50);
    msgPosType->setText("");

    msgDis->setText("");
    msgDis->setAlignment(Qt::AlignRight);

    msgSpace->setMinimumWidth(10);

    msgProBar->setStyleSheet("QProgressBar:horizontal {border: 1px solid gray;border-radius: 3px;background: white;padding: 1px;text-align:center;}QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #a3cf62);}");
    msgProBar->setMaximumSize(200,15);
    msgProBar->setMinimumSize(200,15);
    msgProBar->setValue(0);

    statusBar()->addWidget(msgPartL);
    statusBar()->addWidget(msgTty);
    statusBar()->addWidget(msgLable);
    //statusBar()->addWidget(msgSpace);
    //statusBar()->addWidget(msgPos);
    //statusBar()->addWidget(msgPosType);
    statusBar()->addWidget(msgDis,1);
    statusBar()->addWidget(msgProBar);


    if(_com.contains("COM1"))
        on_actionCOM1_triggered();
    else if(_com.contains("COM2"))
        on_actionCOM2_triggered();
    else if(_com.contains("COM3"))
        on_actionCOM3_triggered();
    else if(_com.contains("COM4"))
        on_actionCOM4_triggered();
    else
        on_actionCOM5_triggered();

    _com += QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION+_com);

    connect(readTimer, SIGNAL(timeout()), SLOT(onReadyRead()));
    connect(serialPort, SIGNAL(readyRead()), SLOT(onReadyRead()));
    connect(writeTimer,SIGNAL(timeout()), SLOT(insertHtmlToWidget()));

    QStringList binFileList;
    tools.getBinFileInfoList(&binFileList);
    ui->comboBoxAppName->addItems(binFileList);

    tools.getSerialPort(serialPort);
    tools.getProgressBar(msgProBar);
    tools.getStatusLabel(msgDis);
    tools.getAppInfo(&appName,&appID,&appVersion,&cpuType);
    tools.getErrMsg(&errMsg);
}

SMT_Mini::~SMT_Mini()
{
    delete ui;
}


static QString portReadBuf;
void SMT_Mini::onReadyRead(void)
{
    QRegExp exp("\033\\[[0-9];[0-9]{0,2}m|\033\\[0m|\033\\[[J]{0,1}");
    int readCount = 0;
    unsigned char readBuf[1024];
    memset(readBuf,0x00,sizeof(readBuf));

    if (serialPort->bytesAvailable())
    {
        readCount = serialPort->read((char *)readBuf,sizeof(readBuf));
    }


    if(readCount > 0)
    {
        if(strchr((char *)readBuf,'\x02') != NULL)
        {
            if(strstr((char *)readBuf,ARM9_CMD) != NULL)
            {
                //msgLable->setText("ARM9");
            }
            return;
        }
        portReadBuf.append((char*)readBuf);
        if(portReadBuf.contains(exp) && !writeTimer->isActive())
            writeTimer->start(300);
    }
    else
        return;
    if(!portReadBuf.isEmpty() && !writeTimer->isActive())
    {
        ui->plnEditSerialPort->insertPlainText(portReadBuf);
        portReadBuf.clear();
        ui->plnEditSerialPort->moveCursor(QTextCursor::End);
    }
}

void SMT_Mini::insertHtmlToWidget()
{
    auto_ptr<ansifilter::CodeGenerator> generator(ansifilter::CodeGenerator::getInstance(ansifilter::HTML));
    generator->setEncoding("ISO-8859-1");
    generator->setFragmentCode(false);
    generator->setFont("Lucida Console");
    generator->setPreformatting ( ansifilter::WRAP_SIMPLE, 0);
    generator->setFontSize("10pt");
    ui->plnEditSerialPort->appendHtml(QString(generator->generateString(portReadBuf.toStdString()).c_str()));
    writeTimer->stop();
    portReadBuf.clear();
    ui->plnEditSerialPort->moveCursor(QTextCursor::End);
}

void SMT_Mini::on_actionConnect_triggered()
{
    QString port;
    ui->plnEditSerialPort->clear();
    port = tools.getPrivateProfileString("PortSettings","port","COM1",NULL);
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
        ui->menuComPortSet->setDisabled(true);
        msgLable->setStyleSheet("QLabel { background-color : #a3cf62;}");
        port = "-" + port + QString::fromLocal8Bit("-已连接");
        this->setWindowTitle(SMT_VERSION + port);
    }
}

void SMT_Mini::on_actionWipe_triggered()
{
    ui->plnEditSerialPort->clear();
}

void SMT_Mini::on_actionDisconnect_triggered()
{
    QString strBuf;
    if(serialPort->isOpen())
        serialPort->close();
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setDisabled(true);
    ui->menuComPortSet->setEnabled(true);
    msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
    strBuf = "-" + tools.getPrivateProfileString("PortSettings","port","COM1",NULL) + QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + strBuf);
    lock = false;
}

void SMT_Mini::updateAppNameList()
{
        QStringList strList;
        int index = -1;
        BIN_FILE_INFO binFileInfo;

        binFileInfo.appName = QString::fromLocal8Bit(appName.toLocal8Bit());
        binFileInfo.appID = QString::number(appID);
        binFileInfo.appVersion = QString::number(appVersion);
        binFileInfo.cpuType = cpuType;

        index = tools.setBinFileInfo(&binFileInfo);

        tools.getBinFileInfoList(&strList);
        ui->comboBoxAppName->clear();
        ui->comboBoxAppName->addItems(strList);
        ui->comboBoxAppName->setCurrentIndex(index+1);
}

void SMT_Mini::on_actionSync_triggered()
{
    QMessageBox msgBox;
    QString strTmp;

    if(lock)
        return;
    lock = true;

    //获取app信息
    setAppInfo();

    msgBox.setWindowTitle(QString::fromLocal8Bit("警告"));
    msgBox.setIconPixmap(QPixmap(":resource/ico/warning.png"));

    msgDis->setText("");
    errMsg.clear();


    if(ui->tabMain->currentIndex() == 1)
    {

        if(binPath.isEmpty())
        {
            errMsg = QString::fromLocal8Bit("请选择要压缩的源文件！");
            ui->pushButtonBinFileSelect->setFocus();
            goto _exit;
        }

        if(appName.isEmpty() || appID <= 0)
        {
            errMsg = QString::fromLocal8Bit("应用名称或应用编号错误,请重新输入！");
            ui->lineEditAppID->setFocus();
            goto _exit;
        }


        //查看生成的dwn文件保存目录是否被设置,未设置则使用默认路径
        if(dwnFilePath.isEmpty())
        {
            dwnFilePath = QFileInfo(DWN_SAVE_PATH).absoluteFilePath();
            ui->lineEditDwnFilePath->setText(dwnFilePath);
        }

        updateAppNameList();

        //只传输
        if(convertOption == 1)
        {
            if(!serialPort->isOpen())
            {
                errMsg = QString::fromLocal8Bit("串口未打开,请先打开串口！");
                goto _exit;
            }

            dwnFileName = DWN_SAVE_NAME;
            ui->lineEditDwnAppName->setText(dwnFileName);
            strTmp = dwnFilePath+"/"+dwnFileName;
            if(tools.convertFileToDwn(binPath,strTmp))
            {
                if(tools.transmitFileToPos(strTmp))
                {
                    msgDis->setText(QString::fromLocal8Bit("成功！"));
                    errMsg = QString::fromLocal8Bit("操作成功！");
                }
                else
                    msgDis->setText(QString::fromLocal8Bit("失败！"));
                QFile(strTmp).remove();
            }
            else
                msgDis->setText(QString::fromLocal8Bit("失败！"));
        }
        else
        {
            //查看生成的dwn文件名称是否填写,未填写则使用默认名称
            if(dwnFileName.isEmpty())
            {
                dwnFileName = appName + "_" + QString::number(appID) + "_";
                if(cpuType == ARM7_CPU_TYPE)
                    dwnFileName += "ARM7.dwn";
                else
                    dwnFileName += "ARM9.dwn";
                ui->lineEditDwnFileName->setText(dwnFileName);
            }
            else if(!dwnFileName.contains(".dwn"))
            {
                dwnFileName += ".dwn";
                ui->lineEditDwnFileName->setText(dwnFileName);
            }

            //只压缩
            if(convertOption == 0)
            {
                strTmp = dwnFilePath+"/"+dwnFileName;
                if(tools.convertFileToDwn(binPath,strTmp))
                {
                    msgDis->setText(QString::fromLocal8Bit("成功！"));
                    errMsg = QString::fromLocal8Bit("操作成功！");
                }
                else
                    msgDis->setText(QString::fromLocal8Bit("失败！"));
            }
            //先压缩再传输
            else if(convertOption == 2)
            {
                if(!serialPort->isOpen())
                {
                    errMsg = "串口未打开,请先打开串口！";
                    goto _exit;
                }
                strTmp = dwnFilePath+"/"+dwnFileName;
                if(tools.convertFileToDwn(binPath,strTmp))
                {
                    if(tools.transmitFileToPos(strTmp))
                    {
                        msgDis->setText(QString::fromLocal8Bit("成功！"));
                        errMsg = QString::fromLocal8Bit("操作成功！");
                    }
                    else
                        msgDis->setText(QString::fromLocal8Bit("失败！"));
                }
                else
                    msgDis->setText(QString::fromLocal8Bit("失败！"));
            }
        }
    }
    else if(ui->tabMain->currentIndex() == 2)
    {
        if(arm7FilePath.isEmpty())
            errMsg = QString::fromLocal8Bit("ARM7文件未正确选择,请正确选择文件！");
        else if(arm9FilePath.isEmpty())
            errMsg = QString::fromLocal8Bit("ARM9文件未正确选择,请正确选择文件！");
        else if(arm7AppID <= 0 || arm9AppID <= 0)
            errMsg = QString::fromLocal8Bit("应用编号不正确,请正确选择文件！");
        else if(arm7AppID != arm9AppID)
            errMsg = QString::fromLocal8Bit("应用编号不同,请正确选择文件！");
        else if(arm7FilePath.compare(arm9FilePath) == 0)
            errMsg = QString::fromLocal8Bit("ARM7文件与ARM9文件为同一文件,请正确选择文件！");
        else
        {
            //查看合并后的dwn文件保存目录是否被设置,未设置则使用默认路径
            if(mergeFilePath.isEmpty())
            {
                mergeFilePath = QFileInfo(DWN_SAVE_PATH).absoluteFilePath();
                ui->lineEditMergeFilePath->setText(mergeFilePath);
            }

            //如果合并后的文件名称未设置,使用默认文件名称
            if(mergeFileName.isEmpty())
            {
                mergeFileName = arm7AppName + "_" + QString::number(arm7AppID) + "_Merge.dwn";
                ui->lineEditMergeFileName->setText(mergeFileName);
            }
            else if(!mergeFileName.contains(".dwn"))
            {
                mergeFileName += ".dwn";
                ui->lineEditMergeFileName->setText(mergeFileName);
            }

            strTmp = mergeFilePath+"/"+mergeFileName;
            if(mergeOption == 0)
            {

                if(tools.mergeDnwFile(arm7FilePath,arm9FilePath,strTmp) > 0)
                {
                    msgDis->setText(QString::fromLocal8Bit("成功！"));
                    errMsg = QString::fromLocal8Bit("文件合并成功！");
                }
                else
                    msgDis->setText(QString::fromLocal8Bit("失败！"));
            }
            else if(mergeOption == 1)
            {
                if(!serialPort->isOpen())
                    errMsg = QString::fromLocal8Bit("串口未打开,请先打开串口！");
                else if(tools.mergeDnwFile(arm7FilePath,arm9FilePath,strTmp) > 0)
                {
                    if(tools.transmitFileToPos(strTmp))
                    {
                        msgDis->setText(QString::fromLocal8Bit("成功！"));
                        errMsg = QString::fromLocal8Bit("文件合并成功！");
                    }
                    else
                        msgDis->setText(QString::fromLocal8Bit("失败！"));
                }
                else
                    msgDis->setText(QString::fromLocal8Bit("失败！"));
            }
        }
    }
    else if(ui->tabMain->currentIndex() == 3)
    {
        if(dwnTransmitPath.isEmpty())
            errMsg = QString::fromLocal8Bit("dwn文件未正确选择,请正确选择文件！");
        else if(ui->lineEditDwnAppID->text().toInt() <= 0)
            errMsg = QString::fromLocal8Bit("dwn文件中应用编号不正确,请正确选择文件！");
        else if(!serialPort->isOpen())
            errMsg = QString::fromLocal8Bit("串口未打开,请先打开串口！");
        else if(tools.transmitFileToPos(dwnTransmitPath))
        {
            msgDis->setText(QString::fromLocal8Bit("成功！"));
            errMsg = QString::fromLocal8Bit("文件下载成功！");
        }
        else
            msgDis->setText(QString::fromLocal8Bit("失败！"));
    }
_exit:
    msgBox.setText(errMsg);
    msgBox.exec();
    lock = false;
}

//过滤器实现函数
static bool capsLockFlag = false;
static QString editAppIDBuf;
static QString editAppVerBuf;
bool SMT_Mini::eventFilter(QObject *watched, QEvent *event)
{

    if(watched == ui->plnEditSerialPort)//首先判断控件
    {
        if(event->type()==QEvent::KeyPress)     //然后再判断控件的具体事件 (这里指获得键盘按键事件)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();
            int modifitKey = keyEvent->modifiers();

            if(modifitKey == Qt::ShiftModifier && key >= Qt::Key_A && key <= Qt::Key_Z)
            {
                if(capsLockFlag)
                    key += 32;
                strCmd += QString(key);
                insertCmdChar(key);
            }
            else if(modifitKey == Qt::NoModifier && key >= Qt::Key_A && key <= Qt::Key_Z)
            {
                if(!capsLockFlag)
                {
                    key += 32;
                }
                strCmd += QString(key);
                insertCmdChar(key);
            }
            else if(modifitKey == Qt::NoModifier && key == Qt::Key_CapsLock)
            {
                if(capsLockFlag)
                    capsLockFlag = false;
                else
                    capsLockFlag = true;
            }
            else if(modifitKey == Qt::NoModifier && (key == Qt::Key_Enter || key == Qt::Key_Return))
            {
                if(!serialPort->isOpen())
                {
                    strCmd.clear();
                    return true;
                }
                if(!strCmd.isEmpty())
                {
                    QTextCursor textCursor = ui->plnEditSerialPort->textCursor();
                    for(int i=0;i<strCmd.count();i++)
                    {
                        textCursor.deletePreviousChar();
                    }
                    ui->plnEditSerialPort->setTextCursor(textCursor);
                }
                //发送信息
                writeCommand(key);
            }
            else if(key == Qt::Key_Tab)
            {
                //if(serialPort->isOpen())
                    //writeCommand(key);
            }
            else if(key == Qt::Key_Backspace)
            {
                if(!strCmd.isEmpty() && serialPort->isOpen())
                {
                    QTextCursor textCursor = ui->plnEditSerialPort->textCursor();
                    textCursor.deletePreviousChar();
                    ui->plnEditSerialPort->setTextCursor(textCursor);
                    strCmd = strCmd.leftRef(strCmd.count()-1).toString();
                }
            }
            else if((key >= Qt::Key_Space && key <= Qt::Key_At) ||
                    (key >= Qt::Key_BracketLeft && key <= Qt::Key_QuoteLeft) ||
                    (key >= Qt::Key_BraceLeft && key <= Qt::Key_AsciiTilde))
            {
                strCmd += QString(key);
                insertCmdChar(key);
            }
            else if(key == Qt::Key_Escape || key == Qt::Key_Up || key == Qt::Key_Down)
            {
                writeCommand(key);
            }
            else if(modifitKey == Qt::ControlModifier && (key == Qt::Key_C))
            {
                writeCommand(key);
            }
            return true;
        }
        else if(watched == ui->lineEditAppID)
        {

            if(event->type()==QEvent::FocusIn)     //然后再判断控件的具体事件 (这里指获得焦点事件)
            {
                editAppIDBuf = ui->lineEditAppID->text();
            }
            else if(event->type()==QEvent::FocusOut)//控件的失去焦点事件
            {
                editAppIDBuf.clear();
            }
        }
        else if(watched == ui->lineEditVersion)
        {
            if(event->type() == QEvent::FocusIn)
            {
                editAppVerBuf = ui->lineEditVersion->text();
            }
            else if(event->type() == QEvent::FocusOut)
            {
                editAppVerBuf.clear();
            }
        }
    }
    return QWidget::eventFilter(watched,event);     //最后将事件交给上层对话框
}

void SMT_Mini::on_lineEditAppID_textEdited(const QString &arg1)
{
    int strCount = arg1.count(),i;
    for(i=0;i<strCount;i++)
    {
        //对输入的字符进行过滤，只允许输入数字
        if(arg1.mid(i,1).contains(QRegExp("[0-9]")))
            continue;
        else
            break;
    }

    //如果输入含有非数字或是ID大于65535，不允许输入
    if(i < strCount || ui->lineEditAppID->text().toInt() > 65535)
        ui->lineEditAppID->setText(editAppIDBuf);
    editAppIDBuf = ui->lineEditAppID->text();
}

void SMT_Mini::on_lineEditVersion_textEdited(const QString &arg1)
{
    int strCount = arg1.count(),i;
    for(i=0;i<strCount;i++)
    {
        //对输入的字符进行过滤，只允许输入数字
        if(arg1.mid(i,1).contains(QRegExp("[0-9]")))
            continue;
        else
            break;
    }

    //如果输入含有非数字，不允许输入
    if(i < strCount)
        ui->lineEditVersion->setText(editAppVerBuf);
    editAppVerBuf = ui->lineEditVersion->text();
}

void SMT_Mini::writeCommand(int key)
{
    int i, cmdCount = strCmdList.count();
    if(!serialPort->isOpen())
        return;
    //如果程序锁为true，不允许发送
    if(lock)
        return;
    if(key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        if(!strCmd.isEmpty())
        {
            //将命令插入到命令列表中
            for(i=0;i<cmdCount;i++)
            {
                if(strCmdList.at(i).compare(strCmd) == 0)
                {
                    if(i != cmdCount-1)
                    {
                        strCmdList.removeAt(i);
                        strCmdList<<strCmd;
                    }
                    break;
                }
            }
            if(i == cmdCount)
                strCmdList<<strCmd;
        }
        //写命令到Port端口
        strCmd += "\n";
        serialPort->write(strCmd.toLatin1());
        strCmd.clear();
    }
    else if(key == Qt::Key_Tab)
    {
        serialPort->write(strCmd.toLatin1());
        serialPort->write("\t");
        //补全CMD
    }
    else if(key == Qt::Key_Up)
    {
        if(cmdCount == 0)
            return;

        QTextCursor textCursor = ui->plnEditSerialPort->textCursor();
        for(i=0;i<strCmd.count();i++)
        {
            textCursor.deletePreviousChar();
        }
        ui->plnEditSerialPort->setTextCursor(textCursor);


        if(!strCmd.isEmpty())
        {
            for(i=0;i<cmdCount;i++)
            {
                //命令已经在命令列表中
                if(strCmdList.at(i).compare(strCmd) == 0)
                {
                    if(i > 0)
                        strCmd = strCmdList.at(i-1);
                    break;
                }
            }
            if(i == cmdCount)
                strCmd = strCmdList.at(cmdCount-1);
        }
        else
        {
            strCmd = strCmdList.at(cmdCount-1);
        }

        ui->plnEditSerialPort->moveCursor(QTextCursor::End);
        ui->plnEditSerialPort->insertPlainText(strCmd);
        return;
    }
    else if(key == Qt::Key_Down)
    {
        if(cmdCount == 0)
            return;

        QTextCursor textCursor = ui->plnEditSerialPort->textCursor();
        for(i=0;i<strCmd.count();i++)
        {
            textCursor.deletePreviousChar();
        }
        ui->plnEditSerialPort->setTextCursor(textCursor);


        if(!strCmd.isEmpty())
        {
            for(i=0;i<cmdCount;i++)
            {
                //命令已经在命令列表中
                if(strCmdList.at(i).compare(strCmd) == 0)
                {
                    if(i < cmdCount-1)
                        strCmd = strCmdList.at(i+1);
                    break;
                }
            }
            if(i == cmdCount)
                strCmd = strCmdList.at(cmdCount-1);
        }
        else
        {
            strCmd = strCmdList.at(cmdCount-1);
        }

        ui->plnEditSerialPort->moveCursor(QTextCursor::End);
        ui->plnEditSerialPort->insertPlainText(strCmd);
        return;
    }
    else if(key == Qt::Key_Escape)
    {
        serialPort->write(QString(key).toLatin1());
    }
    else if(key == Qt::Key_C)
    {
        serialPort->write("\x03");
    }
}

void SMT_Mini::insertCmdChar(int key)
{
    ui->plnEditSerialPort->moveCursor(QTextCursor::End);
    ui->plnEditSerialPort->insertPlainText(QString(key));
}

/*
void SMT_Mini::resizeEvent(QResizeEvent *e)
{
    int   width   =   this->width();
    int   height   =   this->height();

    ui->tabMain->setGeometry(0,0,width,height-70);
    ui->plnEditSerialPort->setGeometry(0,0,width,height-95);


    QMainWindow::resizeEvent(e);
}*/


void SMT_Mini::clearAppInfo(int type)
{
    if(type == 0)
    {
        appName.clear();
        appID = 0;
        appVersion = 0;
        cpuType = 1;
        binPath.clear();
        ui->comboBoxCPUType->setCurrentIndex(0);
        ui->lineEditAppID->clear();
        ui->comboBoxAppName->clearEditText();
        ui->lineEditVersion->clear();
        ui->lineEditBinFilePath->clear();
    }
    else if(type == 1)
    {
        arm7FilePath.clear();
        arm7AppName.clear();
        arm7AppID = 0;
        ui->lineEditArm7FilePath->clear();
        ui->lineEditArm7ID->clear();
        ui->lineEditArm7Name->clear();
    }
    else if(type == 2)
    {
        arm9FilePath.clear();
        arm9AppName.clear();
        arm9AppID = 0;
        ui->lineEditArm9FilePath->clear();
        ui->lineEditArm9ID->clear();
        ui->lineEditArm9Name->clear();
    }
    else if(type == 3)
    {
        dwnTransmitPath.clear();
        ui->lineEditDwnAppID->clear();
        ui->lineEditDwnAppName->clear();
        ui->lineEditDwnAppType->clear();
        ui->lineEditDwnAppVersion->clear();
    }
}

void SMT_Mini::on_actionVersion_triggered()
{
    QMessageBox msgBox;
    QString msg;

    msgBox.setWindowTitle("Version");
    msgBox.setIconPixmap(QPixmap(":resource/ico/logo.png"));
    msg = "<html><B>SMT_Mini</B>: SAND Management Tool Mini<br><B>Version</B>: ";
    msg += SMT_VERSION;
    msg += "<br>Copyright 2012 Shanghai SAND.All rights reserved.";
    msg += "<br><a href=http://www.sand.com.cn>www.sand.com.cn</a></html>";
    msgBox.setText(msg);
    msgBox.exec();
}

void SMT_Mini::on_actionExit_triggered()
{
    SMT_Mini::close();
}


void SMT_Mini::on_pushButtonArm7FileSelect_clicked()
{

    DWN_FILE_INFO curFileInfo;
    QString strTmp;
    QMessageBox msgBox;

    tools.clearDwnFileInfo(&curFileInfo);
    strTmp = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开文件"),
                                                    ".",
                                                    QString::fromLocal8Bit("DWN文件 (*.dwn);;所有文件 (*.*)"));
    if(strTmp !="")
    {
        arm7FilePath = strTmp;
        ui->lineEditArm7FilePath->setText(arm7FilePath);
        if(tools.getFileInfo(&curFileInfo,arm7FilePath))
        {
            if(curFileInfo.curMergeType == 1)
                errMsg = QString::fromLocal8Bit("此文件已经合并过,请勿重复合并文件！");
            else if(curFileInfo.curCpuType != ARM7_CPU_TYPE)
                errMsg = QString::fromLocal8Bit("此文件为ARM9类型文件,请正确选择！");
            else
            {
                arm7AppName = curFileInfo.curAppName;
                arm7AppID = curFileInfo.curAppID;
                ui->lineEditArm7ID->setText(QString::number(arm7AppID));
                ui->lineEditArm7Name->setText(arm7AppName);
                return;
            }
        }
        else
        {
            //提示文件不可用
            //errMsg = QString::fromLocal8Bit("文件格式错误,请正确选择文件！");
        }
        clearAppInfo(1);
        ui->lineEditArm7FilePath->setText(strTmp);
        msgBox.setWindowTitle(QString::fromLocal8Bit("警告"));
        msgBox.setIconPixmap(QPixmap(":resource/ico/warning.png"));
        msgBox.setText(errMsg);
        msgBox.exec();
    }
}

void SMT_Mini::on_pushButtonArm9FileSelect_clicked()
{
    DWN_FILE_INFO curFileInfo;
    QString strTmp;
    QMessageBox msgBox;

    tools.clearDwnFileInfo(&curFileInfo);
    strTmp = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开文件"),
                                                    ".",
                                                    QString::fromLocal8Bit("DWN文件 (*.dwn);;所有文件 (*.*)"));
    if(strTmp !="")
    {
        arm9FilePath = strTmp;
        ui->lineEditArm9FilePath->setText(arm9FilePath);
        if(tools.getFileInfo(&curFileInfo,arm9FilePath))
        {
            if(curFileInfo.curMergeType == 1)
                errMsg = QString::fromLocal8Bit("此文件已经合并过,请勿重复合并文件！");
            else if(curFileInfo.curCpuType != ARM9_CPU_TYPE)
                errMsg = QString::fromLocal8Bit("此文件为ARM7类型文件,请正确选择！");
            else
            {
                arm9AppName = curFileInfo.curAppName;
                arm9AppID = curFileInfo.curAppID;
                ui->lineEditArm9ID->setText(QString::number(arm9AppID));
                ui->lineEditArm9Name->setText(arm9AppName);
                return;
            }
        }
        else
        {
            //提示文件不可用
            //errMsg = QString::fromLocal8Bit("文件格式错误,请正确选择文件！");
        }
        clearAppInfo(2);
        ui->lineEditArm9FilePath->setText(strTmp);
        msgBox.setWindowTitle(QString::fromLocal8Bit("警告"));
        msgBox.setIconPixmap(QPixmap(":resource/ico/warning.png"));
        msgBox.setText(errMsg);
        msgBox.exec();
    }
}

void SMT_Mini::on_pushButtonMergeFileSavePath_clicked()
{
    QString strTmp;
    QMessageBox msgBox;
    strTmp = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("打开目录"),
                                                    ".",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(strTmp !="")
    {
        ui->lineEditMergeFilePath->setText(strTmp);
    }
}

void SMT_Mini::on_pushButtonDwnFileSavePath_clicked()
{
    QString strTmp;
    strTmp = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("打开目录"),
                                                    ".",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(strTmp !="")
    {
        dwnFilePath = strTmp;
        ui->lineEditDwnFilePath->setText(dwnFilePath);
    }
}

void SMT_Mini::on_pushButtonBinFileSelect_clicked()
{
    QString strTmp;
    strTmp = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开文件"),
                                                    ".",
                                                    QString::fromLocal8Bit("bin文件 (*.bin);;所有文件 (*.*)"));
    if(strTmp !="")
    {
        binPath = strTmp;
        ui->lineEditBinFilePath->setText(binPath);
    }
}

void SMT_Mini::on_radioButtonTransmit_clicked()
{
    if(ui->radioButtonTransmit->isChecked())
    {
        ui->lineEditDwnFileName->setDisabled(true);
        ui->pushButtonDwnFileSavePath->setDisabled(true);
        convertOption = 1;
    }
}

void SMT_Mini::on_radioButtonConvert_clicked()
{
    if(ui->radioButtonConvert->isChecked())
    {
        ui->lineEditDwnFileName->setEnabled(true);
        ui->pushButtonDwnFileSavePath->setEnabled(true);
        convertOption = 0;
    }
}

void SMT_Mini::on_radioButtonConvertAndTransmit_clicked()
{
    if(ui->radioButtonConvertAndTransmit->isChecked())
    {
        ui->lineEditDwnFileName->setEnabled(true);
        ui->pushButtonDwnFileSavePath->setEnabled(true);
        convertOption = 2;
    }
}

void SMT_Mini::on_tabMain_currentChanged(int index)
{
    if(index == 0)
    {
        ui->actionSync->setDisabled(true);
    }
    else
    {
        ui->actionSync->setEnabled(true);
    }
}

void SMT_Mini::on_comboBoxCPUType_currentIndexChanged(int index)
{
    cpuType = index + 1;
}

void SMT_Mini::setAppInfo(void)
{
    appName = ui->comboBoxAppName->currentText();
    appID = ui->lineEditAppID->text().toInt();
    appVersion = ui->lineEditVersion->text().toInt();
    dwnFileName = ui->lineEditDwnFileName->text();
    mergeFileName = ui->lineEditMergeFileName->text();
}

void SMT_Mini::on_radioButtonMerge_clicked()
{
    mergeOption = 0;
}

void SMT_Mini::on_radioButtonMergeTransmit_clicked()
{
    mergeOption = 1;
}

void SMT_Mini::on_actionCOM1_triggered()
{
    QString strBuf;

    resetActionComPortIco();
    ui->actionCOM1->setIcon(QIcon(":resource/ico/select.png"));
    tools.writePrivateProfileString("PortSettings","port","COM1",NULL);

    strBuf = "  COM1";
    msgLable->setText(strBuf);
    strBuf += QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + strBuf);
}

void SMT_Mini::on_actionCOM2_triggered()
{
    QString strBuf;

    resetActionComPortIco();
    ui->actionCOM2->setIcon(QIcon(":resource/ico/select.png"));
    tools.writePrivateProfileString("PortSettings","port","COM2",NULL);

    strBuf = "  COM2";
    msgLable->setText(strBuf);
    strBuf += QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + strBuf);
}

void SMT_Mini::on_actionCOM3_triggered()
{
    QString strBuf;

    resetActionComPortIco();
    ui->actionCOM3->setIcon(QIcon(":resource/ico/select.png"));
    tools.writePrivateProfileString("PortSettings","port","COM3",NULL);

    strBuf = "  COM3";
    msgLable->setText(strBuf);
    strBuf += QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + strBuf);
}

void SMT_Mini::on_actionCOM4_triggered()
{
    QString strBuf;

    resetActionComPortIco();
    ui->actionCOM4->setIcon(QIcon(":resource/ico/select.png"));
    tools.writePrivateProfileString("PortSettings","port","COM4",NULL);

    strBuf = "  COM4";
    msgLable->setText(strBuf);
    strBuf += QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + strBuf);
}

void SMT_Mini::on_actionCOM5_triggered()
{
    QString strBuf;

    resetActionComPortIco();
    ui->actionCOM5->setIcon(QIcon(":resource/ico/select.png"));
    tools.writePrivateProfileString("PortSettings","port","COM5",NULL);

    strBuf = "  COM5";
    msgLable->setText(strBuf);
    strBuf += QString::fromLocal8Bit("-未连接");
    this->setWindowTitle(SMT_VERSION + strBuf);
}


void SMT_Mini::resetActionComPortIco(void)
{
    ui->actionCOM1->setIcon(QIcon(""));
    ui->actionCOM2->setIcon(QIcon(""));
    ui->actionCOM3->setIcon(QIcon(""));
    ui->actionCOM4->setIcon(QIcon(""));
    ui->actionCOM5->setIcon(QIcon(""));
}


void SMT_Mini::on_pushButtonDwnSelectFilePath_clicked()
{
    QString strTmp;
    QMessageBox msgBox;
    DWN_FILE_INFO dwnFileInfo;
    tools.clearDwnFileInfo(&dwnFileInfo);
    strTmp = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开文件"),
                                                    ".",
                                                    QString::fromLocal8Bit("DWN文件 (*.dwn);;所有文件 (*.*)"));
    if(strTmp !="")
    {
        dwnTransmitPath = strTmp;
        ui->lineDwnSelectFilePath->setText(dwnTransmitPath);
        if(tools.getFileInfo(&dwnFileInfo,dwnTransmitPath))
        {
            ui->lineEditDwnAppID->setText(QString::number(dwnFileInfo.curAppID));
            ui->lineEditDwnAppName->setText(dwnFileInfo.curAppName);
            ui->lineEditDwnAppVersion->setText(QString::number(dwnFileInfo.curAppVersion));
            if(dwnFileInfo.curMergeType == 1)
                strTmp = QString::fromLocal8Bit("集成文件");
            else if(dwnFileInfo.curCpuType == ARM7_CPU_TYPE)
                strTmp = QString::fromLocal8Bit("ARM7类型文件");
            else if(dwnFileInfo.curCpuType == ARM9_CPU_TYPE)
                strTmp = QString::fromLocal8Bit("ARM9类型文件");
            ui->lineEditDwnAppType->setText(strTmp);
        }
        else
        {
            clearAppInfo(3);
            ui->lineDwnSelectFilePath->setText(strTmp);
            //提示文件不可用
            msgBox.setWindowTitle(QString::fromLocal8Bit("警告"));
            msgBox.setIconPixmap(QPixmap(":resource/ico/warning.png"));
            msgBox.setText(errMsg);
            msgBox.exec();
        }
    }
}

void SMT_Mini::on_comboBoxAppName_currentIndexChanged(const QString &arg1)
{
    BIN_FILE_INFO binFileInfo;
    binFileInfo.appName = arg1;
    if(!arg1.isEmpty() &&tools.getBinFileInfo(&binFileInfo))
    {
        ui->lineEditAppID->setText(binFileInfo.appID);
        ui->lineEditVersion->setText(binFileInfo.appVersion);
        ui->comboBoxCPUType->setCurrentIndex(binFileInfo.cpuType-1);

        appID = binFileInfo.appID.toInt();
        appVersion = binFileInfo.appVersion.toInt();
        cpuType = binFileInfo.cpuType;
    }
}
