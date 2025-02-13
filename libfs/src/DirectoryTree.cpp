
#include "fs/DirectoryTree.h"
#include "fs/const_fs.h"

#include <assert.h>

#include <algorithm>


#include <stdio.h>

FileSystem::IVirtualNode::IVirtualNode()
{
};
FileSystem::IVirtualNode::IVirtualNode(const string & Name)
{

}	
//void FileSystem::IVirtualNode::setClusterNumber(DWORD cluster_number)
//{
//	cluster_number_ = cluster_number;
//}
//void FileSystem::IVirtualNode::setName(const char * directoryName)
//{
//	name_.append(directoryName,ms_dos_name);
//}
//string FileSystem::IVirtualNode::name() const 
//{
//	return name_;
//}
//DWORD FileSystem::IVirtualNode::size() const 
//{
//	return size_;
//}
//DWORD FileSystem::IVirtualNode::cluster_number() const 
//{
//	return cluster_number_;
//}

/************************************************************************/
/*                            File                                      */
/************************************************************************/
FileSystem::FileNode::FileNode()
	: Parent_(NULL)
	, file_name_(L"")
	, file_size_(0)
	, cluster_(0)
	, file_times_(NULL)
	, FileHandle_(nullptr)
{

}
FileSystem::FileNode::FileNode(const string & file_name)
	: Parent_(NULL)
	, file_name_(L"")
	, file_size_(0)
	, cluster_(0)	
	, file_times_(NULL)
	, FileHandle_(nullptr)
{
	file_name_ = ToWideString(file_name);
}
FileSystem::FileNode::FileNode(const char * file_name)
	: Parent_(NULL)
	, file_name_(L"")
	, file_size_(0)
	, cluster_(0)
	, file_times_(NULL)
	, FileHandle_(nullptr)
{
	file_name_ = ToWideString(file_name);
}
FileSystem::FileNode::FileNode(const char * file_name, const file_size_type fileSize)
	: Parent_(NULL)
	, file_name_(L"")
	, file_size_(fileSize)
	, cluster_(0)
	, file_times_(NULL)
	, FileHandle_(nullptr)
{
	file_name_ = ToWideString(file_name);
}
FileSystem::FileNode::FileNode(const wstring & file_name)
	: Parent_(NULL)
	, file_name_(L"")
	, file_size_(0)
	, cluster_(0)	
	, file_times_(NULL)
	, FileHandle_(nullptr)
{
}
FileSystem::FileNode::FileNode(const wchar_t * file_name)
	: Parent_(NULL)
	, file_name_(file_name)
	, file_size_(0)
	, cluster_(0)
	, file_times_(NULL)
	, FileHandle_(nullptr)
{
}
FileSystem::FileNode::FileNode(const wchar_t * file_name, const file_size_type fileSize)
	: Parent_(NULL)
	, file_name_(file_name)
	, file_size_(fileSize)
	, cluster_(0)
	, file_times_(NULL)
	, FileHandle_(nullptr)
{
}
FileSystem::FileNode::~FileNode()
{
//	DEBUG_SHOW("Destructor [\"FileNode\"]...(%s)\r\n",ToString(file_name_).c_str());	
	if (file_times_)
	{
		delete file_times_;
		file_times_ = NULL;
	}
	Parent_ = NULL;
	this->CloseFile();
}
FileSystem::DirectoryNode * FileSystem::FileNode::parent() const
{
	return Parent_;
}
FileSystem::NodeEntry FileSystem::FileNode::child(const wstring & node_name)
{
	return NULL;
}
FileSystem::NodeEntry FileSystem::FileNode::child(const int nIndex)
{
	return NULL;
}
int FileSystem::FileNode::count() const
{
	return 0;
}
wstring FileSystem::FileNode::name() const
{
	return file_name_;
}
FileSystem::file_size_type FileSystem::FileNode::size() const 
{
	return file_size_;
}
FileSystem::NodeType FileSystem::FileNode::type() const
{
	return file_type;
}
DWORD FileSystem::FileNode::cluster() const
{
	return cluster_;
}
void FileSystem::FileNode::setParent( DirectoryNode * parent)
{
	Parent_ = parent;
}
void FileSystem::FileNode::setParent( const DirectoryEntry & parent)
{
	Parent_ = parent.get();
}
void FileSystem::FileNode::setFileSize(const file_size_type file_size)
{
	file_size_ = file_size;
}
void FileSystem::FileNode::setCluster(const DWORD clusterNumber)
{
	cluster_ = clusterNumber;
}
FileSystem::EntryTimes * FileSystem::FileNode::getDates() const
{
	return file_times_;
}
void FileSystem::FileNode::setFileDates(EntryTimes * pEntryTimes)
{
	file_times_ = pEntryTimes;
}
FileSystem::File_Handle * FileSystem::FileNode::getFileHandle() const
{
	return FileHandle_;
}
FileSystem::File_Handle * FileSystem::FileNode::OpenFile()
{
	if (FileHandle_)
		delete FileHandle_;

	FileHandle_ = new File_Handle();
	return FileHandle_;
}
void FileSystem::FileNode::CloseFile()
{
	if ( FileHandle_ )
	{
		delete FileHandle_;
		FileHandle_ = nullptr;
	}
}
/************************************************************************/
/*                          Directory                                   */
/************************************************************************/

FileSystem::NodeEntry FileSystem::findByIndex( const list< NodeEntry > & nodeList, int nIndex)
{
	auto findIter = nodeList.begin();
	int findIndex = 0;
	while (findIter != nodeList.end())
	{
		if (findIndex == nIndex)
			return *findIter;

		++findIndex;
		++findIter;
	}
	return NULL;
}


FileSystem::DirectoryNode::DirectoryNode()
	: Parent_(NULL)
	, directory_size_(0)
	, directory_name_(L"")
	, cluster_(0)
	, folder_times_(NULL)
{
	current_Node_ = DirectoryMap_.end();
}
FileSystem::DirectoryNode::DirectoryNode(const string & Name)
	: Parent_(NULL) 
	, directory_size_(0)
	, directory_name_(L"")
	, cluster_(0)
	, folder_times_(NULL)
{
	directory_name_ = ToWideString(Name);
	current_Node_ = DirectoryMap_.end();
}
FileSystem::DirectoryNode::DirectoryNode(const char * Name)
	: Parent_(NULL) 
	, directory_size_(0)
	, directory_name_(L"")
	, cluster_(0)
	, folder_times_(NULL)
{
	directory_name_ = ToWideString(Name);
	current_Node_ = DirectoryMap_.end();

}
FileSystem::DirectoryNode::DirectoryNode(const char * Name, const file_size_type directorySize)
	: Parent_(NULL) 
	, directory_size_(0)
	, directory_name_(L"")
	, cluster_(0)
	, folder_times_(NULL)
{
	directory_name_ = ToWideString(Name);
	current_Node_ = DirectoryMap_.end();
}
FileSystem::DirectoryNode::DirectoryNode(const wstring & Name)
	: Parent_(NULL) 
	, directory_size_(0)
	, directory_name_(Name)
	, cluster_(0)
	, folder_times_(NULL)
{
	current_Node_ = DirectoryMap_.end();
}
FileSystem::DirectoryNode::DirectoryNode(const wchar_t * Name)
	: Parent_(NULL) 
	, directory_size_(0)
	, directory_name_(Name)
	, cluster_(0)
	, folder_times_(NULL)
{
	current_Node_ = DirectoryMap_.end();

}
FileSystem::DirectoryNode::DirectoryNode(const wchar_t * Name, const file_size_type directorySize)
	: Parent_(NULL) 
	, directory_size_(0)
	, directory_name_(Name)
	, cluster_(0)
	, folder_times_(NULL)
{
	current_Node_ = DirectoryMap_.end();
}
FileSystem::DirectoryNode::~DirectoryNode()
{
//	DEBUG_SHOW("Destructor [\"DirectoryNode\"]...(%s)\r\n",ToString(directory_name_).c_str());
	if (folder_times_)
	{
		delete folder_times_;
		folder_times_ = NULL;
	}
}
FileSystem::DirectoryNode * FileSystem::DirectoryNode::parent() const
{
	return Parent_;
}

FileSystem::NodeEntry FileSystem::DirectoryNode::child(const wstring & node_name)
{
	auto findIter = DirectoryMap_.find(node_name);
	return ( findIter != DirectoryMap_.end() ) ? findIter->second : NULL;
	//auto findIter = std::find_if( FolderList_.begin(), FolderList_.end(), std::bind2nd(CompareByName(),node_name) );

	//if (findIter != FolderList_.end())
	//	return *findIter;

	//findIter = std::find_if( FileList_.begin(), FileList_.end(), std::bind2nd(CompareByName(),node_name) );

	//return (findIter != FileList_.end()) ? *findIter : NULL;
}
FileSystem::NodeEntry FileSystem::DirectoryNode::child(const int nIndex)
{
	//if (nIndex < FolderList_.size())
	//{
	//	if ( NodeEntry node_entry = findByIndex (FolderList_, nIndex ) )
	//		return node_entry;
	//}
	//return findByIndex(FileList_,nIndex - FolderList_.size() /*- 1*/);

	auto findIter = DirectoryMap_.begin();
	int iRowNumber = 0;
	while (findIter != DirectoryMap_.end())
	{
		if (nIndex == iRowNumber)
			return findIter->second;

		++iRowNumber;
		++findIter;
	}
	return NULL;
}
int FileSystem::DirectoryNode::count() const
{
	return DirectoryMap_.size();
}
wstring FileSystem::DirectoryNode::name() const
{
	return directory_name_;
}
FileSystem::file_size_type FileSystem::DirectoryNode::size() const 
{
	//auto contsIter = DirectoryMap_.begin();
	//directory_size_ = 0;
	//while (contsIter != DirectoryMap_.end())
	//{
	//	directory_size_ += contsIter->second->size();
	//	++contsIter;
	//}
	return directory_size_;
}
FileSystem::NodeType FileSystem::DirectoryNode::type() const
{
	return folder_type;
}

void FileSystem::DirectoryNode::add_folder(DirectoryEntry & directory_entry)
{
	assert(directory_entry != NULL);

	directory_entry->setParent(this);
	DirectoryMap_.insert(DirectoryPair(directory_entry->name(), directory_entry));
	//FolderList_.push_back(directory_entry);
}

void FileSystem::DirectoryNode::add_folder( DirectoryNode * directory)
{
	assert(directory != NULL);
	std::shared_ptr<DirectoryNode> dir_ptr(directory);
	add_folder(dir_ptr);

}
void FileSystem::DirectoryNode::add_file(FileNode *file_node)
{
	assert(file_node != NULL);
	std::shared_ptr<FileNode> file_ptr(file_node);
	add_file(file_ptr);
}
void FileSystem::DirectoryNode::add_file( FileEntry & file_entry)
{
	assert (file_entry != NULL);

	file_entry->setParent(this);
	DirectoryMap_.insert( DirectoryPair( file_entry->name(), file_entry) );
	//FileList_.push_back(file_entry);
}
void FileSystem::DirectoryNode::add_Entry(NodeEntry node_entry)
{
	assert (node_entry != NULL);
	//DirectoryMap_.insert( DirectoryPair (node_entry->name(), node_entry) );
}
//	?????????????????
int FileSystem::DirectoryNode::indexNumber( IVirtualNode * childNode) const
{
	auto constIter = DirectoryMap_.find(childNode->name());
	return ( constIter != DirectoryMap_.end() ) ?
				std::distance(DirectoryMap_.begin(), constIter)
				: -1;

	//auto const_inter = std::find_if( FolderList_.begin(), FolderList_.end(), std::bind2nd(CompareByName(),childNode->name()) );
	//if (const_inter != FolderList_.end())
	//	return std::distance(FolderList_.begin(),const_inter);

	//const_inter = std::find_if( FileList_.begin(), FileList_.end(), std::bind2nd(CompareByName(),childNode->name()) );

	//return (const_inter != FileList_.end() ) ? 
	//					std::distance(FileList_.begin(), const_inter) + FolderList_.size()
	//					: -1;
}
void FileSystem::DirectoryNode::setParent( DirectoryNode * parent)
{
	Parent_ = parent;
}
void FileSystem::DirectoryNode::setParent(const DirectoryEntry & parentFolder)
{
	Parent_ = parentFolder.get();
}
void FileSystem::DirectoryNode::setCluster(const DWORD cluster)
{
	cluster_ = cluster;
}
DWORD FileSystem::DirectoryNode::cluster() const
{
	return cluster_;
}
FileSystem::NodeEntry FileSystem::DirectoryNode::first() /*const*/
{
	current_Node_ = DirectoryMap_.begin();
	if ( current_Node_ != DirectoryMap_.end() )
	{
		return current_Node_->second;
	}
	return NULL;
}
FileSystem::NodeEntry FileSystem::DirectoryNode::next() /*const*/
{
	if ( current_Node_ != DirectoryMap_.end() )
	{
		++current_Node_;
		if (current_Node_ != DirectoryMap_.end())
			return current_Node_->second;
	}
	return NULL;
}
FileSystem::EntryTimes * FileSystem::DirectoryNode::getDates() const
{
	return folder_times_;
}
void FileSystem::DirectoryNode::setFolderDates(EntryTimes * pFolderTimes)
{
	folder_times_ = pFolderTimes;
}



FileSystem::EntryTimes::EntryTimes(const WORD data_created, const WORD time_created,
	const WORD data_modified, const WORD time_modified,
	const WORD data_accessed, const WORD time_accessed, const BYTE time_created_ms)
	: created_data_( data_created )
	, created_time_( time_created )
	, modified_data_( data_modified )
	, modified_time_( time_modified )
	, accessed_data_( data_accessed )
	, accessed_time_( time_accessed )
	, created_time_ms_( time_created_ms )
{

}
WORD FileSystem::EntryTimes::created_data() const
{
	return created_data_;
}
WORD FileSystem::EntryTimes::created_time() const
{
	return created_time_;
}
WORD FileSystem::EntryTimes::accessed_data() const
{
	return accessed_data_;
}
WORD FileSystem::EntryTimes::accessed_time() const
{
	return accessed_time_;
}
WORD FileSystem::EntryTimes::modified_data() const
{
	return modified_data_;
}
WORD FileSystem::EntryTimes::modified_time() const
{
	return modified_time_;
}
BYTE FileSystem::EntryTimes::created_time_ms() const
{
	return created_time_ms_;
}
