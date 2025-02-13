#ifndef FSTREEMODEL_H
#define FSTREEMODEL_H

 #include <QAbstractItemModel>
 #include <QVariant>
 #include <QFileIconProvider>

#include "BaseIndex.h"
#include "fs/AbstractFS.h"

using FileSystem::DirectoryEntry;
using FileSystem::DirectoryNode;
using FileSystem::NodeEntry;
using FileSystem::FileNode;
using FileSystem::IVirtualNode;

#include <assert.h>

class TreeIndex
	: public BaseIndex
{
	typedef map<wstring,TreeIndex *> MapFolder;
	typedef pair<wstring,TreeIndex *> PairFolder;
public:
	TreeIndex();
	TreeIndex( const DirectoryEntry & directory);
	~TreeIndex();
	void clear();
	void setEntry( const DirectoryEntry & directory);
	inline DirectoryEntry getEntry() const;
	inline int count() const;
	void setParent(TreeIndex * parent);
	inline TreeIndex * parent() const;
	TreeIndex * getChild(const int rowNumber) const;
	int row_index(const TreeIndex * parent_index) const;
	void resetChilds();
	void insertChild( const DirectoryEntry & directory_entry);
	wstring getRootPath();
	TreeIndex * index_node(const wstring & folder_path);
	void updateCheckChilds(const Qt::CheckState checked);
	void updateCheckParents(const Qt::CheckState checked);
	Qt::CheckState checkFormChilds(const Qt::CheckState current_state);
private:
	DirectoryEntry directory_;
	TreeIndex * parent_;
	MapFolder folders_;
};

class FSTreeModel 
	: public QAbstractItemModel
{
	Q_OBJECT

public:
	FSTreeModel( QObject *parent, TreeIndex * RootIndex);
	~FSTreeModel();

	Qt::ItemFlags flags(const QModelIndex &index) const ;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const ;
	virtual int	columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
	virtual int	rowCount ( const QModelIndex & parent = QModelIndex() ) const ;
	virtual QModelIndex	index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	virtual QModelIndex	parent ( const QModelIndex & index ) const ;
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);

	void update_view(const QModelIndex & index);

	TreeIndex * indexByPath( const wstring & folder_path );
	QModelIndex indexFromPath( const wstring & folder_path );

	void updateCheckState(const QModelIndex & currentIndex, Qt::CheckState checkState);
signals:
	void updateTableState(const Qt::CheckState checkState);

protected:
	TreeIndex * fromIndex( const QModelIndex & qIndex) const;
private:
	TreeIndex * RootIndex_;
	QFileIconProvider iconProvider_;
};

#endif // FSTREEMODEL_H
