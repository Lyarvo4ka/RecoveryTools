#pragma once

#include "FileSystemDLL.h"

#include "DirectoryTree.h"
#include "VirtualReader.h"



namespace FileSystem
{
	#include <list>

	#pragma pack(1)
	struct partition_enty
	{
		BYTE boot_flag;			// partition boot flag
		BYTE start_CHS[3];		// start address CHS
		BYTE partition_type;	// partition type
		BYTE end_CHS[3];		// end address CHS
		DWORD start_LBA;		// start address LBA
		DWORD sector_count;		// count in sectors
	};
	#pragma pack ()

	class CPartition
	{
	public:
		CPartition(const DWORD start_sector, const DWORD sector_count)
			: start_sector_(start_sector)
			, sector_count_(sector_count)
		{

		};
		void setData(const DWORD start_sector, const DWORD sector_count)
		{
			start_sector_ = start_sector;
			sector_count_ = sector_count;
		}
		void setStartSector(const DWORD start_sector)
		{
			start_sector_ = start_sector;
		}
		void setSectorCount(const DWORD sector_count)
		{
			sector_count_ = sector_count;
		}
		DWORD start_sector() const
		{
			return start_sector_;
		}
		DWORD sector_count() const
		{
			return sector_count_;
		}
	private:
		DWORD start_sector_;
		DWORD sector_count_;
	};

	typedef std::shared_ptr<CPartition> PartitionEntry;

	class  AbstractFileSystem
	{
	public:
		virtual ~AbstractFileSystem() {};

		virtual bool mount( const PartitionEntry &) = 0;

		/*
			"getFirst" writes (first) file or folder, from directory entry "head_folder",
			into input_node. Function if success returns true.
		*/
		virtual bool getFirst( DirectoryEntry & head_folder, NodeEntry & input_node) = 0;

		/*
			"getNext" writes (next) file or folder. Into input_node.
		*/
		virtual bool getNext( NodeEntry & node_entry ) = 0;

		virtual bool setOffset(FileEntry file_entry, LONGLONG offset) = 0;
		virtual bool ReadFile( const FileEntry file_entry, BYTE * data, DWORD data_size, DWORD & bytes_read) = 0;

	};

	typedef std::shared_ptr<AbstractFileSystem> AbstractFS;



	//class ClusterEntry
	//{
	//public:
	//	ClusterEntry(const DWORD cluster_number, const DWORD count_clusters);
	//	DWORD getNumber() const
	//	{
	//		return number_;
	//	}
	//	DWORD getCount() const
	//	{
	//		return count_;
	//	}
	//	void setNumber(const DWORD cluster_number)
	//	{
	//		number_ = cluster_number;
	//	}
	//	void setCount(const DWORD count_clusters)
	//	{
	//		count_ = count_clusters;
	//	}
	//	void setClusterData(const DWORD cluster_number, const DWORD count_cluster)
	//	{
	//		setNumber(cluster_number);
	//		setCount(count_cluster);
	//	}
	//	void inc()
	//	{
	//		++count_;
	//	}
	//private:
	//	DWORD number_;
	//	DWORD count_;
	//};

	//typedef std::list< ClusterEntry > ListClusters;

}