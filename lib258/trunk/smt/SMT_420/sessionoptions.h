#ifndef SESSIONOPTIONS_H
#define SESSIONOPTIONS_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "tools.h"

namespace Ui {
class SessionOptions;
}

class SessionOptions : public QDialog
{
    Q_OBJECT
    
public:
    explicit SessionOptions(QWidget *parent = 0);
    ~SessionOptions();
    
private slots:
    void on_treeWdtOptions_itemClicked(QTreeWidgetItem *item, int column);

    void on_buttonSave_clicked();

    void on_buttonClose_clicked();

private:
    Ui::SessionOptions *ui;
    QString language;
    Tools tools;
};

#endif // SESSIONOPTIONS_H
