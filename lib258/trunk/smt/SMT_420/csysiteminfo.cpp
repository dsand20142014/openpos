#include "csysiteminfo.h"

CSYSItemInfo::CSYSItemInfo()
{
    this->resetItem();
}

void CSYSItemInfo::resetItem()
{
    this->type = 0;
    this->srcName.clear();
    this->srcPath.clear();
    this->status = 0;
    this->version = "0";
    this->dtaPath.clear();
    this->insmod = 1;
    this->dbg = 0;
}

int CSYSItemInfo::getItemCount(QList<CSYSItemInfo> *itemList)
{
    int count = 0;
    for(int i=0;i<itemList->count();i++)
    {
        if(itemList->at(i).type == this->type)
            count++;
    }
    return count;
}

bool CSYSItemInfo::addItem(QList<CSYSItemInfo> *itemList)
{
    int index = 0;
    //ºÏ≤È «∑Ò”–≥ÂÕª
    if(isItemExist(itemList) == -1)
    {
        for(int i=0;i<itemList->count();i++)
        {
            if(itemList->at(i).type <= this->type)
            {
                index++;
            }
            else
                break;
        }
        itemList->insert(index,*this);
        return true;
    }
    return false;
}

int CSYSItemInfo::isItemExist(QList<CSYSItemInfo> *itemList)
{
    for(int i=0;i<itemList->count();i++)
    {
        if(itemList->at(i).type == this->type &&
                itemList->at(i).srcName.compare(this->srcName) == 0)
        {
            if(type == LIB && this->version.compare(itemList->at(i).version) != 0)
                continue;
            return i;
        }
    }
    return -1;
}


bool CSYSItemInfo::replaceItem(int index, QList<CSYSItemInfo> *itemList)
{
    if(index < itemList->count() && this->type == itemList->at(index).type)
    {
        itemList->replace(index,*this);
        return true;
    }
    return false;
}

bool CSYSItemInfo::removeItem(int index, QList<CSYSItemInfo> *itemList)
{
    if(index < itemList->count() && this->type == itemList->at(index).type)
    {
        itemList->takeAt(index);
        return true;
    }
    return false;
}
