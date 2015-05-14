#include "cappiteminfo.h"

CAPPItemInfo::CAPPItemInfo()
{
    this->resetItem();
}

void CAPPItemInfo::resetItem()
{
    this->appDesc.clear();
    this->appNumber = 0;
    this->appSettings.clear();
    this->appStatus = 0;
    this->checkCode.clear();
    this->editDate.clear();
    this->editType = 0;
    this->hideFlag = false;
    this->ICOPicPath.clear();
    this->itemCode.clear();
    this->logoPicPath.clear();
    this->delPicture.clear();
    this->owner.clear();
    this->srcName.clear();
    this->srcPath.clear();
    this->version.clear();
    this->dtaPath.clear();
    this->uiScriptPath.clear();
    this->delUiScriptName.clear();
}

int CAPPItemInfo::isItemExist(QList<CAPPItemInfo> *itemList)
{
    for(int i=0;i<itemList->count();i++)
    {
        if(itemList->at(i).appNumber == this->appNumber)
            return i;
    }
    return -1;
}
bool CAPPItemInfo::addItem(QList<CAPPItemInfo> *itemList)
{
    //¼ì²éÊÇ·ñÓÐ³åÍ»
    if(isItemExist(itemList) == -1)
    {
        itemList->insert(itemList->count(),*this);
        return true;
    }
    return false;
}
