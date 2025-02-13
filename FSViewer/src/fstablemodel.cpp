#include "fstablemodel.h"

#include <assert.h>

 #include <QDate>
 #include <QTime>
TableIndex::TableIndex()
{

}
TableIndex::TableIndex(const NodeEntry & node_entry)
	: node_entry_(node_entry)
	, parent_(nullptr)
{
	resetChilds();
}
TableIndex::~TableIndex()
{
	qDebug("TableIndex");
	parent_ = nullptr;
	clear();
}
void TableIndex::clear()
{
	auto delIter = FolderArray_.begin();
	while (delIter != FolderArray_.end())
	{
		TableIndex * pTableIndex = *delIter;
		delete pTableIndex;
		++delIter;
	}
	FolderArray_.clear();
	delIter = FileArray_.begin();
	while (delIter != FileArray_.end())
	{
		TableIndex * pTableIndex = *delIter;
		delete pTableIndex;
		++delIter;
	}
	FileArray_.clear();
}
void TableIndex::setEntry( const NodeEntry & node_entry)
{
	node_entry_ = node_entry;
}
NodeEntry TableIndex::getEntry() const
{
	return node_entry_;
}
int TableIndex::count() const
{
	return FolderArray_.size() + FileArray_.size();
}
void TableIndex::setParent(TableIndex * parent)
{
	parent_ = parent;
	setChecked(parent->checked());
}
TableIndex * TableIndex::parent() const
{
	return parent_;
}
TableIndex * TableIndex::getChild(const int rowNumber) const
{
	if (rowNumber < FolderArray_.size() )
	{
		//assert(FolderArray_.size() != 0 );
		return FolderArray_.at(rowNumber);
	}
	int fileRow = rowNumber - FolderArray_.size();
	return (fileRow < FileArray_.size() ) ? 
		FileArray_.at(fileRow) 
		: nullptr;
}
int TableIndex::row_index(const TableIndex * parent_index) const
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
void TableIndex::resetChilds()
{
	clear();
	for (int iNode = 0 ; iNode < node_entry_->count(); ++iNode)
	{
		insertChild(node_entry_->child(iNode));
	}
	std::sort(FolderArray_.begin(), FolderArray_.end(),GreaterName);
	std::sort(FileArray_.begin(), FileArray_.end(),GreaterName);
}
void TableIndex::insertChild( const NodeEntry & node_entry)
{
	TableIndex * pTableIndex = new TableIndex(node_entry);
	pTableIndex->setParent(this);

	if ( node_entry->type() == FileSystem::folder_type )
		FolderArray_.push_back(pTableIndex);
	else
		FileArray_.push_back(pTableIndex);
}
TableIndex * TableIndex::index_node(const wstring & folder_path)
{
	std::size_t firstPos = folder_path.find(L'\\');

	if ( firstPos == wstring::npos)
	{
		if ( folder_path.empty())
			return nullptr;

		TableIndex * pIndex = nullptr;
		auto it = std::find_if(FolderArray_.begin(),
			FolderArray_.end(), [&](const TableIndex * iIndex)
			//std::bind2nd( TableFindByName(), folder_path) );
		{ return iIndex->getEntry()->name().compare(folder_path) == 0; });


		if (it != FolderArray_.end())
			return *it;
		else
			return pIndex;
	}

	wstring subStr = folder_path.substr(0,firstPos);

	TableIndex * pIndex = nullptr;
	auto it = std::find_if( FolderArray_.begin(), 
							FolderArray_.end() , 
		[&](const TableIndex * iIndex)
		//std::bind2nd( TableFindByName(), folder_path) );
	{ return iIndex->getEntry()->name().compare(subStr) == 0; });

	if (it != FolderArray_.end())
		return (*it)->index_node(folder_path.substr(++firstPos));
	else
		return pIndex;
}
void TableIndex::sortByName( const int order)
{
	std::sort(FolderArray_.begin(), FolderArray_.end(),(order==0) ? GreaterName : LessName);
	std::sort(FileArray_.begin(), FileArray_.end(),(order==0) ? GreaterName : LessName);
}
wstring TableIndex::getRootPath()
{
	if (node_entry_->type() == FileSystem::folder_type)
	{
	wstring rootPath( node_entry_->name() );
	TableIndex * pParent = parent();
	while ( pParent != nullptr)
	{
		wstring parentName = pParent->getEntry()->name();
		rootPath.insert(0,wstring(pParent->getEntry()->name() + L'\\'));
		pParent = pParent->parent();
	}
	return rootPath;
	}
	else
		return wstring();
}
void TableIndex::updateCheckChilds(const Qt::CheckState checked)
{
	auto updateIter = FileArray_.begin();

	while ( updateIter != FileArray_.end() )
	{
		(*updateIter)->setChecked(checked);
		++updateIter;
	}

	updateIter = FolderArray_.begin();
	while ( updateIter != FolderArray_.end() )
	{
		(*updateIter)->updateCheckChilds(checked);
		(*updateIter)->setChecked(checked);
		++updateIter;
	}
}
void TableIndex::updateCheckParents(const Qt::CheckState checked)
{
	TableIndex * parentIndex = this->parent();
	while (parentIndex != nullptr)
	{
		parentIndex->setChecked(parentIndex->checkFormChilds(checked));
		parentIndex = parentIndex->parent();
	}

}
Qt::CheckState TableIndex::checkFormChilds(const Qt::CheckState current_state)
{
	if (FolderArray_.empty())
		return getStateCheckState(FileArray_,current_state);
	else
	{
		Qt::CheckState folderState = getStateCheckState(FolderArray_,current_state);
		if (FileArray_.empty())
			return folderState;
		else
		{
			Qt::CheckState fileState = getStateCheckState(FileArray_,current_state);

			if (fileState != folderState)
				return Qt::PartiallyChecked;
			else
				return folderState;
			
		}
	}
	return current_state;

}
Qt::CheckState getStateCheckState( const vector< TableIndex* > & arrayChilds, Qt::CheckState checked)
{
	if (arrayChilds.size() > 0)
	{
		auto updateIter = arrayChilds.begin();
		checked = (*updateIter)->checked();
		++updateIter;
		while ( updateIter != arrayChilds.end() )
		{
			if ( checked != (*updateIter)->checked() )
				return Qt::PartiallyChecked;
			++updateIter;			
		}	
	}
	return checked;
}


bool GreaterName(const TableIndex * first, const TableIndex * second)
{
	return ( first->getEntry()->name() < second->getEntry()->name() );
}
bool LessName(const TableIndex * first, const TableIndex * second)
{
	return ( first->getEntry()->name() >= second->getEntry()->name() );
	
}


FSTableModel::FSTableModel( QObject * parent, TableIndex * RootIndex)
	: QAbstractItemModel(parent)
	, RootIndex_(RootIndex)
	, sort_order(Qt::AscendingOrder)
{

}

FSTableModel::~FSTableModel()
{
	if (RootIndex_)
	{
		delete RootIndex_;
		RootIndex_ = nullptr;
	}
}
Qt::ItemFlags FSTableModel::flags(const QModelIndex &index) const
{
	return (index.isValid()) ? ( QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable )
		: QAbstractItemModel::flags(index);
}
QVariant FSTableModel::headerData(int section, Qt::Orientation orientation, int role ) const
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
int	FSTableModel::columnCount ( const QModelIndex & parent ) const 
{
	return (parent.isValid() && parent.column() != 0) ? 0 : sizeof( FSheader ) + 1;
}
int	FSTableModel::rowCount ( const QModelIndex & parent ) const
{
	if ( parent.isValid() && parent.column() != 0)
		return 0;
	if (parent.column() > 0)
		return 0;

	TableIndex * parent_index = fromIndex(parent);

	return (parent_index) ? parent_index->count() : 0 ;
}
QModelIndex	FSTableModel::index ( int row, int column, const QModelIndex & parent ) const
{
	if ( (RootIndex_ == NULL) || row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent) )
		return QModelIndex();

	if ( ! hasIndex(row,column, parent) )
		return QModelIndex();

	TableIndex * parent_index = fromIndex(parent);

	TableIndex * child_index  = parent_index->getChild(row);
	if (child_index)
		return createIndex( row ,column, child_index );

	return QModelIndex();
}
QModelIndex	FSTableModel::parent ( const QModelIndex & index ) const 
{
	if ( !index.isValid() )	
		return QModelIndex();

	if ( TableIndex * child_index = fromIndex(index) )
	{
		if ( TableIndex * parent_index = child_index->parent() )
		{
			if ( parent_index == RootIndex_)
				return QModelIndex();

			if ( TableIndex * grandParent_index = parent_index->parent() )
				return createIndex( grandParent_index->row_index(parent_index), 0 , parent_index);
		}
	}
	return QModelIndex();
}
QVariant FSTableModel::data ( const QModelIndex & index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	if ( TableIndex * model_index = static_cast < TableIndex *>
		( index.internalPointer() ) )
		if (NodeEntry node_entry = model_index->getEntry())
		{
				//if (role == Qt::CheckStateRole)
				//	return static_cast<int>(model_index->checked());
				//else
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
						return static_cast<int>( model_index->checked() );
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
				return QVariant();
}
bool FSTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if ( !index.isValid() )
		return false;

	if ( TableIndex * model_index = fromIndex(index))
	{
		if (role == Qt::CheckStateRole)
		{
			Qt::CheckState checkState =  (Qt::CheckState)value.toInt();
			model_index->updateCheckChilds(checkState);
			model_index->setChecked(checkState);
			model_index->updateCheckParents(checkState);
			if (model_index->getEntry()->type() == FileSystem::folder_type)
				emit updateTreeState(QString::fromStdWString( model_index->getRootPath()) , model_index->checked() );

			if (TableIndex * parent_index = model_index->parent())
				emit updateParentState( parent_index->checked() );


		}
		emit dataChanged(index,index);
		return true;
	}
	return false;

}
TableIndex * FSTableModel::fromIndex( const QModelIndex & qIndex) const
{
	if ( qIndex.isValid() )
	{
		TableIndex * model_index = static_cast < TableIndex *>
			( qIndex.internalPointer() ) ;
		return model_index;
	}
	return RootIndex_;
}

QString FSTableModel::dataToString( const WORD data_format) const
{
	BYTE day = data_format & 0x001F;
	BYTE mounth = ( data_format & 0x00F0 ) >> 5;
	WORD year = (( data_format & 0xFF00 ) >> 9) + 1980;
	//QDate dos_date( day, mounth, 12 );
	//bool bValid = dos_date.isValid();

	return ( QDate::isValid(year,mounth,day) ) ? QString("%1.%2.%3").arg(day).arg(mounth).arg(year) : QString("");
	//return ( dos_date.isValid() ) ? dos_date.toString(Qt::LocalDate) : QString("");
}
QString FSTableModel::timeToString( const WORD time_format, const BYTE milisec) const
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
//void FSTableModel::setRootPath(const wstring & rootPath)
//{
//	assert(RootIndex_ != nullptr);
//
//
//}
TableIndex * FSTableModel::indexByPath(const wstring & folder_path)
{
	std::size_t firstPos = folder_path.find(L'\\');
	if ( firstPos == wstring::npos)
	{
		if ( folder_path.length() > 0)
			return RootIndex_;
		else
			return nullptr;
	}
	TableIndex * pIndex = nullptr;
	// get "Root:"
	//pIndex = RootIndex_->getChild(0);
	wstring next_path(folder_path.substr(++firstPos));
//	if (pIndex)
		pIndex = RootIndex_->index_node(next_path);
	return pIndex;
}
QModelIndex FSTableModel::indexFromPath( const wstring & folder_path)
{
	if ( TableIndex * current_index = indexByPath(folder_path))
	{
		TableIndex *pParent = current_index->parent();
		if (pParent)
		{
			return createIndex(pParent->row_index(current_index),0,current_index);
		}
	}
	return QModelIndex();
}
void FSTableModel::sort ( int column, Qt::SortOrder order )
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
		if (TableIndex * parent_index = static_cast < TableIndex *>
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
