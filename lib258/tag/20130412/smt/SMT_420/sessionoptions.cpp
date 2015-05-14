#include "sessionoptions.h"
#include "ui_sessionoptions.h"
#include <QStringList>
#include <QAbstractButton>
#include <QFileDialog>


SessionOptions::SessionOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionOptions)
{
    ui->setupUi(this);
    ui->grpBoxSerial->show();
    ui->groBoxNET->hide();
    ui->groBoxLanguage->hide();
    ui->labelMsg->hide();
    ui->groBoxTerminal->hide();

    QStringList portListItems;

#if defined(Q_OS_WIN)
    portListItems<<"COM1"<<"COM2"<<"COM3"<<"COM4"<<"COM5"<<"COM6"<<"COM7"<<"COM8"<<"COM9"<<"COM10"
                <<"COM11"<<"COM12"<<"COM13"<<"COM14"<<"COM15"<<"COM16"<<"COM17"<<"COM18"<<"COM19"<<"COM20"
               <<"COM21"<<"COM22"<<"COM23"<<"COM24"<<"COM25"<<"COM26"<<"COM27"<<"COM28"<<"COM29"<<"COM30";
    ui->cmbBoxPort->addItems(portListItems);
    ui->cmbBoxPort->setEditText(tools.getPrivateProfileString("PortSettings","port","COM1",NULL));
#endif

#ifdef Q_OS_UNIX
    portListItems<<"tty1"<<"tty2"<<"tty3"<<"tty4"<<"tty5"<<"tty6"<<"tty7"<<"tty8"<<"tty9"<<"tty10"
                <<"tty11"<<"tty12"<<"tty13"<<"tty14"<<"tty15"<<"tty16"<<"tty17"<<"tty18"<<"tty19"<<"tty20"
               <<"tty21"<<"tty22"<<"tty23"<<"tty24"<<"tty25"<<"tty26"<<"tty27"<<"tty28"<<"tty29"<<"tty30";
    ui->cmbBoxPort->addItems(portListItems);
    ui->cmbBoxPort->setEditText(tools.getPrivateProfileString("PortSettings","port","tty1",NULL));
#endif

    ui->cmbBoxBaudRate->addItem(QLatin1String("110"), BAUD110);
    ui->cmbBoxBaudRate->addItem(QLatin1String("300"), BAUD300);
    ui->cmbBoxBaudRate->addItem(QLatin1String("600"), BAUD600);
    ui->cmbBoxBaudRate->addItem(QLatin1String("1200"), BAUD1200);
    ui->cmbBoxBaudRate->addItem(QLatin1String("2400"), BAUD2400);
    ui->cmbBoxBaudRate->addItem(QLatin1String("4800"), BAUD4800);
    ui->cmbBoxBaudRate->addItem(QLatin1String("9600"), BAUD9600);
    ui->cmbBoxBaudRate->addItem(QLatin1String("19200"), BAUD19200);
    ui->cmbBoxBaudRate->addItem(QLatin1String("38400"), BAUD38400);
    ui->cmbBoxBaudRate->addItem(QLatin1String("57600"), BAUD57600);
    ui->cmbBoxBaudRate->addItem(QLatin1String("115200"), BAUD115200);
    ui->cmbBoxBaudRate->setCurrentIndex(10);

    ui->cmbBoxDataBits->addItem(QLatin1String("5"), DATA_5);
    ui->cmbBoxDataBits->addItem(QLatin1String("6"), DATA_6);
    ui->cmbBoxDataBits->addItem(QLatin1String("7"), DATA_7);
    ui->cmbBoxDataBits->addItem(QLatin1String("8"), DATA_8);
    ui->cmbBoxDataBits->setCurrentIndex(3);

    ui->cmbBoxParity->addItem(QLatin1String("NONE"), PAR_NONE);
    ui->cmbBoxParity->addItem(QLatin1String("ODD"), PAR_ODD);
    ui->cmbBoxParity->addItem(QLatin1String("EVEN"), PAR_EVEN);

    ui->cmbBoxStopBits->addItem(QLatin1String("1"), STOP_1);
    ui->cmbBoxStopBits->addItem(QLatin1String("2"), STOP_2);

    ui->cmbBoxBaudRate->setEditText(tools.getPrivateProfileString("PortSettings","baudRate","115200",NULL));
    for(int i=0;i<ui->cmbBoxDataBits->count();i++)
    {
        if(ui->cmbBoxDataBits->itemText(i).compare(tools.getPrivateProfileString("PortSettings","dataBits","8",NULL)) == 0)
            ui->cmbBoxDataBits->setCurrentIndex(i);
    }
    for(int i=0;i<ui->cmbBoxParity->count();i++)
    {
        if(ui->cmbBoxParity->itemData(i).toInt() == tools.getPrivateProfileInt("PortSettings","parity",0,NULL))
            ui->cmbBoxParity->setCurrentIndex(i);
    }
    for(int i=0;i<ui->cmbBoxStopBits->count();i++)
    {
        if(ui->cmbBoxStopBits->itemData(i).toInt() == tools.getPrivateProfileInt("PortSettings","stopBits",0,NULL))
            ui->cmbBoxStopBits->setCurrentIndex(i);
    }

    //读取并设置IP信息
    ui->lineEditPort->setText(tools.getPrivateProfileString("PortSettings","IPPort","11111",NULL));
    //设置当前语言,CN为中文,EN为英文
    language = tools.getPrivateProfileString("PortSettings","Language","CN",NULL);
    if(language.compare("CN") == 0)
        ui->radioButCN->setChecked(true);
    else if(language.compare("EN") == 0)
        ui->radioButEN->setChecked(true);
    //设置密钥路径
    //connect(ui->buttonCancel, SIGNAL(clicked()), qApp, SLOT(quit()));
}

SessionOptions::~SessionOptions()
{
    delete ui;
}

void SessionOptions::on_treeWdtOptions_itemClicked(QTreeWidgetItem *item, int column)
{
    QString strTemp;
    strTemp = item->text(0);
    if(strTemp.contains(tr("Connection")) || strTemp.contains(tr("Serial")))
    {
        ui->labelTital->setText(tr("Serial"));
        ui->grpBoxSerial->show();
        ui->groBoxNET->hide();
        ui->groBoxLanguage->hide();
        ui->groBoxTerminal->hide();
    }
    else if(strTemp.contains(tr("NetWork")))
    {
        ui->labelTital->setText(tr("NetWork"));
        ui->groBoxNET->show();
        ui->grpBoxSerial->hide();
        ui->groBoxLanguage->hide();
        ui->groBoxTerminal->hide();
    }
    else if(strTemp.contains(tr("Terminal")))
    {
        ui->labelTital->setText(tr("Terminal"));
        ui->groBoxTerminal->show();
        ui->grpBoxSerial->hide();
        ui->groBoxNET->hide();
        ui->groBoxLanguage->hide();
    }
    else if(strTemp.contains(tr("Language")))
    {
        ui->labelTital->setText(tr("Language"));
        ui->grpBoxSerial->hide();
        ui->groBoxLanguage->show();
        ui->groBoxNET->hide();
        ui->groBoxTerminal->hide();
    }
}


void SessionOptions::on_buttonSave_clicked()
{
    ui->labelMsg->setText(tr("SAVED"));
    ui->labelMsg->setStyleSheet("QLabel { color : #FF0000;}");
    ui->labelMsg->show();
    tools.writePrivateProfileString("PortSettings","port",ui->cmbBoxPort->currentText(),NULL);
    tools.writePrivateProfileString("PortSettings","baudRate",ui->cmbBoxBaudRate->currentText(),NULL);
    tools.writePrivateProfileString("PortSettings","dataBits",ui->cmbBoxDataBits->currentText(),NULL);
    tools.writePrivateProfileString("PortSettings","parity",ui->cmbBoxParity->itemData(ui->cmbBoxParity->currentIndex()).toString(),NULL);
    tools.writePrivateProfileString("PortSettings","stopBits",ui->cmbBoxStopBits->itemData(ui->cmbBoxStopBits->currentIndex()).toString(),NULL);


    tools.writePrivateProfileString("PortSettings","IPPort",ui->lineEditPort->text(),NULL);


    if(ui->radioButCN->isChecked())
        language = "CN";
    else if(ui->radioButEN->isChecked())
        language = "EN";
    tools.writePrivateProfileString("PortSettings","Language",language,NULL);
    tools.sleep(500);
    ui->labelMsg->hide();

}

void SessionOptions::on_buttonClose_clicked()
{
    SessionOptions::close();
}
