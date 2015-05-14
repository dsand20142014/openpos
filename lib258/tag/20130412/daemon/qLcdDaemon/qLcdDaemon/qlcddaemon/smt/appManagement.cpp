#include "appManagement.h"


AppManagement::AppManagement(QWidget *parent) :
    BaseForm(tr("APP MANAGEMENT"), parent)
{

    COL_ID = 0;
    COL_INFO = 1;
    COL_CHB = 2;
    APPROLE = 0xfe;
    appNum = 0;

    tableWidget = new QTableWidget();
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setResizeMode(COL_CHB, QHeaderView::Stretch);
    tableWidget->setColumnCount(3);
    tableWidget->setColumnWidth(COL_ID, 60);
    //tableWidget->setColumnWidth(COL_INFO, 100);
    tableWidget->setColumnWidth(COL_CHB, 30);
    //tableWidget->setStyleSheet("selection-background-color: rgb(0,24,59); selection-color: white");
    tableWidget->setFocusPolicy(Qt::NoFocus);
    tableWidget->verticalHeader()->setResizeMode(COL_INFO, QHeaderView::Stretch);

    QTableWidgetItem *headerId = new QTableWidgetItem(tr("ID"));
    tableWidget->setHorizontalHeaderItem(COL_ID, headerId);
    QTableWidgetItem *headerName = new QTableWidgetItem(tr("Name"));
    tableWidget->setHorizontalHeaderItem(COL_INFO, headerName);
    QTableWidgetItem *headerHid = new QTableWidgetItem(tr("Hid"));
    tableWidget->setHorizontalHeaderItem(COL_CHB, headerHid);

    btnDel = new Button(tr("Delete"));
    btnDel->setMinimumSize(50, 30);
    btnUp = new Button(tr("Up"));
    btnUp->setMinimumSize(50, 30);
    btnDown = new Button(tr("Down"));
    btnDown->setMinimumSize(50, 30);


    QHBoxLayout *layouth = new QHBoxLayout();
    layouth->addWidget(btnUp);
    layouth->addWidget(btnDown);
    layouth->addWidget(btnDel);

    layout->addLayout(layouth);
    layout->addWidget(tableWidget);
    layout->setContentsMargins(10, 0, 10, 0);

    btnYes = new QPushButton(tr("Ok"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnYes->setMinimumSize(80, 30);
    btnCancel->setMinimumSize(80, 30);

    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowFlags(Qt::CustomizeWindowHint);
    msgBox.addButton(btnYes, QMessageBox::YesRole);
    msgBox.addButton(btnCancel, QMessageBox::NoRole);

    connect(btnUp, SIGNAL(clicked()), this, SLOT(slot_up()));
    connect(btnDown, SIGNAL(clicked()), this, SLOT(slot_down()));
    connect(btnOk, SIGNAL(clicked()), this, SLOT(slot_save()));
    connect(btnDel, SIGNAL(clicked()), this, SLOT(slot_delete()));

    btnOk->show();

    setAppList();//"/daemon/conf.ini");
}

void AppManagement::slot_save()
{
try {
    /* Get list from tableWidget */
    int n = tableWidget->rowCount();
    int i = 0;

    for (i=0; i<n; i++)
    {
        QTableWidgetItem *item = tableWidget->item(i, COL_ID); //ID
        //QString strName = tableWidget->itemAt(i, COL_INFO)->text(); //Name
        QCheckBox *checkBox = (QCheckBox *)tableWidget->cellWidget(i, COL_CHB);

        APPINFOLIST list = item->data(APPROLE).value<APPINFOLIST>();

        memcpy(&appList[i], &list, sizeof(APPINFOLIST));

        if(checkBox->isChecked()) {
            memcpy(appList[i].hidden, "1", 2);
        } else {
            memcpy(appList[i].hidden, "0", 2);
        }
    }

    if(!delList.isEmpty()) {
        for(int j=0; j<delList.count(); j++) {
            memcpy(&appList[n+j], &delList.at(j), sizeof(APPINFOLIST));
        }
    }
#ifdef debug
    /*debug*/
    for(i=0; i<appNum; i++)
    {
        qDebug("%s %s", appList[i].id, appList[i].name);
    }
#endif
    try {
        if (Os__applist_set(appList, appNum) < 0) {
            qDebug("Save ERROR: Os__applist_set failed!");
            QMessageBox::warning(this, "Error", "Save ERROR: Os__applist_set failed!", QMessageBox::Ok);
        }
    }
    catch (int e) {
        qDebug("QT catch exception: Os__applist_set ERROR");
    }

    close();

    }
    catch (int e) {
        qDebug("QT catch exception!");
    }
}

void AppManagement::slot_delete()
{
    int r = tableWidget->currentRow();
    if (r < 0) {
        return;
    }
    msgBox.setText(tr("Are you sure to delete?"));
    msgBox.exec();
    if(msgBox.clickedButton() == btnCancel) {
        return;
    }    

    QTableWidgetItem *item = tableWidget->item(r, COL_ID);

    QVariant v = item->data(APPROLE);

    if (!v.isNull()) {
        APPINFOLIST list = v.value<APPINFOLIST>();

        memcpy(list.edit, "3", 2);
        delList.append(list);
    }

    tableWidget->removeRow(r);
    tableWidget->update();
}

void AppManagement::slot_up()
{
    int r = tableWidget->currentRow();

    if (r <= 0)
        return;

    bool check = false;

    QCheckBox *checkBox = (QCheckBox *)tableWidget->cellWidget(r, COL_CHB);
    QCheckBox *checkBoxUp = (QCheckBox *)tableWidget->cellWidget(r-1, COL_CHB);
    if (checkBox == NULL || checkBoxUp == NULL) {
        qDebug("slot_up: currentItem is NULL!!");
        return;
    }

    QTableWidgetItem *itemid = tableWidget->takeItem(r, COL_ID);
    QTableWidgetItem *item = tableWidget->takeItem(r, COL_INFO);
    tableWidget->setItem(r, COL_ID, tableWidget->takeItem(r-1, COL_ID));
    tableWidget->setItem(r, COL_INFO, tableWidget->takeItem(r-1, COL_INFO));
    tableWidget->setItem(r-1, COL_ID, itemid);
    tableWidget->setItem(r-1, COL_INFO, item);

    check = checkBox->isChecked();
    checkBox->setChecked(checkBoxUp->isChecked());
    checkBoxUp->setChecked(check);

    tableWidget->setCurrentItem(item);
}

void AppManagement::slot_down()
{
    int r = tableWidget->currentRow();    

    if (r < 0 || r == tableWidget->rowCount() - 1)
        return;

    bool check = false;

    QCheckBox *checkBox = (QCheckBox *)tableWidget->cellWidget(r, COL_CHB);
    QCheckBox *checkBoxDown = (QCheckBox *)tableWidget->cellWidget(r+1, COL_CHB);
    if (checkBox == NULL || checkBoxDown == NULL) {
        qDebug("slot_down: currentItem is NULL!!");
        return;
    }

    QTableWidgetItem *itemid = tableWidget->takeItem(r, COL_ID);
    QTableWidgetItem *item = tableWidget->takeItem(r, COL_INFO);
    tableWidget->setItem(r, COL_ID, tableWidget->takeItem(r+1, COL_ID));
    tableWidget->setItem(r, COL_INFO, tableWidget->takeItem(r+1, COL_INFO));
    tableWidget->setItem(r+1, COL_ID, itemid);
    tableWidget->setItem(r+1, COL_INFO, item);

    check = checkBox->isChecked();
    checkBox->setChecked(checkBoxDown->isChecked());
    checkBoxDown->setChecked(check);

    tableWidget->setCurrentItem(item);
}

void AppManagement::setAppList()//QString filename)
{
    int i, n = 0;
try {
    n = Os__applist_get(appList, 50);
    }
catch (int e) {
    qDebug("QT catch exception: Os__applist_get ERROR");
    }

    appNum = n;

    if (n <= 0) {
        btnOk->setEnabled(false);
        btnUp->setEnabled(false);
        btnDel->setEnabled(false);
        btnDown->setEnabled(false);
    }

    tableWidget->setRowCount(n);

    for (i=0; i<n; i++) {

        QTableWidgetItem *item = new QTableWidgetItem(appList[i].id);//set.value("ID").toString());
        QVariant v;
        v.setValue(appList[i]);
        item->setData(APPROLE, v);
        tableWidget->setItem(i, COL_ID, item);

        item = new QTableWidgetItem(appList[i].name);
        tableWidget->setItem(i, COL_INFO, item);

        QCheckBox *checkBox = new QCheckBox(this);
        if (appList[i].hidden[0] != '0') {
            checkBox->setChecked(true); //隐藏的APP
        }
        checkBox->setMinimumSize(20, 20);
        tableWidget->setCellWidget(i, COL_CHB, checkBox);

    }
}


AppManagement::~AppManagement()
{
}
