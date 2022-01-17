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

private:
	Ui::RawRecoveryClass ui;
	QAction* contectMenu_;
};
