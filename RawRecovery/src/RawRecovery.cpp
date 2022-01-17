#include "RawRecovery.h"

#include "json/jsonreader.h"
#include "json/signaturereader.h"

#include "io/iodevice.h"
#include "deviceitem.h"
#include "signatureitem.h"

#include <QMessageBox>


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
				}
			}
		}
	}
}
