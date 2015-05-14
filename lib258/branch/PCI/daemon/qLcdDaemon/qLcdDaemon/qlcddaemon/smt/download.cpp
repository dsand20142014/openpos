#include <QFileDialog>
#include "download.h"
#include "common.h"

extern "C"{
#include "sand_key.h"

}

Download::Download(int usb, QWidget *parent) :
    BaseForm(tr("DOWNLOAD"), parent)
{    

    if (usb) {
        // USB download
        USB = usb;
        title->setText(tr("USB DOWNLOAD"));
    }
    else {
        USB = 0;
        // TF download
        title->setText(tr("TF DOWNLOAD"));
    }

    //messagebox
    btnCover = new QPushButton();
    btnUpdate = new QPushButton();
    btnCover->setMinimumSize(80, 30);
    btnUpdate->setMinimumSize(80, 30);

    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowFlags(Qt::CustomizeWindowHint);
    msgBox.addButton(btnCover, QMessageBox::YesRole);
    msgBox.addButton(btnUpdate, QMessageBox::NoRole);

    /* mount tf/usb */
retry:
    if (Os__mt_active(1, USB) < 0) {
        QString device;
        if (USB) {
            device = tr("Cannot find USB device!\nPlease insert your device and click Ok to retry.");
        } else {
            device = tr("Cannot find TF device!\nPlease insert your device and click Ok to retry.");
        }
        /* 挂载失败 */        
        btnCover->setText(tr("Ok"));
        btnUpdate->setText(tr("Cancel"));
        msgBox.setText(device);

        msgBox.exec();

        if (msgBox.clickedButton() == btnCover) {
            goto retry;
        } else {
            close();
        }

    } else {

        layout->setMargin(0);
        listWidget = new QListWidget();
        listWidget->setIconSize(QSize(20, 20));
        listWidget->setContentsMargins(QMargins(5,5,5,5));

        connect(listWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(slot_listWidget_clicked(QModelIndex)));
        layout->addWidget(listWidget);

        currentDir = "/mnt/mt";
        QDir dir(currentDir);
        getFile(dir);

        //show detail
        listApp = new tableWidget();

        layout->setMargin(0);

        btnOk->show();
        btnOk->setDisabled(true);
        connect(btnOk, SIGNAL(clicked()), this, SLOT(slot_btnOk_clicked()));        
    }
}

void Download::getFile(QDir dir)
{
    int type;        
    QStringList filters;

    listWidget->clear();

    filters << "*.pln";
    dir.setFilter(QDir::AllDirs|QDir::Files);
    dir.setNameFilters(filters);
    QFileInfoList list = dir.entryInfoList();    

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        //printf("%s\n", fileInfo.fileName().toLatin1().data());
        QIcon icon;

        if(fileInfo.isDir()) {
            icon.addFile("/daemon/image/dir.png");
            type = 1;
        }
        else {
            icon.addFile("/daemon/image/file.png");
            type = 0;
        }
        new QListWidgetItem(icon, tr(fileInfo.fileName().toLocal8Bit().data()), listWidget, type); // utf8

    }
}

void Download::checkApp()
{
    btnCover->setText(tr("Cover"));
    btnUpdate->setText(tr("Update"));

try{
    QSettings set(conffile, QSettings::IniFormat);
    QSettings settings("/daemon/conf.ini", QSettings::IniFormat);

    int n =  set.childGroups().count();
    int m =  settings.childGroups().count();

    if (n == 0 || m == 0){
        return;
    }
    QString strwarning = " is already installed!\n[Cover] to delete old app.\n[Update] to update the app.";

    for (int i=0; i<n; i++) {

        QString name = "APP";
        name.append(QString::number(i+1));
        set.beginGroup(name);

        QString strid = set.value("ID").toString();
        QString strupdate = set.value("UPDATE").toString();

        set.setValue("EDIT", "0");

        for (int j=0; j<m; j++) {
            QString strname = "APP";
            strname.append(QString::number(j+1));
            settings.beginGroup(strname);

            if(strid == settings.value("ID").toString() && strupdate == "1") {
                msgBox.setText(strid.append(strwarning));
                msgBox.exec();

                if(msgBox.clickedButton() == btnCover) {
                /*if(QMessageBox::warning(this, tr("Warning"), strid.append(strwarning),
                                  QMessageBox::Save, QMessageBox::Discard) == QMessageBox::Save) {*/
                    /* cover 2 */
                    set.setValue("EDIT", "2");
                } else {
                    set.setValue("EDIT", "1");
                    //qDebug("hidden: %s", settings.value("HIDDEN").toString().toLatin1().data());
                    set.setValue("HIDDEN", settings.value("HIDDEN").toString());
                }
            }
            settings.endGroup();
        }
        set.endGroup();
    }
  }
    catch(QString exception) {
        perror("Download catch exception!\n");
    }
}

void Download::slot_btnOk_clicked()
{
    //Os__set_key_ensure(200);

    listApp->getData(conffile);

    /* check APP */
    checkApp();

    /* begin to download */
    Downloading *dl = new Downloading(true);
    dl->showMaximized();

    //close();//FIXME need?
}

void Download::slot_listWidget_clicked(QModelIndex index)
{
    //Os__set_key_ensure(200);

    int r = index.row();
    QDir dir(currentDir);
    if (currentDir == "mnt") {
        return;
    }

    QListWidgetItem *listitem = listWidget->item(r);
    QString name = listitem->text();


    if(listitem->type() == 1) { //dir

        currentDir.append("/");
        currentDir.append(name);

        if(!dir.cd(currentDir)) {
            qWarning("Cannot find the directory: %s", currentDir.toLatin1().data());
        }
        getFile(dir);
    } else {
        /* selection is a pln file */
        //printf("XXXXXXXXX APP %s will download...\n", name.toLatin1().data());
        QString filename = currentDir;
        filename.append("/");
        filename.append(name);

        /* express the pln file */
        conffile = Os__pln_unzip(filename.toLatin1().data());

        if(conffile != NULL) {
            layout->removeWidget(listWidget);
            listApp->setData(conffile);     
            layout->addWidget(listApp);
            btnOk->setDisabled(false);
        }
        else {
            messageBox *msgbox = new messageBox(tr("The pln file ERROR!"));
            msgbox->exec();
        }
    }
}
/*
void Download::keyPressEvent(QKeyEvent *e)
{
    Os__set_key_ensure(200); // for sleep
}

void Download::mousePressEvent(QMouseEvent *)
{
    Os__set_key_ensure(200); // for sleep
}
*/
Download::~Download()
{
    Os__mt_active(0, USB);
}
