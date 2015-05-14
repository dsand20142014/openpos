#include <QSettings>
#include <QFileInfo>
#include <QIcon>
#include <QUrl>
#include <QFileIconProvider>
#include <QDir>
#include "treewidget.h"
#include "tools.h"

TreeWidget::TreeWidget(QWidget *parent, Qt::WindowFlags f):QTreeWidget(parent)
{
    QStringList rootPath;
    rootPath<<QObject::tr("WORKSPACE");
    rootTreeWdtItem = new QTreeWidgetItem(rootPath);


    rootTreeWdtItem->setIcon(0, QIcon(":resource/ico/ws.png"));
    addTopLevelItem(rootTreeWdtItem);

    //初始化时加载配置中的目录
    QSettings iniSettings(INI_FILE_CONFIG,QSettings::IniFormat);
    iniSettings.setIniCodec(INI_FILE_CODE);
    iniSettings.beginGroup("rootPath");
    QStringList allKeyList = iniSettings.allKeys();
    for(int i=0;i<allKeyList.count();i++)
    {
        QString pathTemp = iniSettings.value(allKeyList.at(i)).toString();
        if(QFileInfo(pathTemp).exists())
        {
            rootPathList<<pathTemp;
            sortingFileDir(addChildToRoot(rootTreeWdtItem,pathTemp,rootTreeWdtItem->childCount()),pathTemp);
        }
        else
        {
            iniSettings.remove(allKeyList.at(i));
        }
    }
    iniSettings.endGroup();

    rootTreeWdtItem->setExpanded(true);

}

void TreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}
void TreeWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QString temp;
    QSettings iniSettings(INI_FILE_CONFIG,QSettings::IniFormat);
    iniSettings.setIniCodec(INI_FILE_CODE);
    int i = 0;

    if(mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        temp = urlList.at(0).path();
        temp = temp.rightRef(temp.count()-1).toString();
        if(temp.isEmpty())
            return;

        for(i=0;i<rootPathList.count();i++)
        {
            if(rootPathList.at(i).compare(temp) == 0)
                return;
        }
    }
    else
        return;
    event->acceptProposedAction();
    QFileInfo fileInfo(temp);
    if(!fileInfo.isDir())
        return;
    rootPathList<<temp;
    sortingFileDir(addChildToRoot(rootTreeWdtItem,temp,rootTreeWdtItem->childCount()),temp);


    iniSettings.beginGroup("rootPath");
    QStringList allKeyList = iniSettings.allKeys();
    int addIndex = -1;
    if(allKeyList.count() == 0)
    {
        iniSettings.setValue("0",temp);
    }
    else
    {
        for(i=0;i<allKeyList.count();i++)
        {
            if(addIndex == -1)
            {
                if(i != allKeyList.at(i).toInt())
                    addIndex = i;
            }
            QString pathTemp = iniSettings.value(allKeyList.at(i)).toString();
            if(pathTemp.compare(temp) == 0)
                return;
        }
        if(addIndex == -1)
            iniSettings.setValue(QString::number(i,10),temp);
        else
            iniSettings.setValue(QString::number(addIndex,10),temp);
    }
    iniSettings.endGroup();
}

void TreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}


void TreeWidget::sortingFileDir(QTreeWidgetItem *parentWidgetItem, QString parentPath)
{
    QDir dir;
    dir.setPath(parentPath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    QFileIconProvider fileIcon;

    const QFileInfoList fileList = dir.entryInfoList();

    for (int i = 0; i < fileList.size(); i++)
    {
        QFileInfo fileInfo = fileList.at(i);
        QStringList fileColumn;
        QIcon icon = fileIcon.icon(fileInfo);
        QString subName = fileInfo.suffix();
        if(subName.contains("dwn") || subName.contains("ico"))
            continue;
        fileColumn.append(fileInfo.fileName());
        //if (fileInfo.fileName() == "." || fileInfo.fileName() == ".." ); // nothing
        if(fileInfo.isDir()) {
            QTreeWidgetItem *child = new QTreeWidgetItem(fileColumn,1000);
            child->setIcon(0, icon);
            child->setFlags(child->flags()&(~12));
            parentWidgetItem->addChild(child);
            // 查询子目录
            //sortingFileDir(child, fileInfo.filePath());
        }
        else if(fileInfo.suffix().contains("")){
            fileColumn.append(QString::number(fileInfo.size()));
            QTreeWidgetItem *child = new QTreeWidgetItem(fileColumn);
            child->setIcon(0, icon);
            child->setFlags(child->flags()&(~12));
            parentWidgetItem->addChild(child);
        }
    }
}

QTreeWidgetItem * TreeWidget::addChildToRoot(QTreeWidgetItem *root, QString path, int index)
{
    QFileIconProvider fileIcon;
    QFileInfo fileInfo(path);
    QStringList strlist;
    QIcon icon = fileIcon.icon(fileInfo);
    if(fileInfo.fileName().isEmpty())
        strlist<<path.replace("/","");
    else
        strlist<<fileInfo.fileName();
    QTreeWidgetItem * child = new QTreeWidgetItem(strlist,QTreeWidgetItem::UserType);
    child->setIcon(0, icon);
    root->insertChild(index,child);
    return root->child(index);
}

void TreeWidget::sortingChildFile(QTreeWidgetItem *curTreeWdtItem)
{
    if(curTreeWdtItem->type() == 1000 && curTreeWdtItem->childCount() == 0)
    {
        sortingFileDir(curTreeWdtItem, getCurItemPath(curTreeWdtItem));
    }
}

QString TreeWidget::getCurItemPath(QTreeWidgetItem *curTreeWdtItem)
{
    QString tempPath;
    if(curTreeWdtItem->parent() != rootTreeWdtItem)
    {
        tempPath = getCurItemPath(curTreeWdtItem->parent()) + "/" + curTreeWdtItem->text(0);
    }
    else
    {
        tempPath = rootPathList.at(rootTreeWdtItem->indexOfChild(curTreeWdtItem));
    }
    return tempPath;
}

void TreeWidget::removeChileTree(QTreeWidgetItem *curTreeWdtItem)
{
    QSettings iniSettings(INI_FILE_CONFIG,QSettings::IniFormat);
    iniSettings.setIniCodec(INI_FILE_CODE);
    QTreeWidgetItem *tempTreeItem = curTreeWdtItem;
    QString rmPath;
    int index;
    while(tempTreeItem->parent() != rootTreeWdtItem)
    {
        tempTreeItem = tempTreeItem->parent();
    }
    index = rootTreeWdtItem->indexOfChild(tempTreeItem);
    rmPath = rootPathList.at(index);
    rootPathList.removeAt(index);
    rootTreeWdtItem->takeChild(index);

    iniSettings.beginGroup("rootPath");
    QStringList allKeyList = iniSettings.allKeys();
    for(int i=0;i<allKeyList.count();i++)
    {
        QString pathTemp = iniSettings.value(allKeyList.at(i)).toString();
        if(pathTemp.compare(rmPath) == 0)
        {
            iniSettings.remove(allKeyList.at(i));
            iniSettings.endGroup();
            return;
        }
    }
    iniSettings.endGroup();

}

void TreeWidget::refreshChileTree(QTreeWidgetItem *curTreeWdtItem)
{
    QTreeWidgetItem *tempTreeItem = curTreeWdtItem;
    QString refreshPath;
    int index;
    while(tempTreeItem->parent() != rootTreeWdtItem)
    {
        tempTreeItem = tempTreeItem->parent();
    }
    index = rootTreeWdtItem->indexOfChild(tempTreeItem);
    refreshPath = rootPathList.at(index);
    if(QFileInfo(refreshPath).exists())
    {
        sortingFileDir(addChildToRoot(rootTreeWdtItem,refreshPath,index),refreshPath);
        rootTreeWdtItem->takeChild(index+1);
    }
    else
        removeChileTree(curTreeWdtItem);
}

void creatNewConnect(void)
{

}
