#ifndef RECOVERDIALOG_H
#define RECOVERDIALOG_H

#include <QDialog>
#include "ui_RecoverDialog.h"

class RecoverDialog : public QDialog
{
	Q_OBJECT

public:
	RecoverDialog(QWidget *parent, Ui::RecoverDialog * pRecoveryDialog);
	~RecoverDialog();

	void setFolderPath(const QString & folderPath);
	QString getFolderPath() const;

protected slots:
	void OpenFolderDialog();
	void TextChanged();
private:
	QString folderPath_;
	 Ui::RecoverDialog * RecoveryDialog_;
};

#endif // RECOVERDIALOG_H
