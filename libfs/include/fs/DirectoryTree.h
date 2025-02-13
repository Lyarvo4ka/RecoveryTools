#pragma once

// warning C4251: needs to have dll-interface to be used by clients of class
#pragma warning(disable : 4251)

#include "FileSystemDLL.h"

#include <string>
using std::string;
using std::wstring;

#include <map>
using std::map;
using std::pair;

#include <memory>
 #include <windows.h>

#include <vector>
using std::vector;
#include <list>
using std::list;

#include <memory>

//#ifdef _DEBUG
//bool _trace( char * format, ...);
#include <functional>

static string ToString(const wstring & _String)
{
	string strTo;
	if (_String.size() > 0)
	{
		strTo.resize(_String.size());
		//ANSI Cyrillic; Cyrillic (Windows)
		//CP_ACP
		WideCharToMultiByte(CP_ACP, 0, _String.c_str(), -1, &strTo[0], strTo.size(), NULL, NULL);
	}
	return strTo;
}

static wstring ToWideString(const string & _String)
{
	wstring strTo;
	if (_String.size() > 0)
	{
		strTo.resize(_String.size());
		//ANSI Cyrillic; Cyrillic (Windows)
		//WideCharToMultiByte(1251, 0, _String.c_str(), -1, &strTo[0], strTo.size(), NULL, NULL);
		MultiByteToWideChar(CP_ACP, 0, _String.c_str(), -1, &strTo[0], strTo.size());
	}
	return strTo;
}

namespace FileSystem
{
	class IVirtualNode;
	typedef std::shared_ptr<IVirtualNode> NodeEntry;
	class FileNode;
	typedef std::shared_ptr<FileNode> FileEntry;
	class DirectoryNode;
	typedef std::shared_ptr<DirectoryNode> DirectoryEntry;

	typedef LONGLONG file_size_type;

	enum NodeType{ folder_type, file_type};

	class EntryTimes
	{
	public:
		EntryTimes(const WORD data_created, const WORD time_created,
			      const WORD data_modified, const WORD time_modified,
				  const WORD data_accessed, const WORD time_accessed,
				  const BYTE time_created_ms);
		WORD created_data() const;
		WORD created_time() const;
		WORD accessed_data() const;
		WORD accessed_time() const;
		WORD modified_data() const;
		WORD modified_time() const;
		BYTE created_time_ms() const;

	private:
		WORD created_data_;
		WORD created_time_;
		WORD accessed_data_;
		WORD accessed_time_;
		WORD modified_data_;
		WORD modified_time_;
		BYTE created_time_ms_;

	};


	// to header AbstractFS.h
	class ClusterEntry
	{
	public:
		ClusterEntry(const DWORD cluster_number, const DWORD count_clusters)
			: number_( cluster_number )
			, count_ ( count_clusters )
		{

		}
		DWORD getNumber() const
		{
			return number_;
		}
		DWORD getCount() const
		{
			return count_;
		}
		void setNumber(const DWORD cluster_number)
		{
			number_ = cluster_number;
		}
		void setCount(const DWORD count_clusters)
		{
			count_ = count_clusters;
		}
		void setClusterData(const DWORD cluster_number, const DWORD count_cluster)
		{
			setNumber(cluster_number);
			setCount(count_cluster);
		}
		void inc()
		{
			++count_;
		}
	private:
		DWORD number_;
		DWORD count_;
	};

	typedef std::list< ClusterEntry > ListClusters;

	class File_Handle
	{
	public:
		File_Handle()
			: opened(false)
			, position_(0)
		{
			currentInter = clusterList_.end();
		}
		void setPosition(const LONGLONG position)
		{
			position_ = position;
		}
		void addPosition(const LONGLONG position)
		{
			position_ += position;
		}
		LONGLONG getPosition() const
		{
			return position_;
		}	
		void add( const ClusterEntry & cluster_entry)
		{
			clusterList_.push_back(cluster_entry);
		}
		void clearList()
		{
			clusterList_.clear();
		}
		bool isOpen() const
		{
			return opened;
		}
		const ListClusters * getClusterList() const
		{
			return &clusterList_;
		}
		std::size_t getSize() const
		{
			return clusterList_.size();
		}
	private:
		bool opened;				// is Opened file
		LONGLONG position_;			// position in bytes
		ListClusters clusterList_;	// list of clusters
		ListClusters::iterator currentInter;
	};

	class IVirtualNode
	{
	public:

		IVirtualNode();
		IVirtualNode(const string & Name);
		virtual ~IVirtualNode(){/**/};


		virtual DirectoryNode * parent() const = 0;
		virtual NodeEntry child(const wstring & node_name) = 0;
		virtual NodeEntry child(const int nIndex) = 0;
		virtual int count() const = 0;
		virtual wstring name() const = 0;
		virtual file_size_type size() const = 0;
		virtual NodeType type() const = 0;
		virtual DWORD cluster() const = 0;
		//	return structure to 
		virtual EntryTimes * getDates() const = 0;
	};

	////////////////////////////////File//////////////////////////////////////
	class FileNode
		: public IVirtualNode
	{
	public:
		FileNode();
		FileNode(const string & file_name);
		FileNode(const char * file_name);
		FileNode(const char * file_name, const file_size_type fileSize);

		FileNode(const wstring & file_name);
		FileNode(const wchar_t * file_name, const file_size_type fileSize);
		FileNode(const wchar_t * file_name);
		
		virtual ~FileNode();

		virtual DirectoryNode * parent() const;
		virtual NodeEntry child(const wstring & node_name);
		virtual NodeEntry child(const int nIndex);
		virtual int count() const;
		virtual wstring name() const ;
		virtual file_size_type size() const ;
		virtual NodeType type() const;

		DWORD cluster() const;

		void setParent( DirectoryNode * parent);
		void setParent( const DirectoryEntry & parent);
		void setFileSize(const file_size_type file_size);
		void setCluster( const DWORD clusterNumber);

		EntryTimes * getDates() const;
		void setFileDates(EntryTimes * pFileTimes);

		File_Handle * getFileHandle() const;
		File_Handle * OpenFile();
		void CloseFile();

	private:
		DirectoryNode * Parent_;
		wstring file_name_;
		file_size_type file_size_;
		DWORD cluster_;
		EntryTimes * file_times_;
		File_Handle * FileHandle_;
	};

	///////////////////////////////Directory//////////////////////////////////

	//class CompareByName
	//	: public std::binary_function< NodeEntry, string, bool>
	//{
	//public:
	//	bool operator() ( const NodeEntry & node_entry, const string & compareStr) const
	//	{
	//		//return ( node_entry->name().compare( compareStr ) == 0 ) ? true : false;
	//		return (node_entry->name() == compareStr) ;
	//	}
	//};

	NodeEntry findByIndex( const list< NodeEntry > & nodeList, int nIndex);


	class DirectoryNode
		: public IVirtualNode
	{
	public:
		DirectoryNode();

		DirectoryNode(const string & file_name);
		DirectoryNode(const char * Name);
		DirectoryNode(const char * Name, const file_size_type directorySize);

		DirectoryNode(const wstring & file_name);
		DirectoryNode(const wchar_t * Name);
		DirectoryNode(const wchar_t * Name, const file_size_type directorySize);


		virtual ~DirectoryNode();


		virtual DirectoryNode * parent() const;
		virtual NodeEntry child(const wstring & node_name);
		virtual NodeEntry child(const int nIndex);
		virtual int count() const;
		virtual wstring name() const ;
		virtual file_size_type size() const ;
		virtual NodeType type() const;

		//	?????????????????
		int indexNumber(IVirtualNode * childNode) const;

		void add_folder( DirectoryNode * directory_node );
		void add_folder( DirectoryEntry & directory_entry );
		void add_file( FileNode * file_node );
		void add_file( FileEntry & file_entry);
		void add_Entry(NodeEntry node_entry);
		
		void setParent( DirectoryNode * parent);
		void setParent(const DirectoryEntry & parentFolder);
		
		void setCluster(const DWORD cluster);
		DWORD cluster() const;

		NodeEntry first() /*const*/;
		NodeEntry next() /*const*/;

		EntryTimes * getDates() const;
		void setFolderDates(EntryTimes * pFolderTimes);

	private:
		typedef map<wstring , NodeEntry> DirectoryMap;
		typedef pair<wstring , NodeEntry> DirectoryPair;
		typedef map<wstring, NodeEntry>::const_iterator constIter;
		//typedef list< NodeEntry >::const_iterator ConstIter;

		//list< NodeEntry > FolderList_;
		//list< NodeEntry > FileList_;

		//ConstIter iter_folder_;
		//ConstIter iter_file_;
		DirectoryMap DirectoryMap_;
		constIter current_Node_;
		DirectoryNode * Parent_;
		mutable file_size_type directory_size_;
		wstring directory_name_;
		DWORD cluster_;
		EntryTimes *folder_times_;
	};
}

//#include <vector>
//class DirectoryHolder
//{
//public:
//	DirectoryHolder();
//
//private:
//	vector<DirectoryHolder *> childs_;
//	DirectoryHolder * parent_;
//}