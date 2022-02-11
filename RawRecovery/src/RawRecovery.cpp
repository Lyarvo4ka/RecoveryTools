#include "RawRecovery.h"

#include "json/jsonreader.h"
#include "json/signaturereader.h"

#include "io/iodevice.h"
#include "deviceitem.h"
#include "signatureitem.h"

#include <QMessageBox>
#include <QTreeWidget>

#include "propertiesWidget.h"

RawRecovery::RawRecovery(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	auto physical_drives = IO::ReadPhysicalDrives();
	auto rootAdapter = std::make_unique< RootAdapter>();

	auto root_item = new DeviceItem(std::move(rootAdapter));
	for (uint32_t i = 0; i < physical_drives.getSize(); ++i)
	{
		auto physicalAdapter = std::make_unique<PhysicalAdapter>(physical_drives.index(i));
		auto disk_item = new DeviceItem(std::move(physicalAdapter ), root_item);
		root_item->appendChild(disk_item);
	}
	auto device_model = new DeviceModel(root_item, this);
	ui.treeView->setModel(device_model);
	for (auto iColumn = 0; iColumn < device_model->columnCount(); ++iColumn)
	{
		ui.treeView->resizeColumnToContents(iColumn);
	}
	contectMenu_ = new QAction(tr("&TEST SELECT DEVICE!!!!"), this);
	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
//	ui.treeView->addAction(contectMenu_);

	connect(ui.treeView, &QWidget::customContextMenuRequested, this, &RawRecovery::OnDeviceContextMenu);
	connect(ui.treeView, &QTreeView::clicked, this, &RawRecovery::getSelectedDeviceIndex);
//	QObject::connect(treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(getSelectedDeviceIndex(const QModelIndex&)));

	auto folder_path = LR"(c:\soft\!MyPrograms\SignatureTestConsole\signatures\)";
	SignatureReader singReader;
	singReader.loadAllSignatures(folder_path, L".json");
	auto listFileStruct = singReader.getAllSignatures();

	auto sign_root = new SignatureItem(std::make_unique<CategoryFolderAdapter>());

	for (auto theFileStruct : listFileStruct)
	{

		auto parentItem = sign_root->findWithName(theFileStruct.category);
		if (parentItem == nullptr)
		{
			auto categoryAdapter = std::make_unique< CategoryFolderAdapter>();
			categoryAdapter->setName(theFileStruct.category);
			parentItem = new SignatureItem(std::move(categoryAdapter), sign_root);
			sign_root->appendChild(parentItem);
		}
		auto fileStructPtr = toUniqueFileStruct(theFileStruct);
		auto sign_adapter = std::make_unique<SignatureItemAdapter>(*fileStructPtr);
		SignatureItem* sign_item = new SignatureItem(std::move(sign_adapter), parentItem);
		parentItem->appendChild(sign_item);
			
	}

	auto pSignatureTreeModel = new SignatureTreeModel(sign_root, this);
	ui.signatureTree->setModel(pSignatureTreeModel);

	QTreeWidget * propertyWidget = ui.propertyWidget;
	propertyWidget->setColumnCount(2);
	informationItem_tmp = new QTreeWidgetItem(propertyWidget);
	informationItem_tmp->setText(0, "Information");


	QTreeWidgetItem* treeItemName = new QTreeWidgetItem();
	treeItemName->setText(0, "Name");
	treeItemName->setText(1, "No device selected");

	QTreeWidgetItem* treeItemSize = new QTreeWidgetItem();
	treeItemSize->setText(0, "Size");
	treeItemSize->setText(1, "No device selected");

	informationItem_tmp->addChild(treeItemName);
	informationItem_tmp->addChild(treeItemSize);

	informationItem_tmp->setExpanded(true);


}

void RawRecovery::OnDeviceContextMenu(const QPoint& point_pos)
{
	QModelIndex device_cell = ui.treeView->indexAt(point_pos);
	if (device_cell.isValid())
	{
		auto seclected_index = static_cast<DeviceItem*>(device_cell.internalPointer());
		if (seclected_index)
		{
			auto selected_device = seclected_index->getAdapter()->createDevice();
			if (selected_device)
			{
				auto disk_device = std::dynamic_pointer_cast<IO::DiskDevice>(selected_device);
				if (disk_device)
				{
					QMessageBox msgBox;
					auto devInfo = disk_device->getDeviceInfo();
					QString msg_string = "Selected " + QString::fromWCharArray(devInfo.name.c_str()) + "ID = " + QString::number(devInfo.deviceID);
					msgBox.setText(msg_string);
					msgBox.exec();

					//auto deviceNameItem = informationItem_tmp->child(0);
					//deviceNameItem->setText(1, QString::fromWCharArray(devInfo.name.c_str()));
				}
			}
		}
	}
}

void RawRecovery::getSelectedDeviceIndex(const QModelIndex& device_cell)
{
	if (device_cell.isValid())
	{
		auto seclected_index = static_cast<DeviceItem*>(device_cell.internalPointer());
		if (seclected_index)
		{
			auto selected_device = seclected_index->getAdapter()->createDevice();
			if (selected_device)
			{
				PropertiesWidget widget;
				widget.changeDevice(selected_device, ui.propertyWidget);
			}
		}
	}
}