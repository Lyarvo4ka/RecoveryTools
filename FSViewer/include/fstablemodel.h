#ifndef FSTABLEMODEL_H
#define FSTABLEMODEL_H

#include <functional>

#include <QAbstractItemModel>
 #include <QFileIconProvider>

#include "BaseIndex.h"
#include "fs/AbstractFS.h"
using FileSystem::DirectoryEntry;
using FileSystem::DirectoryNode;
using FileSystem::NodeEntry;
using FileSystem::FileNode;
using FileSystem::IVirtualNode;

class TableIndex
	: public BaseIndex
{
public:
	TableIndex();
	TableIndex(const NodeEntry & node_entry);
	~TableIndex();
	void clear();
	void setEntry( const NodeEntry & node_entry);
	inline NodeEntry getEntry() const;
	inline int count() const;
	void setParent(TableIndex * parent);
	inline TableIndex * parent() const;	
	TableIndex * getChild(const int rowNumber) const;
	int row_index(const TableIndex * parent_index) const;
	void resetChilds();
	void insertChild( const NodeEntry & node_entry);
	TableIndex * index_node(const wstring & folder_path);
	void sortByName( const int order = 0);
	wstring getRootPath();
	void updateCheckChilds(const Qt::CheckState checked);
	void updateCheckParents(const Qt::CheckState checked);

	Qt::CheckState checkFormChilds(const Qt::CheckState current_state);
private:
	TableIndex * parent_;
	NodeEntry node_entry_;
	vector< TableIndex* > FolderArray_;
	vector< TableIndex* > FileArray_;
};

Qt::CheckState getStateCheckState( const vector< TableIndex* > & arrayChilds, Qt::CheckState checked);

class TableFindByName
	//: public std::binary_function< TableIndex *, wstring , bool >
{
public:
	bool operator() ( const TableIndex * first, const wstring & name) const
	{
		//return ( first->getEntry()->name() == second->getEntry()->name() );
		return ( first->getEntry()->name().compare(name) == 0 );
	}
};

bool GreaterName(const TableIndex * first, const TableIndex * second);
bool LessName(const TableIndex * first, const TableIndex * second);

class FSTableModel : public QAbstractItemModel
{
	Q_OBJECT
	
public:
	static enum FSheader { header_name = 0, header_size, header_created, header_modified, header_accessed };

	FSTableModel(QObject *parent, TableIndex * RootIndex);
	~FSTableModel();

	Qt::ItemFlags flags(const QModelIndex &index) const ;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const ;
	virtual int	columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
	virtual int	rowCount ( const QModelIndex & parent = QModelIndex() ) const ;
	virtual QModelIndex	index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	virtual QModelIndex	parent ( const QModelIndex & index ) const ;
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);


	QString dataToString( const WORD data_format) const;
	QString timeToString( const WORD time_format, const BYTE milisec = 0) const;
	//void setRootPath(const wstring & rootPath);
	TableIndex * indexByPath(const wstring & folder_path);
	QModelIndex indexFromPath( const wstring & folder_path);
	virtual void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );
signals:
	void updateTreeState(const QString & rootPath, Qt::CheckState);
	void updateParentState( Qt::CheckState);
protected:
	TableIndex * fromIndex( const QModelIndex & qIndex) const;
private:
	Qt::SortOrder sort_order;
	TableIndex * RootIndex_;
	QFileIconProvider iconProvider_;
};

#endif // FSTABLEMODEL_H
