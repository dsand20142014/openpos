#include "downloadMode.h"
#include <QLabel>
#include <QMovie>
extern "C"{
#include "param.h"
#include "sand_key.h"
}

DownloadMode::DownloadMode(QWidget *parent) :
    BaseForm(tr("APP DOWNLOAD"), parent)
{
    int i;
    /*syslist << "01.UART_EXIST"
            << "02.OTG_EXIST"
            << "03.WLAN_EXIST"
            << "04.WIFI_EXIST"
            << "05.TF_EXIST"
            << "06.USB_EXIST";*/

    QStringList icons;
    icons   << "image/UART.png"
            << "image/OTG2USB.png"
            << "image/WLAN.png"
            << "image/WIFI.png"
            << "image/TF.png"
            << "image/USB.png";

    QTableWidget *listWidget = new QTableWidget();
    listWidget->setShowGrid(false);
    listWidget->setFocusPolicy(Qt::NoFocus);
    listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listWidget->horizontalHeader()->setVisible(false);
    listWidget->verticalHeader()->setVisible(false);
    listWidget->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
    listWidget->setRowCount(6);
    listWidget->setColumnCount(2);
    listWidget->setColumnWidth(0, 35);
    listWidget->setContentsMargins(QMargins(10, 3, 10, 3));

    layout->setContentsMargins(QMargins(10, 0, 10, 0));
    layout->setSpacing(5);
    layout->addWidget(new QLabel(tr("Please select download mode:")));
    layout->addWidget(listWidget);

    rcom = new QRadioButton(tr("UART"));    
    rusb = new QRadioButton(tr("U DISK"));
    rwifi = new QRadioButton(tr("WIFI"));
    rnet = new QRadioButton(tr("LAN"));
    rtf = new QRadioButton(tr("TF CARD"));
    rotg = new QRadioButton(tr("OTG2UART"));

    for (i=0; i<6; i++) {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setIcon(QIcon(icons.at(i)));
        listWidget->setItem(i, 0, item);
    }

    listWidget->setCellWidget(0, 1, rcom);
    listWidget->setCellWidget(1, 1, rotg);
    listWidget->setCellWidget(2, 1, rnet);
    listWidget->setCellWidget(3, 1, rwifi);
    listWidget->setCellWidget(4, 1, rtf);
    listWidget->setCellWidget(5, 1, rusb);

#if 0
    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(rcom);
    verticalLayout->addWidget(rusb);
    verticalLayout->addWidget(rtf);
    verticalLayout->addWidget(rotg);
    verticalLayout->addWidget(new QLabel(tr("Network:")));

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(rwifi);
    horizontalLayout->addWidget(rnet);
    verticalLayout->addLayout(horizontalLayout);

    layout->addLayout(verticalLayout);
#endif
    connect(btnOk, SIGNAL(clicked()), this, SLOT(btnOk_clicked()));
    btnOk->show();

    checked = getMode();
    switch(checked) {
    case -1:
        qDebug("DownloadMode getMode failed:setting.ini\n");
        break;
    case 1:
        rcom->setChecked(true);
        break;
    case 2:
        rotg->setChecked(true);
        break;
    case 3:
        rnet->setChecked(true);
        break;
    case 4:
        rwifi->setChecked(true);
        break;
    case 5:
        rtf->setChecked(true);
        break;
    case 6:
    default:
        rusb->setChecked(true);
        break;
    }

    rcom->setEnabled(false);
    rnet->setEnabled(false);
    rwifi->setEnabled(false);
    rusb->setEnabled(false);
}

/*
 * Get the mode from setting.ini
 * Return: 1-UART 2-OTG2UART 3-WLAN 4-WIFI 5-TF 6-U DISK
 * -1 - failed
 */
int DownloadMode::getMode() //FIXME
{
    char v;
    //int i;
    unsigned int etcFile;
    unsigned int etcSection1;    
try {
    etcFile = Os__param_loadFullPath("/daemon/setting.ini");
    etcSection1 = Os__param_findSection(etcFile,"SERVER_CONFIG", 0);
    Os__param_get(etcSection1, "CONNECT_STATE", &v, 1);
    Os__param_unload(etcFile);

    if(v && v-'0'>=1 && v-'0' <= 6) {
        return v-'0';
    } else {
        return -1;
    }
    }
catch (int exception){
        qDebug("QT Catch Exception no = %d when <Os__param_get>\n", exception);
        return -1;
    }

}

/*
 * Set key to value in set
 */
void DownloadMode::setValue(QSettings *set, QString key, QString value)
{
    set->beginGroup("SERVER_CONFIG");

    if (value.isEmpty() || key.isEmpty() || !set->isWritable()) {
        return;
    }
    QString str = set->value(key).toString();

    if(str.isEmpty()) {
        qDebug("SERVER_CONFIG ERROR in setting.ini: %s !", key.toLatin1().data());
        return;
    }
    if (str == value) {
        return;
    }
    QStringList strvalue = str.split("#");
    str = " #";

    if (!strvalue.isEmpty()) {
        str.append(strvalue.at(1));
    }

    value.append(str);
    set->setValue(key, value);

    set->endGroup();
}

void DownloadMode::btnOk_clicked()
{
    /* 1-UART 2-OTG2UART 3-WLAN 4-WIFI 5-TF 6-U DISK */
try {

    QSettings set("/daemon/setting.ini", QSettings::IniFormat);    

    if(rcom->isChecked()) {
        //set COM        
        if(checked != 1) {
            checked = 1;
            setValue(&set, "CONNECT_STATE", "1");
        }

        dlcom = new Downloading();
        dlcom->showMaximized();
    }
    else if(rotg->isChecked()) {
        if(checked != 2) {
            checked = 2;
            setValue(&set, "CONNECT_STATE", "2");
        }

        dlcom = new Downloading();
        dlcom->showMaximized();
    }
    else if(rnet->isChecked()) {
        if(checked != 3) {
            checked = 3;
            setValue(&set, "CONNECT_STATE", "3");
        }

        dlcom = new Downloading();
        dlcom->showMaximized();
    }
    else if(rwifi->isChecked()) {
        if(checked != 4) {
            checked = 4;
            setValue(&set, "CONNECT_STATE", "4");
        }

        dlcom = new Downloading();
        dlcom->showMaximized();
    }
    else if(rtf->isChecked()) {
        // TF CARD
        if(checked != 5) {
            checked = 5;
            setValue(&set, "CONNECT_STATE", "5");
        }

        dl = new Download();
        dl->showMaximized();
    }
    else if(rusb->isChecked()) {
        // USB
        if(checked != 6) {
            checked = 6;
            setValue(&set, "CONNECT_STATE", "6");
        }

        dl = new Download(2);
        dl->showMaximized();
    }
}
catch (int exception) {
    qDebug("QT Catch Exception no = %d when <setValue>\n", exception);
}

}

void DownloadMode::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_Enter:
            btnOk_clicked();
            break;
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_1:
            rcom->setChecked(true);
            break;
        case Qt::Key_2:
            rotg->setChecked(true);
            break;
        case Qt::Key_3:
            rnet->setChecked(true);
            break;
        case Qt::Key_4:
            rwifi->setChecked(true);
            break;
        case Qt::Key_5:
            rtf->setChecked(true);
            break;
        case Qt::Key_6:
            rusb->setChecked(true);
            break;
    }
    e->accept();
}

DownloadMode::~DownloadMode()
{
}
