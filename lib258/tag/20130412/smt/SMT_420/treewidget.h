#ifndef TREEWIDGET_H
#define TREEWIDGET_H
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDragEnterEvent>
#include <QString>
#include <QStringList>


class TreeWidget : public QTreeWidget
{
public:
    TreeWidget(QWidget *parent, Qt::WindowFlags f = 0);
protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void sortingFileDir(QTreeWidgetItem *parentWidgetItem, QString parentPath);
    bool addFileDirToTree(QString path,QTreeWidgetItem *parentWidgetItem);

public:
    QStringList rootPathList;
    QTreeWidgetItem *rootTreeWdtItem;
    QString getItemPath(QTreeWidgetItem *curTreeWdtItem);
    QTreeWidgetItem *addChildToRoot(QTreeWidgetItem *root, QString path, int index);
    QString getCurItemPath(QTreeWidgetItem *curTreeWdtItem);
    void sortingChildFile(QTreeWidgetItem *curTreeWdtItem);
    void removeChileTree(QTreeWidgetItem *curTreeWdtItem);
    void refreshChileTree(QTreeWidgetItem *curTreeWdtItem);


};

#endif // TREEWIDGET_H
