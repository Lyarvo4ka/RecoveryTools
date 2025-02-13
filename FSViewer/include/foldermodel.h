#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QList>
#include <QVariant>


#include <QAbstractItemModel>
#include <QFileIconProvider>

#include "fs/DirectoryTree.h"
#include "fs/AbstractFS.h"

using FileSystem::DirectoryEntry;
using FileSystem::DirectoryNode;
using FileSystem::NodeEntry;
using FileSystem::FileNode;
using FileSystem::IVirtualNode;

#include <vector>
using std::vector;
#include <assert.h>




//class TableIndex
//	: public BaseIndex
//{
//public:
//	TableIndex();
//	~TableIndex();
//	int count() const;
//	TableIndex * parent() const;
//	TableIndex * getChild(const int rowNumber) const;
//	int row_index(const TreeIndex * parent_index) const;
//};



class FSModelIndex 
{
public: 
	FSModelIndex(const NodeEntry node_entry, FSModelIndex * parent = 0);
	//IndexFS(const NodeEntry node_entry);
	//IndexFS(IVirtualNode * node_entry);
	~FSModelIndex();
	NodeEntry getEntry() const;
	int count() const;
	FSModelIndex * parent() const;
	FSModelIndex*  getChild(const int rowNumber);
	int row_index(const FSModelIndex * parent_index) const;
	void resetChilds();
	bool isChecked() const;
	void setChecked(const int checked = true);
	//bool operator() (const FSModelIndex & first, const FSModelIndex & second);
	void sortByName( const int order = 0);
private:
	void releaseArray(vector<FSModelIndex *> & arrayToDelete);

private:
	NodeEntry currentEntry_;
	FSModelIndex * parent_;

	bool checked_;

	vector< FSModelIndex* > FolderArray_;
	vector< FSModelIndex* > FileArray_;

};


bool GreaterName(const FSModelIndex * first, const FSModelIndex * second);
bool LessName(const FSModelIndex * first, const FSModelIndex * second);

//template < class T>

//class CompareByName
//	: public std::binary_function< FSModelIndex, FSModelIndex, bool>
//{
//public:
//	bool operator() ( const FSModelIndex * first, const FSModelIndex * second) const
//	{
//		//return ( node_entry->name().compare( compareStr ) == 0 ) ? true : false;
//		//NodeEntry node_entry = model_index->getEntry();
//		return ( first->getEntry()->name() == second->getEntry()->name() );
//	}
//};


class FolderModel : public QAbstractItemModel
{

	Q_OBJECT


public:
	static enum FSheader { header_name = 0, header_size, header_created, header_modified, header_accessed };

	FolderModel(QObject *parent, const DirectoryEntry & RootDirectory);
	FolderModel(QObject *parent, FSModelIndex * _root_folder);

	~FolderModel();
	
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	virtual int	columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
	virtual int	rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual QModelIndex	index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	virtual QModelIndex	parent ( const QModelIndex & index ) const ;
	virtual bool insertRows(int row, int count,	const QModelIndex &parent=QModelIndex());
	//bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const

	bool setHeaderData(int, Qt::Orientation, const QVariant&,
		int=Qt::EditRole) { return false; }

	bool setData(const QModelIndex &index, const QVariant &value,
		int role=Qt::EditRole);

	void Update(const QModelIndex & index);
	QString dataToString( const WORD data_format) const;
	QString timeToString( const WORD time_format, const BYTE milisec = 0) const;

	virtual void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

protected:
	FSModelIndex * EntryFromIndex(const QModelIndex & qIndex) const;

private:
	Qt::SortOrder sort_order;
	bool DirectoryFree_;
	DirectoryEntry RootFolder_;
	QFileIconProvider iconProvider_;
	FSModelIndex * RootIndex_;
};
//
//class FS_Model :
//	public FolderModel
//{
//public:
//	FS_Model(QObject *parent, const DirectoryEntry & rootFolder);
//
//	QVariant headerData(int section, Qt::Orientation orientation,
//		int role = Qt::DisplayRole) const;
//
//	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
//	virtual int	columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
//
//
//};



#endif // FOLDERMODEL_H
