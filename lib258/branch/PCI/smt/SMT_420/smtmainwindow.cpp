#include <memory>
#include <QFileInfo>
#include <QIODevice>
#include <QMessageBox>
#include <QPixmap>
#include <QTranslator>
#include <QHostInfo>
#include <QProcess>
#include "smtmainwindow.h"
#include "ui_smtmainwindow.h"


SMTMainWindow::SMTMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SMTMainWindow)
{
    QString strBuf;
    QString _com = tools.getPrivateProfileString("PortSettings","port","COM1",NULL);
    QString baudRate = tools.getPrivateProfileString("PortSettings","baudRate","115200",NULL);

    ui->setupUi(this);

    //初始化操作
    appIndex = -1;
    sysIndex = -1;
    lock = false;
    //默认传输方式为串口;0表示串口,1表示网络
    transType = 0;
    shakeFlag = false;
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
    tcpSocket = new QTcpSocket(this);
    tcpServer = new QTcpServer(this);
    readTimer->setInterval(10);
    PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    serialPort = new QextSerialPort("com1", settings, QextSerialPort::Polling);
    //为plnEditSerialPort安装过滤器
    ui->plnEditSerialPort->installEventFilter(this);
    ui->editAppID->installEventFilter(this);
    ui->tableWidget->installEventFilter(this);

    //功能未完成，不可用
    ui->actionArm_GCC->setDisabled(true);

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setDisabled(true);
    ui->tabMain->setCurrentIndex(0);
    ui->tabResourceView->setCurrentIndex(0);
    ui->tabFileView->setCurrentIndex(0);
    ui->frameSystem->hide();
    ui->tableWidget->hide();
    ui->labelWarning->hide();
    ui->chkBoxUIScript->setEnabled(false);
    ui->actionUpdate->setEnabled(false);
    //设置应用能够多选 未完成
    //ui->listWgtApplication->setSelectionMode(QAbstractItemView::ExtendedSelection);
    strBuf = QString(SMT_VERSION)+" [SERIAL " + _com + "] ";
    strBuf += tr("Disconnect");
    this->setWindowTitle(strBuf);

    msgTty->setMinimumSize(msgLable->sizeHint());
    QPixmap icon(":resource/ico/conico.png");
    msgTty->setPixmap(icon);
    msgPos->setMinimumSize(msgLable->sizeHint());
    QPixmap type(":resource/ico/pos.png");
    msgPos->setPixmap(type);

    msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
    msgLable->setMinimumWidth(150);
    strBuf = "  "+_com+" :"+baudRate + " ";
    strBuf += tr("Disconnect");
    msgLable->setText(strBuf);

    msgPosType->setMinimumWidth(150);
    msgPosType->setText("PS420");

    msgDis->setText(tr("READY TO SHAKE "));
    msgDis->setAlignment(Qt::AlignRight);

    msgSpace->setMinimumWidth(10);

    msgProBar->setStyleSheet("QProgressBar:horizontal {border: 1px solid gray;border-radius: 3px;background: white;padding: 1px;text-align:center;}QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 #a3cf62);}");
    msgProBar->setMaximumSize(200,15);
    msgProBar->setMinimumSize(200,15);
    msgProBar->setValue(0);

    statusBar()->addWidget(msgPartL);
    statusBar()->addWidget(msgTty);
    statusBar()->addWidget(msgLable);
    statusBar()->addWidget(msgSpace);
    statusBar()->addWidget(msgPos);
    statusBar()->addWidget(msgPosType);
    statusBar()->addWidget(msgDis,1);
    statusBar()->addWidget(msgProBar);


    connect(readTimer, SIGNAL(timeout()), SLOT(onReadyRead()));
    //connect(serialPort, SIGNAL(readyRead()), SLOT(onReadyRead()));
    connect(writeTimer,SIGNAL(timeout()), SLOT(insertHtmlToWidget()));

    tools.getProgressBar(msgProBar);
    tools.getStatusLabel(msgDis,msgLable);
    tools.getTimer(readTimer);
    tools.getSocket(tcpSocket);
    tools.getTransType(&transType);
    tools.getItemAppList(&itemAPPInfo,&itemDelList);
    tools.getItemSysList(&itemSYSInfo,&itemSysDelList);
    tools.getListWidget(ui->listWgtApplication,ui->listWgtSystem,ui->listWgtImage);
    tools.getTableWidget(ui->tableWidget);
    tools.getSerialPort(serialPort);
    tools.getDaemonSet(&daemonSet);
    tools.getTreeWidget(ui->treeScheme);
    tools.initTreeWidget(INI_FILE_CONFIG);

    clearAll();

}

SMTMainWindow::~SMTMainWindow()
{

    delete ui;
}

static QString portReadBuf;

void SMTMainWindow::onReadyRead()
{
    QRegExp exp("\033\\[[0-9];[0-9]{0,2}m|\033\\[0m|\033\\[[J]{0,1}");
    int readCount = 0;
    unsigned char readBuf[BUF_MAX_SIZE];
    unsigned char *point = NULL;
    int ret;
    memset(readBuf,0x00,sizeof(readBuf));
    readCount = tools.read((char *)readBuf,sizeof(readBuf));
    if(readCount > 0)
    {
        point = (unsigned char *)strchr((char *)readBuf,SPD_STX);
        if(point != NULL && *(point+1) <= DOWNCMD_RESPKEY && *(point+1) >= DOWNCMD_REQUEST)
        {
            readTimer->stop();
            //先把point之前的数据写入到屏幕
            if(point != readBuf)
            {
                unsigned char tempBuf[BUF_MAX_SIZE];
                int len = point - readBuf;
                memset(tempBuf,0,sizeof(tempBuf));
                memcpy(tempBuf,readBuf,len);

                //ui->plnEditSerialPort->insertPlainText(QString((char *)tempBuf));
                //ui->plnEditSerialPort->moveCursor(QTextCursor::End);

                memset(tempBuf,0,sizeof(tempBuf));
                memcpy(tempBuf,point,readCount-len);
                memset(readBuf,0,sizeof(readBuf));
                memcpy(readBuf,tempBuf,readCount-len);
                readCount -= len;
            }


            ret = tools.checkPakHeader((char*)readBuf, &readCount, 200);
            //qDebug()<<"ret is "<<QString::number(ret);
            if(ret == 0)
            {
                if(!portReadBuf.isEmpty())
                {
                    ui->plnEditSerialPort->insertPlainText(portReadBuf);
                    portReadBuf.clear();
                }
                tools.processRecvStream(readBuf,readCount);
            }
            readTimer->start();
            return;
        }
        portReadBuf.append((char*)readBuf);

        if(portReadBuf.contains(exp) && !writeTimer->isActive())
            writeTimer->start(200);
    }
    else
    {
        return;
    }
    if(!portReadBuf.isEmpty() && !writeTimer->isActive())
    {
        ui->plnEditSerialPort->insertPlainText(portReadBuf);
        portReadBuf.clear();
        ui->plnEditSerialPort->moveCursor(QTextCursor::End);
    }
}

void SMTMainWindow::insertHtmlToWidget()
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


void SMTMainWindow::on_actionConnect_triggered()
{
    QString strBuf;
    ui->plnEditSerialPort->clear();

    if(transType == 0)
    {
        QString port,baudRate;
        port = tools.getPrivateProfileString("PortSettings","port","COM1",NULL);
        baudRate = tools.getPrivateProfileString("PortSettings","baudRate","115200",NULL);
        strBuf = "  "+port+" :"+baudRate + " ";
        if(!tools.openSerialPort())
        {
            return;
        }
        strBuf += tr("Connect");
        msgLable->setText(strBuf);

        strBuf = QString(SMT_VERSION)+" [SERIAL " + port + "] ";
        strBuf += tr("Connect");
        this->setWindowTitle(strBuf);
        ui->actionNetWork_Server->setDisabled(true);
        ui->actionSerial_Port->setDisabled(true);
    }
    else if(transType == 1)
    {
        tcpServer->listen(QHostAddress::Any,tools.getPrivateProfileInt("PortSettings","IPPort",11111,NULL));
        connect(tcpServer,SIGNAL(newConnection()),this,SLOT(creatNewConnect()));
        ui->actionSerial_Port->setDisabled(true);
        ui->actionNetWork_Server->setDisabled(true);
        connect(tcpServer,SIGNAL(destroyed()),this,SLOT(on_actionDisconnect_triggered()));
        if(tcpSocket->state() == QAbstractSocket::ConnectedState)
        {
            strBuf = "  IP Port:"+tools.getPrivateProfileString("PortSettings","IPPort","11111",NULL) + " ";
            strBuf += tr("Connect");
            msgLable->setText(strBuf);

            strBuf = QString(SMT_VERSION)+" [NETWORK] ";
            strBuf += tr("Connect");
            this->setWindowTitle(strBuf);
            msgLable->setStyleSheet("QLabel { background-color : #a3cf62;}");
        }
    }
    else
        return;
    readTimer->start();
    ui->actionConnect->setDisabled(true);
    ui->actionDisconnect->setEnabled(true);

    clearAll();
}

void SMTMainWindow::on_actionShake_triggered()
{
    const int cmdNum = 5;
    unsigned char cmdArray[cmdNum][2] = {{0x35,0x31},{0x32,0x31},{0x33,0x31},{0x34,0x31},{0x35,0x36}};
    int timeOut = 0;
    bool result = true;
    if(lock)
        return;
    lock = true;
    //开始握手后灰掉按钮
    ui->actionShake->setEnabled(false);
    if(!tools.checkTransPort())
        on_actionConnect_triggered();
    if(tools.checkTransPort())
    {
        clearAll();

        for(int i=0;i<cmdNum;i++)
        {
            msgDis->setText(QString::number(i+1)+"/"+QString::number(cmdNum));
            while(timeOut++ < 3)
            {
                if(!tools.checkTransPort())
                {
                    result = false;
                    goto _exit;
                }
                result = tools.getItemInfoFromPos(&cmdArray[i],1);
                if(result)
                    break;
                tools.sleep(500);
            }
            timeOut = 0;
            if(!result)
                break;
        }

_exit:
        if(result)
        {
            msgDis->setText(tr("FINISH!"));
            ui->actionUpdate->setEnabled(true);
            shakeFlag = true;
        }
        else
        {
            QMessageBox msgBox;
            QString msg;
            msgBox.setWindowTitle(tr("Sorry"));
            msgBox.setIconPixmap(QPixmap(":resource/ico/cry.png"));
            msg = tr("Shake Failure!\nPlease try again!");
            msgBox.setText(msg);
            msgDis->setText(tr("ERROR!"));
            msgBox.exec();
            on_actionWip_triggered();
        }
        lock = false;
    }
    //握手完成，无论成功与否恢复按钮
    ui->actionShake->setEnabled(true);
}

void SMTMainWindow::on_actionWip_triggered()
{
    ui->plnEditSerialPort->clear();
}

void SMTMainWindow::on_actionDisconnect_triggered()
{
    QString strBuf;

    if(transType == 0)
    {
        QString port,baudRate;
        port = tools.getPrivateProfileString("PortSettings","port","COM1",NULL);
        baudRate = tools.getPrivateProfileString("PortSettings","baudRate","115200",NULL);
        strBuf = "  "+port+" :"+baudRate + " ";
        strBuf += tr("Disconnect");
        msgLable->setText(strBuf);

        strBuf = QString(SMT_VERSION)+" [SERIAL " + port + "] ";
        strBuf += tr("Disconnect");
        this->setWindowTitle(strBuf);
        tools.closeSerialPort();
    }
    else if(transType == 1)
    {
        strBuf = "  IP Port:"+tools.getPrivateProfileString("PortSettings","IPPort","11111",NULL) + " ";
        strBuf += tr("Disconnect");
        msgLable->setText(strBuf);

        strBuf = QString(SMT_VERSION)+" [NETWORK] ";
        strBuf += tr("Disconnect");
        this->setWindowTitle(strBuf);

        tcpServer->close();
        msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
    }
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setDisabled(true);
    ui->actionNetWork_Server->setEnabled(true);
    ui->actionSerial_Port->setEnabled(true);
    ui->actionUpdate->setDisabled(true);
    ui->actionShake->setEnabled(true);

    //清理所有数据
    clearAll();
    //logfile.close();
    lock = false;
}


//过滤器实现函数
static bool capsLockFlag = false;
static QString editAppIDBuf;
static QString strKeyBuf;
bool SMTMainWindow::eventFilter(QObject *watched, QEvent *event)
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
                if(!tools.checkTransPort())
                {
                    strCmd.clear();
                    return true;
                }
                if(!strCmd.isEmpty())
                {
                    QTextCursor textCursor = ui->plnEditSerialPort->textCursor();
                    if(transType == 0)
                        for(int i=0;i<strCmd.count();i++)
                            textCursor.deletePreviousChar();
                    else if(transType == 1)
                        ui->plnEditSerialPort->appendHtml("\n");
                    ui->plnEditSerialPort->setTextCursor(textCursor);
                }
                //发送信息
                writeCommand(key);
            }
            else if(key == Qt::Key_Tab)
            {
                ;//tab键自动补全命令还有问题，暂不提供Tab键
                //if(serialPort->isOpen())
                //    writeCommand(key);
            }
            else if(key == Qt::Key_Backspace)
            {
                if(!strCmd.isEmpty() && tools.checkTransPort())
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
    }
    else if(watched == ui->editAppID)
    {

        if (event->type()==QEvent::FocusIn)     //然后再判断控件的具体事件 (这里指获得焦点事件)
        {
            editAppIDBuf = ui->editAppID->text();
        }
        else if(event->type()==QEvent::FocusOut)//控件的失去焦点事件
        {
            editAppIDBuf.clear();
        }
    }
    else if(watched == ui->tableWidget)
    {
        if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();
            int modifitKey = keyEvent->modifiers();
            int index = ui->listWgtSystem->currentRow();
            if(modifitKey == Qt::NoModifier && (key == Qt::Key_Enter || key == Qt::Key_Return) &&
                    index != -1 && itemSYSInfo.at(index).type == SHELL &&
                    itemSYSInfo.at(index).srcName.contains("setting.ini"))
            {
                ui->tableWidget->item(0,0)->setText(ui->tableWidget->item(0,0)->text()+"\n*OK*");
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched,event);     //最后将事件交给上层对话框
}

void SMTMainWindow::on_editAppID_textEdited(const QString &arg1)
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
    if(i < strCount || ui->editAppID->text().toInt() > 65535)
        ui->editAppID->setText(editAppIDBuf);
    editAppIDBuf = ui->editAppID->text();
}

void SMTMainWindow::writeCommand(int key)
{
    int i, cmdCount = strCmdList.count();
    if(!tools.checkTransPort())
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
        tools.write(strCmd.toLocal8Bit().data(),strCmd.length());
        strCmd.clear();
    }
    else if(key == Qt::Key_Tab)
    {
        //serialPort->write(strCmd.toLatin1());
        //serialPort->write("\t");
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
        tools.write(QString(key).toLocal8Bit().data(),1);
    }
    else if(key == Qt::Key_C)
    {
        tools.write((char *)"\x03",sizeof("\x03"));
    }
}

void SMTMainWindow::insertCmdChar(int key)
{
    ui->plnEditSerialPort->moveCursor(QTextCursor::End);
    ui->plnEditSerialPort->insertPlainText(QString(key));
}

void SMTMainWindow::resizeEvent(QResizeEvent *e)
{
    int   width   =   this->width();
    int   height   =   this->height();

    appIndex = ui->listWgtApplication->currentRow();
    sysIndex = ui->listWgtSystem->currentRow();
    ui->tabMain->setGeometry(0,0,width,height-70);
    ui->plnEditSerialPort->setGeometry(0,0,width,height-95);

    //这两个宽固定
    int tabTree_width=201;
    int fileview_width;//=635;
    int groupbox_width=342;
    ui->tabActionOfficer->setGeometry(0,0,width,height-70);
    //设置做边tree
   // tabTree=width-fileview_width-groupbox_width;

    ui->tabResourceView->setGeometry(0,0,tabTree_width,height-92-8);
    ui->treeResource->setGeometry(-1,-1,tabTree_width-4,height-92-8 - 20);
    ui->treeScheme->setGeometry(-1,28,tabTree_width-4,height-92-8 - 50);
    ui->tabResource->setGeometry(0,0,tabTree_width,height-92-8);


    //设置中间filelist
    fileview_width=width-tabTree_width-groupbox_width;
    ui->tabSystem->setGeometry(tabTree_width-3,0,fileview_width,height-92-26);
    ui->listWgtSystem->setGeometry(-1,-1,fileview_width,height-92-26);
    ui->listWgtSystem->reset();
    ui->tabFileView->setGeometry(tabTree_width-3,0,fileview_width,height-92-26);
    ui->listWgtApplication->setGeometry(-1,-1,fileview_width,height-92-26);
    ui->listWgtApplication->reset();

    //设置右边属性栏
    ui->groupBox->setGeometry(tabTree_width+fileview_width-1,0,groupbox_width-6,height-92-10);
    ui->frameApplication->setGeometry(-1,50,groupbox_width-6,height-92-65);

    if(sysIndex != -1 && itemSYSInfo.at(sysIndex).type == SHELL)
        ui->tableWidget->setGeometry(0,50,groupbox_width-7,height-92-27 - 35);
    else
        ui->tableWidget->setGeometry(0,15,groupbox_width-7,height-92-27);

    //ui->listWgtImage->setGeometry(1,213,);
    ui->tabAppSetView->setGeometry(1,130,groupbox_width-7,height-92-193);
    ui->listWgtImage->setGeometry(-1,0,groupbox_width-7,height-92-217);
    ui->editAppSettings->setGeometry(-1,0,groupbox_width-7,height-92-217);

    ui->listWgtApplication->setCurrentRow(appIndex);
    ui->listWgtSystem->setCurrentRow(sysIndex);
    if(ui->tabFileView->currentIndex() == 1 && sysIndex >= 0)
        showItemInfo(itemSYSInfo.at(sysIndex).type);

    QMainWindow::resizeEvent(e);
}

/////////////////////////////////////////////////////////

void SMTMainWindow::on_treeResource_itemClicked(QTreeWidgetItem *item, int column)
{
    ui->treeResource->sortingChildFile(item);
}

void SMTMainWindow::on_treeResource_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(item->parent() == NULL)
        return;
    QString filePath = ui->treeResource->getCurItemPath(item);
    QFileInfo fileInfo(filePath);
    if(fileInfo.isDir())
        return;
    if(lock)
        return;
    lock = true;
    QString fileSuffix = fileInfo.suffix();
    QString fileName = fileInfo.fileName();
    CAPPItemInfo appItemTemp;
    CSYSItemInfo otherItemTemp;

    otherItemTemp.srcName = fileInfo.fileName();
    otherItemTemp.srcPath = fileInfo.filePath();
    otherItemTemp.status = 0x30;

    if(fileSuffix.contains("bin"))
    {
        appItemTemp.appDesc = "new app";
        appItemTemp.srcPath = fileInfo.filePath();
        appItemTemp.srcName = fileInfo.fileName();
        appItemTemp.appNumber = 0;
        appItemTemp.appStatus = 0x30;
        appItemTemp.editType = 0;
        tools.addItemToListWidget(&appItemTemp,APPLICATION);
        showItemInfo(APPLICATION);
        goto _exit;
    }
    else if(fileSuffix.contains("qss"))
    {
        if(ui->tabFileView->currentIndex() == 0 && ui->listWgtApplication->currentRow() >= 0 && !ui->editAppID->text().isEmpty())
        {
            if((itemAPPInfo.at(ui->listWgtApplication->currentRow()).appStatus & 0x40) == 0x40)
            {
                msgDis->setText(tr("Please double click APP to edit \"UI Script\"!"));
                goto _exit;
            }
            curUIScriptPath = fileInfo.filePath();
            ui->chkBoxUIScript->setEnabled(true);
            ui->chkBoxUIScript->setCheckState(Qt::Checked);
            ui->labelUIScript->setText(fileInfo.fileName().left(10));
        }
        goto _exit;
    }
    else if(fileSuffix.contains("dta"))
    {
        if(tools.addDtaToListWidget(filePath))
        {
            if(fileName.contains("driver_lib_font"))
            {
                showItemInfo(itemSYSInfo.at(ui->listWgtSystem->currentRow()).type);
            }
            else
            {
                showItemInfo(APPLICATION);
            }
        }
        goto _exit;
    }
    else if(fileSuffix.contains("pln"))
    {
        if(tools.unpackPln(filePath))
        {
            showItemInfo(APPLICATION);
        }
        goto _exit;
    }
    //如果是图片，将图片直接添加到,因为linuxPos不支持显示ico图标，所以暂时屏蔽ico图标
    else if(/*fileSuffix.contains("ico") || */fileSuffix.contains("png") || fileSuffix.contains("bmp") || fileSuffix.contains("jpg"))
    {
        if(ui->tabFileView->currentIndex() == 0 && ui->listWgtApplication->currentRow() >= 0 && !ui->editAppID->text().isEmpty())
        {
            tools.addImageToListWidget(fileInfo.filePath(),&curIcoPath,&curLogoPath,&curDelPicture);
            ui->tabAppSetView->setCurrentIndex(0);
        }
        goto _exit;
    }

    //lib库
    else if(fileName.contains(".so.") || fileSuffix.contains("so"))
    {
       otherItemTemp.type = LIB;
       if(fileName.contains(".so."))
       {
            otherItemTemp.version = fileName.right(fileName.size()-fileName.lastIndexOf(".so.")-4);
            otherItemTemp.srcName = fileName.left(fileName.lastIndexOf(".so.")+3);
       }
    }
    //驱动文件
    else if(fileName.contains(".ko.") || fileSuffix.contains("ko"))
    {
        otherItemTemp.type = DRIVER;
        if(fileName.contains(".ko."))
        {
            otherItemTemp.version = fileName.right(fileName.size()-fileName.lastIndexOf(".ko.")-4);
            otherItemTemp.srcName = fileName.left(fileName.lastIndexOf(".ko.")+3);
        }
    }
    //字体
    else if(fileName.contains(".ft.") || fileSuffix.contains("ft"))
    {
        otherItemTemp.type = FONT;
        if(fileName.contains(".ft."))
        {
            otherItemTemp.version = fileName.right(fileName.size()-fileName.lastIndexOf(".ft.")-4);
            otherItemTemp.srcName = fileName.left(fileName.lastIndexOf(".ft.")+3);
        }
    }
    //Daemon
    else if(fileName.contains(".dm.") || fileSuffix.contains("dm"))
    {
        otherItemTemp.type = DAEMON;
        if(fileName.contains(".dm."))
        {
            otherItemTemp.version = fileName.right(fileName.size()-fileName.lastIndexOf(".dm.")-4);
            otherItemTemp.srcName = fileName.left(fileName.lastIndexOf(".dm.")+3);
        }
    }
    else if(fileSuffix.contains("img") || fileSuffix.contains("cramfs"))
    {
        otherItemTemp.type = CRAMFS;
    }
    else if(fileSuffix.contains("crt"))
    {
        otherItemTemp.type = CRT;
    }
    //李天波boot
    else if(fileSuffix.contains("s19"))
    {
        otherItemTemp.type = OTHER_BOOT;
    }
    //Uboot
    else if(fileName.contains("uboot"))
    {
        otherItemTemp.type = UBOOT;
    }
    else if(fileName.contains("uImage"))
    {
        otherItemTemp.type = KERNEL;
    }
    else
    {
        otherItemTemp.type = DATA;
    }
    tools.addItemToListWidget(&otherItemTemp,otherItemTemp.type);
    showItemInfo(otherItemTemp.type);
_exit:
    lock = false;
    return;
}


void SMTMainWindow::on_listWgtApplication_itemClicked(QListWidgetItem *item)
{
    showItemInfo(item->type()%1000);
}

void SMTMainWindow::on_listWgtSystem_itemClicked(QListWidgetItem *item)
{
    showItemInfo(item->type()%1000);
}

void SMTMainWindow::showItemInfo(int type)
{
    int index = 0;
    clear();
    if(type == APPLICATION)
    {
        ui->frameSystem->hide();
        ui->frameApplication->show();
        ui->tableWidget->hide();
        ui->tabFileView->setCurrentIndex(0);
        ui->tabAppSetView->setCurrentIndex(0);

        index = ui->listWgtApplication->currentRow();
        CAPPItemInfo itemTemp = itemAPPInfo.at(index);
        //如果应用ID变更，不允许修改
        if(itemTemp.appNumber != 0)
            ui->editAppID->setEnabled(false);

        ui->editAppID->setText(QString::number(itemTemp.appNumber,10));
        ui->editAppDes->setText(itemTemp.appDesc);
        ui->editAppSettings->setText(itemTemp.appSettings);
        ui->editAppVersion->setText(itemTemp.version);
        ui->editItemCode->setText(itemTemp.itemCode);
        ui->editAppEditDate->setText(itemTemp.editDate);

        if(itemTemp.hideFlag)
            ui->chkBoxAppHide->setCheckState(Qt::Checked);
        curIcoPath = itemTemp.ICOPicPath;
        curLogoPath = itemTemp.logoPicPath;
        curDelPicture = itemTemp.delPicture;
        curUIScriptPath = itemTemp.uiScriptPath;
        curDelUIScriptName = itemTemp.delUiScriptName;
        tools.showImage(itemTemp.ICOPicPath);

        if(!curUIScriptPath.isEmpty())
        {
            ui->chkBoxUIScript->setEnabled(true);
            ui->chkBoxUIScript->setCheckState(Qt::Checked);
            ui->labelUIScript->setText(QFileInfo(itemTemp.uiScriptPath).fileName().left(10));
        }
        else
        {
            ui->chkBoxUIScript->setEnabled(false);
            ui->chkBoxUIScript->setCheckState(Qt::Unchecked);
        }

        //显示Logo图片
        for(int i=0;i<itemTemp.logoPicPath.count();i++)
        {
            tools.showImage(itemTemp.logoPicPath.at(i));
        }

    }
    else if(type > 0)
    {
        ui->frameApplication->hide();
        ui->frameSystem->hide();
        ui->tableWidget->show();
        ui->tabFileView->setCurrentIndex(1);
        if(type != SHELL)
        {
            ui->btnApply->hide();
            ui->line->hide();
        }
        tools.drawTable(type);
    }
}

//清空各个选项中的内容
void SMTMainWindow::clear(void)
{
    ui->editAppID->setEnabled(true);
    ui->editAppDes->setEnabled(true);
    ui->editAppSettings->setEnabled(true);
    ui->editItemCode->setEnabled(true);
    ui->editAppVersion->setEnabled(true);
    ui->listWgtImage->setEnabled(true);
    ui->treeResource->setEnabled(true);
    ui->btnApply->setEnabled(true);
    ui->labelWarning->hide();
    ui->editSysVersion->setEnabled(false);
    ui->chkBoxAppHide->setEnabled(true);
    ui->chkBoxUIScript->setEnabled(false);
    ui->labelUIScript->clear();
    ui->chkBoxAppHide->setCheckState(Qt::Unchecked);
    ui->chkBoxUIScript->setCheckState(Qt::Unchecked);
    curIcoPath.clear();
    curLogoPath.clear();
    curDelPicture.clear();
    curUIScriptPath.clear();
    curDelUIScriptName.clear();
    ui->editAppID->clear();
    ui->editAppDes->clear();
    ui->editAppSettings->clear();
    ui->editAppVersion->clear();
    ui->editAppEditDate->clear();
    ui->editItemCode->clear();
    ui->listWgtImage->clear();
    ui->editFileName->clear();
    ui->editSysVersion->clear();
    ui->tableWidget->removeRow(0);
    ui->tableWidget->removeColumn(0);
    ui->radioButtonCover->setChecked(true);
    ui->radioButtonUpdate->setChecked(false);
    ui->radioButtonCover->setEnabled(false);
    ui->radioButtonUpdate->setEnabled(false);
    ui->line->show();
    ui->btnApply->show();
}

void SMTMainWindow::clearAll(void)
{
    itemAPPInfo.clear();
    itemSYSInfo.clear();
    itemDelList.clear();
    itemSysDelList.clear();
    daemonSet.clear();
    ui->listWgtApplication->clear();
    ui->listWgtSystem->clear();
    clear();
    QFile zipFile(UPDATE_DATA_NAME);
    if(zipFile.exists())
        zipFile.remove();
    QFile tempFile(UPDATE_DATA_TEMP_NAME);
    if(tempFile.exists())
        tempFile.remove();
    if(QFile("DTA").exists())
        tools.deleteDirectory(QFileInfo("DTA"));
    if(QFile("download").exists())
        tools.deleteDirectory(QFileInfo("download"));
}

void SMTMainWindow::contextMenuEvent(QContextMenuEvent * event)
{
    if(lock)
        return;

    QMenu* popMenu = new QMenu(this);
    int index = -1;
    int framX = QWidget::mapToGlobal(ui->tabMain->pos()).x();
    int framY = QWidget::mapToGlobal(ui->tabMain->pos()).y();
    int x = QCursor::pos().x();
    int y = QCursor::pos().y();

    QListWidgetItem *appListWidgetItem = ui->listWgtApplication->itemAt(x-framX-201,y-framY-80);
    QListWidgetItem *sysListWidgetItem = ui->listWgtSystem->itemAt(x-framX-201,y-framY-80);
    QTreeWidgetItem *srcTreeWidgetItem = ui->treeResource->itemAt(x-framX,y-framY-75);
    QTreeWidgetItem *cfgTreeWidgetItem = ui->treeScheme->itemAt(x-framX,y-framY-106);

    QIcon delIcon = QIcon(":resource/ico/delete.png");
    QIcon refreshIcon = QIcon(":resource/ico/refresh.png");
    QIcon prevIcon = QIcon(":resource/ico/prev.png");
    QIcon nextIcon = QIcon(":resource/ico/next.png");
    QIcon updateIcon = QIcon(":resource/ico/update.png");

    //如果在应用选项卡
    if(appListWidgetItem != NULL && ui->tabFileView->currentIndex() == 0)
    {

        QAction *prevAction;
        QAction *nextAction;
        QAction *separator1;
        QAction *delAction;
        QAction *separator2;
        QAction * releaseAction;

        index = ui->listWgtApplication->currentRow();
        //ui->listWgtApplication->setCurrentItem(curListWidItem);
        prevAction = popMenu->addAction(prevIcon,tr("Forward"));
        nextAction =  popMenu->addAction(nextIcon,tr("Backward"));
        separator1 = popMenu->addSeparator();
        delAction = popMenu->addAction(delIcon,tr("Delete"));

        if((itemAPPInfo.at(index).appStatus & 0x10) == 0x10)
        {
            separator2 = popMenu->addSeparator();
            releaseAction = popMenu->addAction(updateIcon,tr("Release"));
            connect(releaseAction,SIGNAL(triggered()),this,SLOT(releaseItem()));
        }
        connect(prevAction,SIGNAL(triggered()),this,SLOT(prevMoveItem()));
        connect(nextAction,SIGNAL(triggered()),this,SLOT(nextMoveItem()));
        connect(delAction,SIGNAL(triggered()),this,SLOT(removeItem()));

    }
    //如果在SYS选项卡
    else if(sysListWidgetItem != NULL && ui->tabFileView->currentIndex() == 1)
    {
        index = ui->listWgtSystem->currentRow();
        QAction *delAction;
        QAction *sigAction;
        //drivers.sh,daemon.sh,setting.ini无右击菜单
        if(itemSYSInfo.at(index).type == SHELL)
            return;
        delAction = popMenu->addAction(delIcon,tr("Delete"));
        if(itemSYSInfo.at(index).type == KERNEL || itemSYSInfo.at(index).type == UBOOT
                || itemSYSInfo.at(index).type == CRT || itemSYSInfo.at(index).type == CRAMFS)
        {
            sigAction = popMenu->addAction(updateIcon,tr("Signature"));
            connect(sigAction,SIGNAL(triggered()),this,SLOT(releaseItem()));
        }

        connect(delAction,SIGNAL(triggered()),this,SLOT(removeItem()));

    }
    else if(srcTreeWidgetItem != NULL && srcTreeWidgetItem->parent() != NULL && ui->tabResourceView->currentIndex() == 0)
    {
        QAction *delAction;
        QAction *refreshAction;

        delAction = popMenu->addAction(delIcon,tr("Delete"));
        refreshAction = popMenu->addAction(refreshIcon,tr("Refresh"));
        connect(delAction,SIGNAL(triggered()),this,SLOT(removeTreeItem()));
        connect(refreshAction,SIGNAL(triggered()),this,SLOT(refreshTreeItem()));
    }
    else if(cfgTreeWidgetItem != NULL && cfgTreeWidgetItem->parent() != NULL && ui->tabResourceView->currentIndex() == 1)
    {
        QAction *delAction;
        QAction * releaseAction;

        delAction = popMenu->addAction(delIcon,tr("Delete"));
        releaseAction = popMenu->addAction(updateIcon,tr("Release"));

        connect(delAction,SIGNAL(triggered()),this,SLOT(removeTreeItem()));
        connect(releaseAction,SIGNAL(triggered()),this,SLOT(releasePln()));
    }
    else
        return;

    popMenu->exec(QCursor::pos());
}
void SMTMainWindow::prevMoveItem()
{
    if(lock)
        return;
    tools.swapItemOnListWidget(PREV);
    clear();
}

void SMTMainWindow::nextMoveItem()
{
    if(lock)
        return;
    tools.swapItemOnListWidget(NEXT);
    clear();
}

void SMTMainWindow::removeItem()
{
    int index;

    if(lock)
        return;

    if(ui->tabFileView->currentIndex() == 0)
    {
        tools.removeItemOnListWidget(APPLICATION);
        if(ui->listWgtApplication->count() > 0)
        {
            showItemInfo(APPLICATION);
            return;
        }
    }
    else
    {
        index = ui->listWgtSystem->currentRow();
        tools.removeItemOnListWidget(itemSYSInfo.at(index).type);

        if(ui->listWgtSystem->count() > 0)
        {
            index = ui->listWgtSystem->currentRow();
            showItemInfo(itemSYSInfo.at(index).type);
            return;
        }
    }
    clear();

}

/*
void SMTMainWindow::removeMultipleItems()
{
    int index = -1, count = 0, i;
    QItemSelectionModel itemSelectionModel;
    QModelIndexList modelIndexList;
    if(lock)
        return;
    if(ui->tabFileView->currentIndex() == 0)
    {
        ui->listWgtApplication->setSelectionModel($itemSelectionModel);
        modelIndexList = itemSelectionModel.selectedIndexes();
        count = modelIndexList.count();
        if(count > 0)
        {
            for()
        }
    }
}*/

void SMTMainWindow::removeTreeItem()
{
    QTreeWidgetItem *curTreeItem;
    if(ui->tabResourceView->currentIndex() == 0)
    {
        curTreeItem = ui->treeResource->currentItem();
        if(curTreeItem->parent() != NULL)
            ui->treeResource->removeChileTree(curTreeItem);
    }
    else
    {
        curTreeItem = ui->treeScheme->currentItem();
        if(curTreeItem->parent() != NULL)
            tools.removeScheme(curTreeItem);
    }
}

void SMTMainWindow::refreshTreeItem()
{
    QTreeWidgetItem *curTreeItem = ui->treeResource->currentItem();
    if(curTreeItem->parent() != NULL)
        ui->treeResource->refreshChileTree(curTreeItem);
}

void SMTMainWindow::releaseItem()
{
    int index;

    if(lock)
        return;

    if(ui->tabFileView->currentIndex() == 0)
    {
        index = ui->listWgtApplication->currentRow();
        msgDis->setText(tr("START"));
        tools.sleep(500);
        tools.initMsgPorBar(100);
        if(tools.updateItemToPos(APPLICATION,index,1))
        {
            msgProBar->setValue(100);
            msgDis->setText(tr("SUCCESS"));
        }
        else
            msgDis->setText(tr("FAIL"));
    }
    else if(ui->tabFileView->currentIndex() == 1)
    {
        index = ui->listWgtSystem->currentRow();
        CSYSItemInfo itemTemp = itemSYSInfo.at(index);
        msgDis->setText(tr("START"));
        tools.sleep(500);
        tools.initMsgPorBar(100);
        if(tools.creatRSAFile(&itemTemp))
        {
            msgProBar->setValue(100);
            msgDis->setText(tr("SUCCESS"));
        }
        else
            msgDis->setText(tr("FAIL"));
    }
}


void SMTMainWindow::releasePln()
{
    if(lock)
        return;
    QTreeWidgetItem *curTreeItem = ui->treeScheme->currentItem();
    if(curTreeItem->parent() != NULL)
    {
        msgDis->setText(tr("START"));
        if(tools.releasePlnToUSB(curTreeItem))
            msgDis->setText(tr("SUCCESS"));
        else
            msgDis->setText(tr("FAIL"));
    }
}

void SMTMainWindow::on_btnApply_clicked()
{
    int index = 0;
    int replaceIndex = 0;

    if(lock)
        return;

    //应用
    if(ui->tabFileView->currentIndex() == 0)
    {
        index = ui->listWgtApplication->currentRow();

        if(ui->editAppID->text().isEmpty() || ui->editAppDes->text().isEmpty() || index == -1)
            return;

        CAPPItemInfo itemTemp = itemAPPInfo.at(index);
        //对于没有从POS获取详细信息的APP不允许修改
        if((itemTemp.appStatus & 0x40) == 0x40 && !ui->editAppSettings->toPlainText().isEmpty())
        {
            msgDis->setText(tr("Please double click APP to edit \"Business Setting\"!"));
            return;
        }
        itemTemp.appNumber = ui->editAppID->text().toInt();

        replaceIndex = itemTemp.isItemExist(&itemAPPInfo);
        //如果添加应用有冲突
        if(index != replaceIndex && replaceIndex != -1)
        {
            //先置灰除更新覆盖为的其它控件
            if(!ui->radioButtonCover->isEnabled())
            {
                ui->editAppID->setEnabled(false);
                ui->editAppDes->setEnabled(false);
                ui->editAppSettings->setEnabled(false);
                ui->editItemCode->setEnabled(false);
                ui->editAppVersion->setEnabled(false);
                ui->chkBoxAppHide->setEnabled(false);
                ui->listWgtImage->setEnabled(false);
                ui->treeResource->setEnabled(false);
                ui->radioButtonCover->setEnabled(true);
                ui->radioButtonUpdate->setEnabled(true);
                ui->labelWarning->show();
                return;
            }
            else if((itemAPPInfo.at(replaceIndex).appStatus & 0x10) == 0x00)
            {
                //如果用户选择覆盖,设置更新类型为覆盖
                if(ui->radioButtonCover->isChecked())
                    itemTemp.editType = 2;
                else
                    itemTemp.editType = 1;
            }
        }
/*
        if(itemTemp.appDesc.compare(ui->editAppDes->text()) != 0)
        {

            itemTemp.appStatus |= 0x20;
            //if((itemTemp.appStatus & 0x10) == 0x00)
            //    itemTemp.editType = 2;
        }
*/
        itemTemp.appDesc = ui->editAppDes->text();
        itemTemp.hideFlag = ui->chkBoxAppHide->isChecked();
        itemTemp.version = ui->editAppVersion->text();
        itemTemp.itemCode = ui->editItemCode->text();

        if(itemTemp.appSettings.compare(ui->editAppSettings->toPlainText()) != 0)
        {
            itemTemp.appSettings = ui->editAppSettings->toPlainText();
            itemTemp.appStatus |= 0x24;
            if((itemTemp.appStatus & 0x10) == 0x00)
                itemTemp.editType = 1;
        }

        if(itemTemp.ICOPicPath.compare(curIcoPath) != 0)
        {
            itemTemp.ICOPicPath = curIcoPath;
            itemTemp.appStatus |= 0x21;
            if((itemTemp.appStatus & 0x10) == 0x00)
                itemTemp.editType = 1;
        }
        if(itemTemp.logoPicPath.count() != curLogoPath.count() || curDelPicture.count() > 0)
        {
            itemTemp.appStatus |= 0x22;
            itemTemp.logoPicPath = curLogoPath;
            itemTemp.delPicture = curDelPicture;
            if((itemTemp.appStatus & 0x10) == 0x00)
                itemTemp.editType = 1;
        }
        else
        {
            for(int i=0;i<curLogoPath.count();i++)
            {
                if(curLogoPath.at(i).compare(itemTemp.logoPicPath.at(i)) != 0)
                {
                    itemTemp.appStatus |= 0x22;
                    itemTemp.logoPicPath = curLogoPath;
                    if((itemTemp.appStatus & 0x10) == 0x00)
                        itemTemp.editType = 1;
                    break;
                }
            }
        }

        if(!ui->chkBoxUIScript->isChecked())
            curUIScriptPath.clear();
        if(itemTemp.uiScriptPath.compare(curUIScriptPath) != 0)
        {
            itemTemp.uiScriptPath = curUIScriptPath;
            itemTemp.delUiScriptName = curDelUIScriptName;
            itemTemp.appStatus |= 0x28;
            if((itemTemp.appStatus & 0x10) == 0x00)
                itemTemp.editType = 1;
        }

        tools.modifyItemOnListWidget(&itemTemp,APPLICATION);
    }
    //其它类型如Drivers，lib等
    else
    {
        if((index = ui->listWgtSystem->currentRow()) == -1)
            return;
        CSYSItemInfo itemTemp = itemSYSInfo.at(index);
        if(itemTemp.type == SHELL)
        {

            //对是否加载进行处理
            if(itemTemp.srcName.compare("drivers.sh") == 0)
                tools.updateInsmodList(DRIVER);
            else if(itemTemp.srcName.compare("setting.ini") == 0)
                tools.updateDaemonSet();
            else if(itemTemp.srcName.compare("daemon.sh") == 0)
                tools.updateInsmodList(DAEMON);
            return;
        }
        if(ui->editFileName->text().isEmpty())
            return;

        if(itemTemp.version.compare(ui->editSysVersion->text()) != 0)
        {
            itemTemp.version = ui->editSysVersion->text();
            itemTemp.status |= 0x20;
        }
        tools.modifyItemOnListWidget(&itemTemp,itemTemp.type);
    }
    clear();
    return;
}

void SMTMainWindow::on_listWgtApplication_itemDoubleClicked(QListWidgetItem *item)
{
    const int cmdNo = 4;
    //双击应用时,获取应用配置文件,应用图标,应用所用到的打印图片和UI脚本文件
    unsigned char cmdArray[cmdNo][2] = {{0x31,0x32},{0x31,0x34},{0x31,0x35},{0x31,0x37}};
    QFile tempFile(UPDATE_DATA_TEMP_NAME);
    QFile zipFile(UPDATE_DATA_NAME);
    bool result = true;
    int timeOut = 0;
    int index = 0;

    if(lock)
        return;
    lock = true;

    if(zipFile.exists())
        zipFile.remove();
    if(tempFile.exists())
        tempFile.remove();
    index = ui->listWgtApplication->currentRow();
    if((itemAPPInfo.at(index).appStatus & 0x50) != 0x40)
    {
        lock = false;
        return;
    }
    msgDis->setText("1/1");
    while(timeOut++ < 3)
    {
        result = tools.getItemInfoFromPos(cmdArray,cmdNo);
        if(result)
            break;
        tools.sleep(500);
    }

    if(result)
    {
        showItemInfo(item->type()%1000);
        msgDis->setText(tr("FINISH!"));
    }
    else
        msgDis->setText(tr("ERROR!"));
    lock = false;
}

void SMTMainWindow::on_listWgtImage_itemDoubleClicked(QListWidgetItem *item)
{
    int index = ui->listWgtImage->currentRow();
    int i;
    QString picture;

    if(lock)
        return;

    if(index == 0)
    {
        curIcoPath = "";
        tools.showImage(curIcoPath);
        ui->listWgtImage->takeItem(index+1);
    }
    else
    {
        picture = QFileInfo(curLogoPath.at(index-1)).fileName();
        ui->listWgtImage->takeItem(index);
        curLogoPath.takeAt(index-1);
        //先查找图片删除列表中是否有此相同的文件名,没有的话添加到删除列表中
        for(i=0;i<curDelPicture.count();i++)
        {
            if(picture.compare(curDelPicture.at(i)) == 0)
                break;
        }
        if(i >= curDelPicture.count())
            curDelPicture<<picture;
    }
}

void SMTMainWindow::on_actionConfig_triggered()
{
    SessionOptions optionsWindow;
    optionsWindow.exec();

    //当传输方式为串口并且未连接时更新串口设置信息
    if(transType == 0 && ui->actionConnect->isEnabled())
    {
        QString strBuf, port, baudRate;
        port = tools.getPrivateProfileString("PortSettings","port","COM1",NULL);
        baudRate = tools.getPrivateProfileString("PortSettings","baudRate","115200",NULL);
        strBuf = "  "+port+" :"+baudRate + " ";
        strBuf += tr("Disconnect");
        msgLable->setText(strBuf);

        strBuf = QString(SMT_VERSION)+" [SERIAL " + port + "] ";
        strBuf += tr("Disconnect");
        this->setWindowTitle(strBuf);
    }
}

void SMTMainWindow::on_actionUpdate_triggered()
{
    QMessageBox msgBox;
    QString msg;

    if(lock)
        return;
    lock = true;
    if(tools.updateAllItem())
    {
        msgBox.setWindowTitle(tr("Congratulations"));
        msgBox.setIconPixmap(QPixmap(":resource/ico/smile.png"));
        msg = tr("Update Success!");
        ui->actionUpdate->setEnabled(false);
    }
    else
    {
        msgBox.setWindowTitle(tr("Sorry"));
        msgBox.setIconPixmap(QPixmap(":resource/ico/cry.png"));
        msg = tr("Update Failure!");
    }
    msgBox.setText(msg);
    msgBox.exec();
    lock = false;
}

void SMTMainWindow::on_btnCreateScheme_clicked()
{
    QString schemeName;
    if(!ui->editSchemeName->text().isEmpty())
    {
        schemeName = ui->editSchemeName->text().replace(" ","");
        tools.saveScheme(schemeName.toLocal8Bit().data());
    }
}


void SMTMainWindow::on_treeScheme_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(lock)
        return;
    if(item->parent() != NULL)
        tools.addItemFromScheme(item);
}

void SMTMainWindow::on_chkBoxUIScript_stateChanged(int arg1)
{
    int index = ui->listWgtApplication->currentRow();
    if(index < 0 || ui->tabFileView->currentIndex() != 0)
        return;
    if(arg1 == Qt::Unchecked)
    {
        if((itemAPPInfo.at(index).appStatus & 0x10) == 0)
        {
            //对于删除UI脚本,将删除的UI脚本名保存起来
            if(curDelUIScriptName.isEmpty())
            {
                curDelUIScriptName = QFileInfo(curUIScriptPath).fileName();
            }
        }

        //curUIScriptPath.clear();
    }
    else
    {
        if((itemAPPInfo.at(index).appStatus & 0x10) == 0)
        {
            //如果添加的UI脚本名与以删除的UI脚本名字相同,则清空已删除变量
            if(!curDelUIScriptName.isEmpty() && curDelUIScriptName.compare(QFileInfo(curUIScriptPath).fileName()) == 0)
                curDelUIScriptName.clear();
        }
    }
}

void SMTMainWindow::on_actionNetWork_Server_triggered()
{
    QString strBuf;
    //QString localHostName = QHostInfo::localHostName();
    //QHostInfo info = QHostInfo::fromName(localHostName);
    strBuf = "  IP Port:"+tools.getPrivateProfileString("PortSettings","IPPort","11111",NULL) + " ";
    strBuf += tr("Disconnect");
    msgLable->setText(strBuf);
    strBuf = QString(SMT_VERSION)+" [NETWORK] ";
    strBuf += tr("Disconnect");
    this->setWindowTitle(strBuf);
    transType = 1;
}

void SMTMainWindow::on_actionSerial_Port_triggered()
{
    QString port,baudRate,strBuf;
    port = tools.getPrivateProfileString("PortSettings","port","COM1",NULL);
    baudRate  = tools.getPrivateProfileString("PortSettings","baudRate","115200",NULL);
    strBuf = "  "+port+" :"+baudRate + " ";
    strBuf += tr("Disconnect");
    msgLable->setText(strBuf);
    strBuf = QString(SMT_VERSION)+" [SERIAL " + port +"] ";
    strBuf += tr("Disconnect");
    this->setWindowTitle(strBuf);
    transType = 0;
}

void SMTMainWindow::on_actionVersion_triggered()
{
    QMessageBox msgBox;
    QString msg;

    msgBox.setWindowTitle(tr("Version"));
    msgBox.setIconPixmap(QPixmap(":resource/ico/logo.png"));
    msg = "<html><B>SMT</B>: SAND Management Tool<br><B>Version</B>: ";
    msg += SMT_VERSION;
    msg += "<br>Copyright 2012 Shanghai SAND.All rights reserved.";
    msg += "<br><a href=http://www.sand.com.cn>www.sand.com.cn</a></html>";
    msgBox.setText(msg);
    msgBox.exec();
}

void SMTMainWindow::on_actionExit_triggered()
{
    SMTMainWindow::close();
}
void SMTMainWindow::creatNewConnect()
{
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
        return;

    tcpSocket = tcpServer->nextPendingConnection(); //得到每个连进来的socket
    tcpSocket->setReadBufferSize(0);
    //qDebug()<<QString::number(tcpSocket->readBufferSize());
    //将创建的tcpSocket传递给Tools
    tools.getSocket(tcpSocket);
    //connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(onReadyRead())); //有可读的信息，触发读函数
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disconnectNetwork()));
    msgLable->setStyleSheet("QLabel { background-color : #a3cf62;}");
}

void SMTMainWindow::disconnectNetwork()
{
    tcpSocket->close();
    msgLable->setStyleSheet("QLabel { background-color : #f58f98;}");
}

void SMTMainWindow::on_actionArm_GCC_triggered()
{

    QProcess p;
    QString msg;
    p.start("cmd.exe", QStringList() << "/c" << "d:\\test.bat");
    if (p.waitForStarted())
    {
        while(1)
        {
            tools.sleep(20);
            msg = QString(p.readAllStandardOutput());
            if(!msg.isEmpty())
                ui->plnEditSerialPort->appendPlainText(msg);
            if(p.state() == QProcess::NotRunning)
                break;
        }
        msg = QString(p.readAllStandardOutput());
        if(!msg.isEmpty())
            ui->plnEditSerialPort->appendPlainText(msg);
    }
    else
        ui->plnEditSerialPort->appendHtml("source file error\n");;
}
