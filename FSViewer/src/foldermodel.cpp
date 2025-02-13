#include "foldermodel.h"

#include <QStringList>
#include <memory>

	#include <QFileIconProvider>
	#include <QDate>
	#include <QTime>

#include <time.h>


FSModelIndex::FSModelIndex(const NodeEntry node_entry, FSModelIndex * parent)
	: currentEntry_(node_entry)
	, checked_(false)
	, parent_(parent)
{
	for (int iNode = 0 ; iNode < currentEntry_->count(); ++iNode)
	{
		NodeEntry tempNode(currentEntry_->child(iNode));
		if ( tempNode->type() == FileSystem::folder_type )
			FolderArray_.push_back( new FSModelIndex( tempNode , this));
		else
			FileArray_.push_back( new FSModelIndex( tempNode , this ));
	}

	std::sort(FolderArray_.begin(), FolderArray_.end(),GreaterName);
	std::sort(FileArray_.begin(), FileArray_.end(),GreaterName);
}
FSModelIndex::~FSModelIndex()
{
	releaseArray(FolderArray_);
	releaseArray(FileArray_);
	parent_ = nullptr;
}
void FSModelIndex::releaseArray(vector<FSModelIndex *> & arrayToDelete)
{
	auto delIter = arrayToDelete.begin();
	while ( delIter != arrayToDelete.end() )
	{
		FSModelIndex * pDeleteIndex = *delIter;
		if (!pDeleteIndex)
		{
			delete pDeleteIndex;
			pDeleteIndex = NULL;
		}
		++delIter;
	}
	arrayToDelete.clear();
}
int FSModelIndex::count() const
{
	return FolderArray_.size() + FileArray_.size();
}
NodeEntry FSModelIndex::getEntry() const
{
	return currentEntry_;
}
FSModelIndex * FSModelIndex::parent() const
{
	return parent_;
}
FSModelIndex*  FSModelIndex::getChild(const int rowNumber)
{
	if (rowNumber < FolderArray_.size() )
	{
		//assert(FolderArray_.size() != 0 );
		return FolderArray_.at(rowNumber);
	}
	int fileRow = rowNumber - FolderArray_.size();
	return (fileRow < FileArray_.size() ) ? 
		FileArray_.at(fileRow) 
		: NULL;
}
int FSModelIndex::row_index(const FSModelIndex * parent_index) const
{
	auto findIter = FolderArray_.begin();
	findIter = std::find(FolderArray_.begin(), FolderArray_.end(),parent_index);
	if ( findIter != FolderArray_.end() )
		return std::distance(FolderArray_.begin(),findIter);

	findIter = std::find(FileArray_.begin(), FileArray_.end(),parent_index);
	return ( findIter != FileArray_.end() ) ?
					(std::distance(FolderArray_.begin(),findIter) + FolderArray_.size() ) 
				:	-1;
}
void FSModelIndex::resetChilds()
{
	for (int iNode = 0 ; iNode < currentEntry_->count(); ++iNode)
	{
		NodeEntry tempNode(currentEntry_->child(iNode));
		if ( tempNode->type() == FileSystem::folder_type )
			FolderArray_.push_back( new FSModelIndex( tempNode , this));
		else
			FileArray_.push_back( new FSModelIndex( tempNode , this ));
	}
	std::sort(FolderArray_.begin(), FolderArray_.end(),GreaterName);
	std::sort(FileArray_.begin(), FileArray_.end(),GreaterName);
}
bool FSModelIndex::isChecked() const
{
	return checked_;
}
void FSModelIndex::setChecked(const int checked )
{
	checked_ = checked;
}
//bool FSModelIndex::operator < ( const FSModelIndex & second)
//{
//	return ( this->currentEntry_->name() < second->getEntry()->name() );
//}
void FSModelIndex::sortByName( const int order)
{
	std::sort(FolderArray_.begin(), FolderArray_.end(),(order==0) ? GreaterName : LessName);
	std::sort(FileArray_.begin(), FileArray_.end(),(order==0) ? GreaterName : LessName);
}

bool GreaterName(const FSModelIndex * first, const FSModelIndex * second)
{
	return ( first->getEntry()->name() < second->getEntry()->name() );
}
bool LessName(const FSModelIndex * first, const FSModelIndex * second)
{
	return ( first->getEntry()->name() >= second->getEntry()->name() );
}
FolderModel::FolderModel(QObject *parent, const DirectoryEntry & RootDirectory)
	: QAbstractItemModel(parent)
	, RootFolder_(RootDirectory)
{
	if ( !RootFolder_ )
		RootFolder_ = DirectoryEntry( new DirectoryNode("MAIN ROOT (HEADER)") );

	sort_order = Qt::AscendingOrder;

}


FolderModel::FolderModel(QObject *parent, FSModelIndex * _root_folder)
	: QAbstractItemModel(parent)
	, RootIndex_(_root_folder)
{
	sort_order = Qt::AscendingOrder;
}

FolderModel::~FolderModel()
{
	if (RootIndex_)
	{
		delete RootIndex_;
		RootIndex_ = nullptr;
	}
}
int	FolderModel::columnCount ( const QModelIndex & parent ) const 
{
	return parent.isValid() && parent.column() != 0 ? 0 : sizeof( FSheader ) + 1;
}
QVariant FolderModel::data ( const QModelIndex & index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	//if (role != Qt::DisplayRole)
	//	return QVariant();

	if ( FSModelIndex * model_index = static_cast < FSModelIndex *>
					( index.internalPointer() ) )
	{
		if (NodeEntry node_entry = model_index->getEntry())
		{
			if (role == Qt::DisplayRole)
			{
				switch ( index.column() )
				{
				case header_name:
					return QString::fromStdWString(node_entry->name().c_str());

				case header_size:
					if (node_entry->type() == FileSystem::folder_type)
						return QString(tr(""));
					return QString("%1 bytes").arg(node_entry->size());
				case header_created:
					if ( FileSystem::EntryTimes * entryData = node_entry->getDates() )
						return dataToString(entryData->created_data()) + " " + timeToString(entryData->created_time(),entryData->created_time_ms());
					break;
				case header_modified:
					if ( FileSystem::EntryTimes * entryData = node_entry->getDates() )
						return dataToString(entryData->modified_data()) + " " + timeToString(entryData->modified_time());
					break;
				case header_accessed:
					if ( FileSystem::EntryTimes * entryData = node_entry->getDates() )
						return dataToString(entryData->accessed_data()) + " " + timeToString(entryData->accessed_time());
					break;
				}
			}
			else
			{
				if (index.column() == 0)
				{
					if (role == Qt::CheckStateRole)
						return static_cast<int>(model_index->isChecked() ? Qt::Checked 
						: Qt::Unchecked);
					else
						if (role == Qt::DecorationRole)
						{
							return (node_entry->type() == FileSystem::folder_type) ?
								iconProvider_.icon(QFileIconProvider::Folder)
								: iconProvider_.icon(QFileIconProvider::File);

						}
				}
			}
		}
		
	}
	//if (IVirtualNode * dir_entry = static_cast < IVirtualNode * >
	//				( index.internalPointer() ) )
	//{
	//	if (role == Qt::DisplayRole)
	//	{
	//		switch ( index.column() )
	//		{
	//		case header_name:
	//			return QString::fromStdWString(dir_entry->name().c_str());

	//		case header_size:
	//			if (dir_entry->type() == FileSystem::folder_type)
	//				return QString(tr(""));
	//			return QString("%1 bytes").arg(dir_entry->size());
	//		case header_created:
	//			if ( FileSystem::EntryTimes * entryData = dir_entry->getDates() )
	//				return dataToString(entryData->created_data()) + " " + timeToString(entryData->created_time(),entryData->created_time_ms());
	//			break;
	//		case header_modified:
	//			if ( FileSystem::EntryTimes * entryData = dir_entry->getDates() )
	//				return dataToString(entryData->modified_data()) + " " + timeToString(entryData->modified_time());
	//			break;
	//		case header_accessed:
	//			if ( FileSystem::EntryTimes * entryData = dir_entry->getDates() )
	//				return dataToString(entryData->accessed_data()) + " " + timeToString(entryData->accessed_time());
	//			break;
	//		}
	//	}
	//	else
	//	{
	//		if (index.column() == 0)
	//		{
	//			if (role == Qt::CheckStateRole)
	//				return static_cast<int>(dir_entry->isChecked() ? Qt::Checked 
	//																: Qt::Unchecked);
	//			else
	//			if (role == Qt::DecorationRole)
	//			{
	//				return (dir_entry->type() == FileSystem::folder_type) ?
	//							iconProvider_.icon(QFileIconProvider::Folder)
	//							: iconProvider_.icon(QFileIconProvider::File);

	//			}
	//		}
	//	}
	//}

	return QVariant();
}
Qt::ItemFlags FolderModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags theFlags = QAbstractItemModel::flags(index);
	if (index.isValid())
	{
		theFlags |= Qt::ItemIsUserCheckable ;
		//theFlags |= Qt::DecorationRole;
	}

	return theFlags;
}
QVariant FolderModel::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	switch (section)
	{
		case header_name: return tr("Name");
		case header_size: return tr("Size");
		case header_created: return tr("Created");
		case header_modified : return tr("Modified");
		case header_accessed : return tr("Accessed");
	}

	return QVariant();
}
int	FolderModel::rowCount ( const QModelIndex & parent ) const
{
	if ( parent.isValid() && parent.column() != 0)
		return 0;
	if (parent.column() > 0)
		return 0;

	FSModelIndex * parent_index = EntryFromIndex(parent);

	int row = (parent_index) ? parent_index->count() : 0 ;
	return row;

}
QModelIndex	FolderModel::index ( int row, int column, const QModelIndex & parent  ) const
{
	if ( (RootIndex_ == NULL) || row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent) )
		return QModelIndex();

	if ( ! hasIndex(row,column, parent) )
		return QModelIndex();

	FSModelIndex * parent_index = EntryFromIndex(parent);

	FSModelIndex * child_index  = parent_index->getChild(row);
	if (child_index)
		return createIndex( row ,column, child_index );

	return QModelIndex();
}
QModelIndex	FolderModel::parent ( const QModelIndex & index ) const 
{
	if ( !index.isValid() )	
		return QModelIndex();

	if ( FSModelIndex * child_index = EntryFromIndex(index) )
	{
		if ( FSModelIndex * parent_index = child_index->parent() )
		{
			if ( parent_index == RootIndex_)
				return QModelIndex();

			if ( FSModelIndex * grandParent_index = parent_index->parent() )
				return createIndex( grandParent_index->row_index(parent_index), 0 , parent_index);
		}
	}
	return QModelIndex();

	//if ( IVirtualNode * childFolder = EntryFromIndex(index))
	//{
	//	if ( DirectoryNode * parentFolder = childFolder->parent() )
	//	{
	//		if ( parentFolder == RootFolder_.get())
	//			return QModelIndex();
	//		
	//		if ( DirectoryNode * grandParentFolder = parentFolder->parent() )
	//		{
	//			return createIndex(grandParentFolder->indexNumber(parentFolder), 0 , parentFolder);
	//		}
	//	}
	//}
	//Directory * child_entry = static_cast < Directory * > 
	//				( index.internalPointer() );

	//Directory * parent_entry = child_entry->GetParent();

	//if (parent_entry)
	//{
	//if ( parent_entry == RootFolder_)
	//	return QModelIndex();

	//int iRow =  parent_entry->IndexNumber( child_entry );
	//return createIndex(iRow,0,parent_entry);
	//}
	//else
	//	return QModelIndex();




	//if (child_entry == parent_entry)
	//	return QModelIndex();

	//if (Directory *childEntry = EntryFromIndex(index))
	//{
	//	Directory *parentEntry = childEntry->GetParent();

	//	if (parentEntry == RootFolder_)
	//		return QModelIndex();

	//	if ( parentEntry )
	//	{
	//		int iRow =  parentEntry->IndexNumber(DirectoryEntry(childEntry));
	//		return createIndex( iRow, 0 , parentEntry );
	//	}

	//	//Directory * grandParentEntry = parentEntry->GetParent();
	//	//if (grandParentEntry)
	//	//{
	//	//	DirectoryEntry dir_entry(parentEntry);
	//	//	int row = grandParentEntry->IndexNumber(dir_entry);
	//	//	return createIndex(row,0,parentEntry);
	//	//}
	//}

	//return createIndex(parent_entry->IndexNumber(DirectoryEntry(child_entry)), 0 );
	//return QModelIndex();

}
bool FolderModel::insertRows(int row, int count, const QModelIndex &parent)
{
	//if ( !parent.isValid() )
	//	return false;

	//if ( !RootFolder_ )
	//	RootFolder_ = DirectoryEntry ( new DirectoryNode("Header:") );

	//DirectoryNode * pDirEntry = static_cast<DirectoryNode*> 
	//			( EntryFromIndex(parent) );

	//
	//beginInsertRows(parent,row,row + count + 1);
	//for (int iRow = 0; iRow < count; ++iRow)
	//{
	//	pDirEntry->add_file( new FileNode("file1"));
	//}
	//endInsertRows();
	//emit dataChanged(parent,parent);
	return true;	 
}
FSModelIndex* FolderModel::EntryFromIndex(const QModelIndex & qIndex) const
{
	if ( qIndex.isValid() )
	{

		//NodeEntry dir_entry1 = std::tr1::static_pointer_cast < IVirtualNode > 
		//						( qIndex.internalPointer() );

		FSModelIndex * model_index = static_cast < FSModelIndex *>
						( qIndex.internalPointer() ) ;
		//IVirtualNode * dir_entry = static_cast < IVirtualNode *>
		//				( qIndex.internalPointer() );

		//if (dir_entry)
			return model_index;
	}

	//return RootFolder_.get();
	return RootIndex_;
}

bool FolderModel::setData(const QModelIndex &index, const QVariant &value,
	int role)
{
	if ( !index.isValid() )
		return false;

	if ( FSModelIndex * model_index = EntryFromIndex(index))
	{
		if (role == Qt::CheckStateRole)
			model_index->setChecked(value.toBool());
		emit dataChanged(index,index);
		return true;
	}
	return false;
}
void FolderModel::Update(const QModelIndex & index)
{
	emit dataChanged(index,index);
}

QString FolderModel::dataToString( const WORD data_format) const
{
	BYTE day = data_format & 0x001F;
	BYTE mounth = ( data_format & 0x00F0 ) >> 5;
	WORD year = (( data_format & 0xFF00 ) >> 9) + 1980;
	//QDate dos_date( day, mounth, 12 );
	//bool bValid = dos_date.isValid();

	return ( QDate::isValid(year,mounth,day) ) ? QString("%1.%2.%3").arg(day).arg(mounth).arg(year) : QString("");
	//return ( dos_date.isValid() ) ? dos_date.toString(Qt::LocalDate) : QString("");
}
QString FolderModel::timeToString( const WORD time_format, const BYTE milisec) const
{
	BYTE time_sec = time_format & 0x001F;
	time_sec = time_sec >> 1;
	if (milisec > 100)
		++time_sec;
	BYTE time_minutes = (time_format & 0x7E0) >> 5;
	BYTE time_hours = time_format >> 11;
	QTime dos_time(time_hours,time_minutes,time_sec);
	bool bTimeValid = dos_time.isValid();

	return ( dos_time.isValid() ) ? dos_time.toString() : QString();
}

void FolderModel::sort ( int column, Qt::SortOrder order )
{
	if (order == sort_order)
		return;

	sort_order = order;
	emit layoutAboutToBeChanged();
	QModelIndexList oldList = persistentIndexList();
	if (oldList.size() == 0)
		return;
	for (int nIndex = 0 ; nIndex < oldList.count() ; ++nIndex)
	{
	QModelIndex parentIndex = oldList.at(nIndex);
	//
	if (FSModelIndex * parent_index = static_cast < FSModelIndex *>
					( parentIndex.internalPointer() ) )
	{
		parent_index->sortByName(order);

	// need to own holder !!!!!!!

	//QModelIndexList newList;
	//for (int nIndex = 0 ; nIndex < oldList.count() ; ++nIndex)
	//{
	//	QModelIndex old_index = oldList.at(nIndex);
	//	FSModelIndex * pChild = parent_index->getChild(nIndex);
	//	QModelIndex new_index = createIndex(old_index.row(),column, pChild);
	//	new_index = new_index.sibling(new_index.row(), column);
	//	newList.append( new_index );
	//}
	
	//changePersistentIndexList(oldList,newList);
	}
	}
	emit layoutChanged();


}

//
//FS_Model::FS_Model(QObject *parent, const DirectoryEntry & rootFolder)
//	: FolderModel(parent,rootFolder)
//{
//
//}
//
//QVariant FS_Model::headerData(int section, Qt::Orientation orientation, int role ) const
//{
//	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
//	switch (section)
//	{
//	case 0: return tr("Name");
//	case 1: return tr("Size");
//	}
//}
//
//QVariant FS_Model::data ( const QModelIndex & index, int role ) const
//{
//	if (!index.isValid())
//		return QVariant();
//
//	//if (role != Qt::DisplayRole)
//	//	return QVariant();
//
//	if (IVirtualNode * dir_entry = static_cast < IVirtualNode * >
//		( index.internalPointer() ) )
//	{
//		if (role == Qt::DisplayRole)
//		{
//			QString strValue;
//			switch ( index.column() )
//			{
//			case 0:
//				strValue = dir_entry->name().c_str();
//				return strValue;
//
//			case 1:
//				if (dir_entry->type() == FileSystem::folder_type)
//					return QString(tr(""));
//				return (long)dir_entry->size();
//			}
//		}
//		else
//		{
//			if (index.column() == 0)
//			{
//				if (role == Qt::CheckStateRole)
//					return static_cast<int>(dir_entry->isChecked() ? Qt::Checked 
//					: Qt::Unchecked);
//				else
//					if (role == Qt::DecorationRole)
//					{
//						QFileIconProvider iconProvider;
//						return (dir_entry->type() == FileSystem::folder_type) ?
//							iconProvider.icon(QFileIconProvider::Folder)
//							: iconProvider.icon(QFileIconProvider::File);
//
//					}
//			}
//		}
//	}
//
//	return QVariant();
//
//}
//
//int	FS_Model::columnCount ( const QModelIndex & parent  ) const 
//{
//	return parent.isValid() && parent.column() != 0 ? 0 : 1;
//}pppp