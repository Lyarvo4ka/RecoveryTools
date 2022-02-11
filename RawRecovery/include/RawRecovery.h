#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RawRecovery.h"

class RawRecovery : public QMainWindow
{
	Q_OBJECT

public:
	RawRecovery(QWidget *parent = Q_NULLPTR);

protected slots:
	void OnDeviceContextMenu(const QPoint& point_pos);
	void getSelectedDeviceIndex(const QModelIndex&);

private:
	Ui::RawRecoveryClass ui;
	QAction* contectMenu_;
	QTreeWidgetItem* informationItem_tmp;
};
