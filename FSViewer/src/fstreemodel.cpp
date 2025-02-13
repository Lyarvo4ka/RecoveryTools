#include "fstreemodel.h"

TreeIndex::TreeIndex()
	: parent_(nullptr)
{
}
TreeIndex::TreeIndex( const DirectoryEntry & directory)
	: parent_(nullptr)
	, directory_(directory)
{
	resetChilds();
}
TreeIndex::~TreeIndex()
{
	qDebug("TreeIndex");
	parent_ = nullptr;
	clear();
}
void TreeIndex::clear()
{
	auto delIter = folders_.begin();
	while (delIter != folders_.end())
	{
		TreeIndex * deleteIndex = delIter->second;
		delete deleteIndex;
		++ delIter;
	}
	folders_.clear();
}
void TreeIndex::setEntry( const DirectoryEntry & directory)
{
	directory_ = directory;
}
DirectoryEntry TreeIndex::getEntry() const
{
	return directory_;
}
int TreeIndex::count() const
{
	return folders_.size();
}
void TreeIndex::setParent(TreeIndex * parent)
{
	parent_ = parent;
}
TreeIndex * TreeIndex::parent() const
{
	return parent_;
}
TreeIndex * TreeIndex::getChild(const int rowNumber) const
{
	auto mapIter(folders_.begin());
	std::advance(mapIter,rowNumber);
	return ( mapIter != folders_.end() ) ? mapIter->second : nullptr;
}
int TreeIndex::row_index(const TreeIndex * parent_index) const
{
	assert(parent_index->getEntry() != nullptr);
	auto findIter = folders_.find(parent_index->getEntry()->name());
	return (findIter != folders_.end() ) ?
		std::distance(folders_.begin(),findIter)
		: -1;
}
void TreeIndex::resetChilds()
{
	clear();
	if (directory_)
		for ( std::size_t iChild = 0; iChild < directory_->count(); ++ iChild )
		{
			NodeEntry dirEntry(directory_->child(iChild));
			if (dirEntry->type() == FileSystem::folder_type)
				insertChild(std::static_pointer_cast<DirectoryNode>(dirEntry));
		}
}
void TreeIndex::insertChild( const DirectoryEntry & directory_entry)
{
	TreeIndex * pTreeIndex = new TreeIndex(directory_entry);
	pTreeIndex->setParent(this);

	folders_.insert( PairFolder( directory_entry->name(), pTreeIndex) );
}
wstring TreeIndex::getRootPath()
{
	wstring rootPath( directory_->name() );
	TreeIndex * pParent = parent();
	while ( pParent != nullptr)
	{
		wstring parentName = pParent->getEntry()->name();
		rootPath.insert(0,wstring(pParent->getEntry()->name() + L'\\'));
		pParent = pParent->parent();
	}

	return rootPath;
}
TreeIndex * TreeIndex::index_node(const wstring & folder_path)
{
	std::size_t firstPos = folder_path.find(L'\\');

	if ( firstPos == wstring::npos)
	{
		if ( folder_path.empty())
			return nullptr;

		TreeIndex * pIndex = nullptr;
		auto it = folders_.find(folder_path);

		if ( it != folders_.end() )
			pIndex = it->second;
		return pIndex;
	}

	wstring subStr = folder_path.substr(0,firstPos);

	TreeIndex * pIndex = nullptr;
	auto it = folders_.find(subStr) ;
	if (it != folders_.end())
		return it->second->index_node(folder_path.substr(++firstPos));
	return pIndex;
}
void TreeIndex::updateCheckChilds(const Qt::CheckState checked)
{
	auto updateIter = folders_.begin();

	while ( updateIter != folders_.end() )
	{
		updateIter->second->setChecked(checked);
		++updateIter;
	}
}
void TreeIndex::updateCheckParents(const Qt::CheckState checked)
{
	TreeIndex * parentIndex = this->parent();
	while (parentIndex != nullptr)
	{
		parentIndex->setChecked(parentIndex->checkFormChilds(checked));
		parentIndex = parentIndex->parent();
	}
}
Qt::CheckState TreeIndex::checkFormChilds(const Qt::CheckState current_state)
{
	Qt::CheckState currentState = this->checked();
	auto checkIter = folders_.begin();

	while (checkIter != folders_.end())
	{
		if ( currentState != checkIter->second->checked() )
			return Qt::PartiallyChecked;
		++checkIter;
	}
	return current_state;
}


FSTreeModel::FSTreeModel( QObject * parent, TreeIndex * RootIndex)
	: QAbstractItemModel(parent)
	, RootIndex_(RootIndex)
{

}
FSTreeModel::~FSTreeModel()
{
	if (RootIndex_)
	{
		delete RootIndex_;
		RootIndex_ = nullptr;
	}
}
Qt::ItemFlags FSTreeModel::flags(const QModelIndex &index) const
{
	return (index.isValid()) ? ( QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable )
		: QAbstractItemModel::flags(index);
}
QVariant FSTreeModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
	//return (orientation == Qt::Horizontal && role == Qt::DisplayRole) ?
	//	tr("") : QVariant();
	return tr("header...");
}
int	FSTreeModel::columnCount ( const QModelIndex & parent ) const 
{
	return (parent.isValid() && parent.column() != 0) ? 0 : 1;
}
int	FSTreeModel::rowCount ( const QModelIndex & parent ) const
{
	if ( parent.isValid() && parent.column() != 0)
		return 0;
	if (parent.column() > 0)
		return 0;

	TreeIndex * parent_index = fromIndex(parent);

	return (parent_index) ? parent_index->count() : 0 ;
}
QModelIndex	FSTreeModel::index ( int row, int column, const QModelIndex & parent ) const
{
	if ( (RootIndex_ == NULL) || row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent) )
		return QModelIndex();

	if ( ! hasIndex(row,column, parent) )
		return QModelIndex();

	TreeIndex * parent_index = fromIndex(parent);

	TreeIndex * child_index  = parent_index->getChild(row);
	if (child_index)
		return createIndex( row ,column, child_index );

	return QModelIndex();
}
QModelIndex	FSTreeModel::parent ( const QModelIndex & index ) const 
{
	if ( !index.isValid() )	
		return QModelIndex();

	if ( TreeIndex * child_index = fromIndex(index) )
	{
		if ( TreeIndex * parent_index = child_index->parent() )
		{
			if ( parent_index == RootIndex_)
				return QModelIndex();

			if ( TreeIndex * grandParent_index = parent_index->parent() )
				return createIndex( grandParent_index->row_index(parent_index), 0 , parent_index);

		}
	}
	return QModelIndex();
}
QVariant FSTreeModel::data ( const QModelIndex & index, int role ) const
{
	if (!index.isValid())
		return QVariant();

	if ( TreeIndex * model_index = static_cast < TreeIndex *>
		( index.internalPointer() ) )
	if (NodeEntry node_entry = model_index->getEntry())
		if (role == Qt::DisplayRole)
		{
			return QString::fromStdWString(node_entry->name().c_str());
		}
		else
			if ( index.column() == 0 )
			{
				if (role == Qt::CheckStateRole)
					return static_cast<int>(model_index->checked());
				else
					if (role == Qt::DecorationRole)
					{
						return (node_entry->type() == FileSystem::folder_type) ?
							iconProvider_.icon(QFileIconProvider::Folder)
							: iconProvider_.icon(QFileIconProvider::File);

					}
			}

	return QVariant();
}
bool FSTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if ( !index.isValid() )
		return false;

	if ( TreeIndex * model_index = fromIndex(index))
	{
		if (role == Qt::CheckStateRole)
		{
			Qt::CheckState checkState =  (Qt::CheckState)value.toInt();
			model_index->updateCheckChilds(checkState);
			model_index->setChecked(checkState);
			model_index->updateCheckParents(checkState);
			//if (TableIndex * parent_index = model_index->parent())
			emit updateTableState(model_index->checked());

		}
		emit dataChanged(index,index);
		return true;
	}
	return false;

}
TreeIndex * FSTreeModel::fromIndex( const QModelIndex & qIndex) const
{
	if ( qIndex.isValid() )
	{
		TreeIndex * model_index = static_cast < TreeIndex *>
			( qIndex.internalPointer() ) ;
		return model_index;
	}
	return RootIndex_;
}
void FSTreeModel::update_view(const QModelIndex & index)
{
	emit dataChanged(index,index);
}
TreeIndex * FSTreeModel::indexByPath( const wstring & folder_path )
{
	std::size_t firstPos = folder_path.find(L'\\');
	if ( firstPos == wstring::npos)
	{
		if ( folder_path.length() > 0)
			return RootIndex_;
		else
			return nullptr;
	}
	TreeIndex * pIndex = nullptr;
	// get "Root:"
	//pIndex = RootIndex_->getChild(0);
	wstring next_path(folder_path.substr(++firstPos));
	//	if (pIndex)
	pIndex = RootIndex_->index_node(next_path);
	return pIndex;
}
QModelIndex FSTreeModel::indexFromPath( const wstring & folder_path )
{
	if ( TreeIndex * current_index = indexByPath(folder_path))
	{
		TreeIndex *pParent = current_index->parent();
		if (pParent)
		{
			return createIndex(pParent->row_index(current_index),0,current_index);
		}
	}
	return QModelIndex();
}
void FSTreeModel::updateCheckState(const QModelIndex & currentIndex, Qt::CheckState checkState)
{
	if (currentIndex.isValid())
	{
		if (TreeIndex * pIndex = static_cast<TreeIndex *> ( currentIndex.internalPointer() ) )
			pIndex->setChecked(checkState);
	//	TreeIndex * parentIndex = pIndex->parent();
	//	while ( parentIndex = nullptr)
	//	{
	//		parentIndex->setChecked(checkState);
	//		parentIndex = parentIndex->parent();
	//	}
	}
}
