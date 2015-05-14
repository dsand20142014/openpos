#include "tablewidget.h"
#include <QCheckBox>
#include <QTextEdit>
#include <QSettings>
#include <QHeaderView>


tableWidget::tableWidget()
{
    setSelectionMode(QAbstractItemView::NoSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setFocusPolicy(Qt::NoFocus);

    horizontalHeader()->setVisible(false);
    verticalHeader()->setResizeMode(2, QHeaderView::Stretch);//FIXME
    setContentsMargins(5,0,5,0);

    setColumnCount(3);
    setColumnWidth(0, 25);
    setColumnWidth(1, 60);
    setColumnWidth(2, 115);
}

/*
 * Create a file list based on filename.
 */
void tableWidget::setData(QString filename)
{
    int i = 0;
    clearContents();

    QSettings set(filename,QSettings::IniFormat);

    int n =  set.childGroups().count();
    //qDebug("tableWidget::setData %d", n);
    setRowCount(n);

    for (i=0; i<n; i++)
    {
        QString name = "APP";
        name.append(QString::number(i+1));
        set.beginGroup(name);

        setRowHeight(i, 50);

        QCheckBox *checkBox = new QCheckBox(this);        
        checkBox->setChecked(true);
        checkBox->setMinimumSize(20, 20);
        setCellWidget(i, 0, checkBox);

        QString strname = set.value("NAME").toString();
        QTableWidgetItem *item = new QTableWidgetItem(tr(strname.toLocal8Bit().data())); //for i18n
        item->setTextAlignment(Qt::AlignCenter);

        //item->setIcon(QIcon(pic));
        setItem(i, 1, item);

        QString str = set.value("ID").toString();
        str.append("\n");
        str.append(set.value("EDITDATE").toString());
        item = new QTableWidgetItem(str);

        setItem(i, 2, item);

        set.endGroup();
    }
}

/*
 * set UPDATE to 1 in conf.ini when checked.
 */
void tableWidget::getData(QString filename)
{
    //get check list
    int i = 0;    
    int n = this->rowCount();
    QSettings set(filename,QSettings::IniFormat);

    for (i=0; i<n; i++)
    {
        QString name = "APP";
        name.append(QString::number(i+1));
        set.beginGroup(name);

        QCheckBox *checkBox = (QCheckBox *)cellWidget(i, 0);
        if(checkBox->isChecked()) {
            //checked
            set.setValue("UPDATE", 1);
        } else {
            //unchecked
            set.setValue("UPDATE", 0);
        }

        set.endGroup();
    }
}

