#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FSViewer.h"


class FSTreeModel;
class FSTableModel;


class FolderModel;
class FS_Model;
class TableIndex;
class TreeIndex;

#include "fs/AbstractFS.h"
#include "io/file.h"


#include "ui_RecoverDialog.h"
#include "recoverdialog.h"

class FSViewer : public QMainWindow
{
	Q_OBJECT

public:
	typedef std::shared_ptr<QDialog> DialogPtr;


	FSViewer(QWidget *parent = Q_NULLPTR);
	~FSViewer();
	void RecoverFile(const QString & folder_path, const FileSystem::FileEntry & file_entry);
	void RecoverRAWFile(const QString& folder_path, const FileSystem::FileEntry& file_entry);
	void RecoverFolder(const QString & folder_path, FileSystem::DirectoryEntry & folder_entry);
	void RecoverFolder(const QString & folder_path, TableIndex * FolderIndex);


signals:
	void recoverFileSignal(const FileSystem::NodeEntry & node_entry);
protected slots:
	void OnTreeViewClicked(QModelIndex modelIndex);
	void DoubleClickedTableView(QModelIndex modelIndex);
	void OnTableContextMenu(const QPoint & point_pos);

	void TableViewCheckedFolders(const QString & node_path, Qt::CheckState checkState);
	void TableViewParentFolders(Qt::CheckState checkState);
	void TreeViewChecked(Qt::CheckState checkState);

	void RecoverySelected();
private:
	void readFolders(TableIndex * FolderIndex);
	void read_and_check(TreeIndex * checkIndex, const Qt::CheckState checkState);
private:
	TreeIndex * toTreeIndex(const QModelIndex & treeIndex);
	TableIndex * toTableIndex(const QModelIndex & tableIndex);

	Ui::FSViewerClass ui;
	FSTreeModel * tree_model_ = nullptr;
	FSTableModel * table_model_ = nullptr;

	//	Dialogs
	Ui::RecoverDialog RecoverUi_;
	RecoverDialog * RecoverDialog_ = nullptr;
	// Actions
	QAction *recFileAction;

	FolderModel * pFSmodel;
	FS_Model * pFullMode;

	IO::FilePtr source_file_;

	FileSystem::AbstractFS abstract_fs;
};
