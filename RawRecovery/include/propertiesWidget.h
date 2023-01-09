#pragma once

#include "deviceitem.h"
#include <QTreeWidget>

class PropertiesWidget
{
    QString name_;
    QSize size_;

//static const QString Name_txt = "Name";
//static const QString Size_txt = "Size";
 
public:
    void changeDevice(IO::IODevicePtr device , QTreeWidget * treeWidget)
    {
        auto informationItem = treeWidget->topLevelItem(0);
		auto deviceNameItem = informationItem->child(0);
        auto deviceSizeItem = informationItem->child(1);

        auto devInfo = device->getDeviceInfo();
		deviceNameItem->setText(1, QString::fromWCharArray(devInfo.name.c_str()));
        deviceSizeItem->setText(1, QString::number(device->Size())  + " (bytes)");


    }

};
