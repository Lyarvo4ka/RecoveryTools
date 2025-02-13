#pragma once

#include "AbstractFS.h"
//#include "FileSystemDLL.h"

// warning C4482: nonstandard extension used: enum 'FileSystem::partition_types' used in qualified name
#pragma warning(disable : 4482)

#include "BootSector.h"
#include "VirtualReader.h"

#include <vector>
using std::vector;


#include <string>
using std::wstring;

#include <queue>
using std::queue;



namespace FileSystem
{
	/////////////////////////////// MBR //////////////////////////////////
	class  MasterBootRecord
	{
	public:
		static const WORD mbr_size = 512;
		static const uint8_t partition_count = 4;
		static const WORD partition_offset = 446;

		MasterBootRecord(const DWORD offsetMBR = 0);
		virtual ~MasterBootRecord();
		void setOffset( const DWORD offsetMBR);
		virtual bool open(const VirtualReader &);
		DWORD count() const;
		PartitionEntry getPartition(const DWORD number) const;

		//void ReadPartitions()
	private:
//		bool add_partition()
		DWORD mbr_offset_;
		vector<PartitionEntry> PartitionArray_;
	};

	/////////////////////////////// Fat_Table //////////////////////////////////
	class  Fat_Table
	{
	public:
		Fat_Table(const VirtualReader & pVirualReader, const LONGLONG offset);
		~Fat_Table();

		void InitTable(const FatBootPtr &);
		DWORD Next_Cluster(DWORD current_cluster);		//return next cluster from FAT table
		
		DWORD size() const
		{
			return table_size_;
		}

		bool Read_Sector(DWORD sector);
	private:
		VirtualReader VirtualReader_;
		DWORD BPS_ = 512;
		LONGLONG offset_;
		BYTE * fat_table_;
		DWORD table_size_;
		vector<bool> Sector_Array_;
	};

	typedef std::shared_ptr<Fat_Table> FatTable;

	DWORD toDWORD(const WORD low, const WORD high);

	/////////////////////////////// FatFS //////////////////////////////////


	class  FatFileSystem
		: public AbstractFileSystem
	{
	public:
		explicit FatFileSystem(const VirtualReader & );
		virtual ~FatFileSystem();

		virtual	bool mount( const PartitionEntry &);
		virtual bool getFirst( DirectoryEntry & head_folder, NodeEntry & input_node);
		//virtual bool getFirst( DirectoryNode * head_folder, NodeEntry & input_node ); 
		virtual bool getNext( NodeEntry & next_node );

		void read_root();
		void read_folder( const DirectoryEntry & head_folder);
		//void read_folder( DirectoryNode * head_folder);
		bool ReadCluster(Cluster & cluster);
		DWORD clusterSector(const DWORD cluster) const;
		LONGLONG clusterSector(const LONGLONG cluster) const;

		void addEntry(const wstring & nameEntry , msdos_dir_entry * pDirEntry);
		void addEntry(const string & nameEntry , msdos_dir_entry * pDirEntry);
		
		bool setOffset(FileEntry file_entry, LONGLONG offset);
		virtual bool ReadFile( const FileEntry file_entry, BYTE * data, DWORD data_size, DWORD &bytes_read);
		DWORD readSizeUsingTable(const FileEntry file_entry);
		void ReadUsingFatTable(const FileEntry file_entry, BYTE * returned_data, DWORD & bytes_read);


	private:
		DirectoryEntry current_folder_;
		PartitionEntry partition_entry;
		VirtualReader reader_;
		FatBootPtr BootSector_;
		FatTable FatTable_;
	};

	typedef std::shared_ptr<FatFileSystem> FatFS;

	bool ReadListClusters(File_Handle * pFileHandle, FatTable & fat_table,  DWORD cluster);
	void appendString(wstring & currentString, msdos_long_dir_entry* pLogDirEntry);
	DWORD  sectorsFromSize(const DWORD data_size, const DWORD sector_size);

	//class FSExport IFileSysten
	//{
	//public:
	//	IFileSysten() { /**/ };
	//	virtual ~IFileSysten() { /**/ };
	//	virtual bool ReadRootDir() = 0;
	//	virtual bool ReadDir() = 0;

	//};


	//class FSExport Fat_Reader
	//{
	//public:
	//	Fat_Reader(const VirtualReader & pVirualReader, const Boot_Fat32 & boot_sector);
	//	~Fat_Reader();
	//	void CreateFatTables();
	//	virtual bool ReadRootDir(DirectoryEntry & root_directory);
	//	virtual bool ReadDir(DirectoryEntry & directory);
	//	DWORD clusterSector(const DWORD cluster) const;
	//private:
	//	VirtualReader VirtualReader_;
	//	Boot_Fat32 BootSector_;
	//	FatTable FatTable_;
	//};

}