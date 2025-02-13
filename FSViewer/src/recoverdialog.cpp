#include "recoverdialog.h"

 #include <QFileDialog>

RecoverDialog::RecoverDialog(QWidget *parent, Ui::RecoverDialog * pRecoveryDialog)
	: QDialog(parent)
	, RecoveryDialog_(pRecoveryDialog)
{

}
RecoverDialog::~RecoverDialog()
{
	qDebug("Destructor RecoverDialog");
}
void RecoverDialog::setFolderPath(const QString & folderPath)
{
	folderPath_ = folderPath;
	RecoveryDialog_->folderPathEdit->setText(folderPath_);
}
QString RecoverDialog::getFolderPath() const
{
	return folderPath_;
}
void RecoverDialog::OpenFolderDialog()
{
	setFolderPath( QFileDialog::getExistingDirectory(this, tr("Select Folder"),
				   folderPath_,
				   QFileDialog::ShowDirsOnly
				   | QFileDialog::DontResolveSymlinks)
				 );
}
void RecoverDialog::TextChanged()
{
	folderPath_ = RecoveryDialog_->folderPathEdit->text();
}
