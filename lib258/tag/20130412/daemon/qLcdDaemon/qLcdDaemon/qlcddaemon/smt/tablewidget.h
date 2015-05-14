#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QTableWidget>

class tableWidget : public QTableWidget
{
public:
    tableWidget();
    void setData(QString filename);
    void getData(QString filename);

};

#endif // TABLEWIDGET_H
