
#include "fs/fat_fs.h"
#include "fs/const_fs.h"

#include <assert.h>



FileSystem::MasterBootRecord::MasterBootRecord(const DWORD offsetMBR)
	: mbr_offset_(offsetMBR)
{

}
FileSystem::MasterBootRecord::~MasterBootRecord()
{
	//DEBUG_SHOW("Destructor [\"MasterBootRecord\"]...\r\n");	
}
void FileSystem::MasterBootRecord::setOffset( const DWORD offsetMBR)
{
	mbr_offset_ = offsetMBR;
}
bool FileSystem::MasterBootRecord::open( const VirtualReader & virtual_reader)
{
	//if (!virtual_reader->isReady())
	//	return false;

	BYTE mbr_data[mbr_size];
	ZeroMemory( mbr_data , sizeof(BYTE) * mbr_size );
	

	if ( !virtual_reader->ReadSector(mbr_data, mbr_offset_) )
		return false;
	
	// checking to signature 0x55AA
	if ( !CheckSignature(mbr_data) )
		return false;

	// testing if it's boot sector !!!!!!!!!
	fat_boot_sector * pFatBootSector = (fat_boot_sector *) mbr_data;

	//if ( GetFATType(*pFatBootSector) == Unknown )
	//{

	partition_enty * pPartition = (partition_enty*)  &mbr_data[partition_offset] ;
	for ( auto iNumber = 1; iNumber < partition_count; ++iNumber)
	{
		if ( pPartition->partition_type != partition_types::free )
		{
			if ( pPartition->partition_type != partition_types::extended_lba )
			{
				partition_enty * add_partition = new partition_enty(*pPartition);
				PartitionArray_.push_back( PartitionEntry(new CPartition(pPartition->start_LBA,pPartition->sector_count) ) );
			}
		}
		++pPartition;
	}
	//}
	//else
	//{
	//	// Detecting boot sector and add this partition
	//	Fat32_boot BootFAT32(*pFatBootSector);
	//	BootFAT32.InitBoot();
	//	PartitionArray_.push_back( PartitionEntry( new CPartition(BootFAT32.hidden(),BootFAT32.sectors()) ) ) ;
	//}

	return true;
}
DWORD FileSystem::MasterBootRecord::count() const
{
	return PartitionArray_.size();
}
FileSystem::PartitionEntry FileSystem::MasterBootRecord::getPartition( const DWORD number ) const
{
	return ( number < PartitionArray_.size() ) 
				?	PartitionArray_[number]
				: NULL ;	
}

/************************************************************************/
/*								Fat_Table                               */
/************************************************************************/
FileSystem::Fat_Table::Fat_Table(const VirtualReader & pVirualReader, const LONGLONG offset)
	: VirtualReader_(pVirualReader)
	, offset_(offset)
	, fat_table_(NULL)
	, table_size_(0)
{
	//InitTable();
}
FileSystem::Fat_Table::~Fat_Table()
{
//	DEBUG_SHOW("Destructor [\"Fat_Table\"]...\r\n");	
	if (fat_table_ != NULL)
	{
		delete [] fat_table_;
		fat_table_ = NULL;
	}
	table_size_ = 0;
}
void FileSystem::Fat_Table::InitTable(const FatBootPtr &  boot_sector)
{
	BPS_ = boot_sector->bytes_per_sector();
	table_size_ = boot_sector->sectors_per_fat() * BPS_;
	fat_table_ = new BYTE[table_size_];
	Sector_Array_.resize(boot_sector->sectors_per_fat(),false);

}
DWORD FileSystem::Fat_Table::Next_Cluster(DWORD current_cluster)
{
	DWORD sector_number = current_cluster*4 / BPS_;
	if (sector_number >= Sector_Array_.size())
		return bad_cluster_fat32;
	if (!Sector_Array_.at(sector_number))
	{
		if (!Read_Sector(sector_number))
			return bad_cluster_fat32;

		Sector_Array_.at(sector_number) = true;
	}

	DWORD * pNexCluster = (DWORD*) &fat_table_[ current_cluster * fat32_cell ];
	*pNexCluster &= 0x0FFFFFFF;
	return *pNexCluster;
}
bool FileSystem::Fat_Table::Read_Sector(DWORD sector)
{
	BYTE * pSectorNumber = &fat_table_[sector*BPS_];
	LONGLONG read_position = offset_ + sector;
	return VirtualReader_->ReadSector(pSectorNumber, read_position);
}


DWORD FileSystem::toDWORD(const WORD low, const WORD high)
{
	DWORD result = low; 
	return (DWORD)( high << 16 | low );
}




/************************************************************************/
/*								FatFS				                    */
/************************************************************************/
FileSystem::FatFileSystem::FatFileSystem(const VirtualReader & virtual_reader )
	:reader_(virtual_reader)
	//, current_folder_()
{
	//ZeroMemory( fat32boot_ , sizeof(Fat32_boot));
}
FileSystem::FatFileSystem::~FatFileSystem()
{

}
 bool FileSystem::FatFileSystem::mount( const PartitionEntry & partition)
 {
	 if ( !reader_->isReady() )
	 {
		 if ( !reader_->Open())
			 return false;
	 }
	 partition_entry = partition;
	 // read boot sector
	 BYTE boot_sector[default_sector_size];
	 ZeroMemory(boot_sector , default_sector_size);
	 if ( !reader_->ReadSector(boot_sector,partition->start_sector()) )
		 return false;

	 BootSector_ = FatBootPtr( new Fat32_boot(boot_sector) );
	 BootSector_->InitBoot();
	 
	 LONGLONG OffsetToTable = partition->start_sector();
	 OffsetToTable += BootSector_->reserved_sectors();
	 FatTable_ = FatTable( new Fat_Table(reader_,OffsetToTable) );
	 FatTable_->InitTable(BootSector_);
	 return true;
 }
//bool FileSystem::FatFileSystem::getFirst( DirectoryNode * head_folder, NodeEntry & input_node)
//{
//	assert(head_folder != NULL);
//	if (current_folder_ != head_folder)
//		current_folder_ = head_folder;
//	if (head_folder->count() == 0)
//	{
//		//if ( head_folder->parent() == NULL )
//		//	read_root();
//		//else
//			read_folder(head_folder);
//	}
//	input_node = current_folder_->first();
//	return true;
//		
//}
bool FileSystem::FatFileSystem::getFirst( DirectoryEntry & head_folder, NodeEntry & input_node)
{
	//return this->getFirst( head_folder.get() , input_node);
	assert(head_folder != NULL);
	if (current_folder_ != head_folder)
		current_folder_ = head_folder;
	if (head_folder->count() == 0)
	{
		//if ( head_folder->parent() == NULL )
		//	read_root();
		//else
		read_folder(head_folder);
	}
	input_node = current_folder_->first();
	return input_node ? true : false;

}
//bool FileSystem::FatFileSystem::getFirst( DirectoryNode * head_folder, NodeEntry & input_node)
//{
//	//assert(head_folder != NULL);
//	//if (current_folder_ != head_folder)
//	//	current_folder_ = head_folder;
//	//if (head_folder->count() == 0)
//	//{
//	//	if ( head_folder->parent() == NULL )
//	//		read_root();
//	//	else
//	//		read_folder(head_folder);
//	//}
//	//input_node = current_folder_->first();
//	return true;
//
//}
bool FileSystem::FatFileSystem::getNext( NodeEntry & next_node )
{
	return (next_node = current_folder_->next()) ? true : false;
}
void FileSystem::FatFileSystem::read_root()
{
	//current_folder_ 


	//Cluster rootCluster(BootSector_->cluster_size());
	//DWORD currentCluster = BootSector_->root_cluster();
	//DWORD iNumber = 0;
	//do 
	//{
	//	rootCluster.setNumber(currentCluster);
	//	ReadCluster(rootCluster);	
	//
	//	DWORD current_entry = 0;
	//	while (current_entry < rootCluster.size())
	//	{
	//		BYTE * pData = rootCluster.data();
	//		msdos_dir_entry * pDirEnry = (msdos_dir_entry *) &pData[current_entry];
	//
	//		iNumber = toDWORD( pDirEnry->cluster_low, pDirEnry->cluster_high );
	//		string strName(pDirEnry->name, ms_dos_name);
	//		strName.append("\0");

	//		if (pDirEnry->attr == attr_read_only ||
	//			pDirEnry->attr == attr_hidden ||
	//			pDirEnry->attr == attr_system ||
	//			/*pDirEnry->attr == attr_volume_id ||*/
	//			pDirEnry->attr == attr_archive
	//			)
	//		{
	//			FileSystem::FileEntry newFileEntry(new FileNode(strName.c_str(),pDirEnry->file_size));
	//			newFileEntry->setCluster(iNumber);
	//			newFileEntry->setFileDates( new EntryTimes( pDirEnry->data_create, pDirEnry->time_create,
	//														pDirEnry->date_modify, pDirEnry->time_modify,
	//														pDirEnry->date_access, 0, pDirEnry->time_create_ms ) );
	//			current_folder_->add_file(newFileEntry);
	//
	//		}
	//		else
	//		if (pDirEnry->attr == attr_directory)
	//		{
	//			FileSystem::DirectoryEntry newDirectory(new DirectoryNode(strName.c_str()) );
	//			newDirectory->setCluster(iNumber);
	//			newDirectory->setFolderDates( new EntryTimes( pDirEnry->data_create, pDirEnry->time_create,
	//														  pDirEnry->date_modify, pDirEnry->time_modify,
	//														  pDirEnry->date_access, 0, pDirEnry->time_create_ms )  );
	//			current_folder_->add_folder(newDirectory);
	//		}
	//
	//		current_entry += dir_entry_size;
	//	}
	//	currentCluster = FatTable_->Next_Cluster(currentCluster);
	//} while (currentCluster != end_cluster_fat32);
}

//void FileSystem::FatFileSystem::read_folder(const DirectoryEntry & head_folder)
//{
//	read_folder( head_folder.get() );
//
/////////////////////////////////////////////
////	do 
////	{
////		folderCluster.setNumber(currentCluster);
////		if  ( !ReadCluster(folderCluster))
////			break;
////
////		while (current_entry < folderCluster.size())
////		{
////			BYTE * pData = folderCluster.data();
////			msdos_dir_entry * pDirEnry = (msdos_dir_entry *) &pData[current_entry];
////			
////			iNumber = toDWORD( pDirEnry->cluster_low, pDirEnry->cluster_high );
////			string strName(pDirEnry->name, ms_dos_name);
////			strName.append("\0");
////			if (pDirEnry->attr == attr_read_only ||
////				pDirEnry->attr == attr_hidden ||
////				pDirEnry->attr == attr_system ||
////				/*pDirEnry->attr == attr_volume_id ||*/
////				pDirEnry->attr == attr_archive
////				)
////			{
////				FileSystem::FileEntry newFileEntry(new FileNode(strName.c_str(),pDirEnry->file_size));
////				newFileEntry->setCluster(iNumber);
////				current_folder_->add_file(newFileEntry);
////
////			}
////			else
////				if (pDirEnry->attr == attr_directory)
////				{
////					FileSystem::DirectoryEntry newDirectory(new DirectoryNode(strName.c_str()) );
////					newDirectory->setCluster(iNumber);
////					current_folder_->add_folder(newDirectory);
////				}
////
////				current_entry += dir_entry_size;
////		}
////		currentCluster = FatTable_->Next_Cluster(currentCluster);
////		//if (currentCluster == 0)
////	} while (currentCluster != end_cluster_fat32);
//
//}
void FileSystem::FatFileSystem::read_folder( const DirectoryEntry & head_folder)
{
	std::shared_ptr<Cluster> folderCluster( new Cluster(BootSector_->cluster_size()));

	auto max_cluster = FatTable_->size() / 4;

	DWORD currentCluster = 0;
	if ( head_folder->parent() == NULL)
		currentCluster = BootSector_->root_cluster();
	else
		currentCluster = head_folder->cluster();


	if (currentCluster > max_cluster)
		return;

	folderCluster->setNumber(currentCluster);

	if ( !ReadCluster(*folderCluster) )
		return ;

	DWORD current_entry = 0;
	if (head_folder->parent() != NULL)
	{
		msdos_dir_entry * pDirEnry = NULL;

		BYTE * pData = folderCluster->data();
		pDirEnry = (msdos_dir_entry *) &pData[current_entry];

		string sDot(pDirEnry->name, ms_dos_name);
		sDot.append("\0");
		// checking to '.' 
		if ( one_dot.compare(sDot) != 0)
			return;

		current_entry += dir_entry_size;
		pDirEnry = (msdos_dir_entry *) &pData[current_entry];

		// checking to '..' 
		string sDoubleDot(pDirEnry->name, ms_dos_name);
		sDoubleDot.append("\0");
		if (double_dot.compare(sDoubleDot) != 0)
			return;
		current_entry += dir_entry_size;
	}
	DWORD iNumber = 0;

	list<std::shared_ptr<Cluster> > folderData; 
	folderData.push_back( folderCluster );
	currentCluster = FatTable_->Next_Cluster(currentCluster);

	// check if in range of fat area


	if ( currentCluster != end_cluster_fat32 )
		do 
		{
			if (max_cluster < currentCluster)
				break;

			std::shared_ptr<Cluster> nextCluster( new Cluster(BootSector_->cluster_size()));
			nextCluster->setNumber(currentCluster);
			if  ( !ReadCluster(*nextCluster))
				break;

			folderData.push_back(nextCluster);

			currentCluster = FatTable_->Next_Cluster(currentCluster);
			if (currentCluster == 0)
				break;
		} while (currentCluster != end_cluster_fat32);

		DWORD folderSize = folderData.size() ;
		folderSize *= folderCluster->size();

		BYTE * pFolderData = new BYTE[folderSize];

		auto listIter = folderData.begin();
		for (std::size_t iCluster = 0; iCluster < BootSector_->cluster_size(); ++iCluster)
		{
			if (listIter == folderData.end())
				break;

			Cluster * clusterPtr = listIter->get();
			memcpy( pFolderData + BootSector_->cluster_size() *iCluster, clusterPtr->data(), clusterPtr->size());
			++ listIter;
		}

		wstring node_name;
		while (current_entry < folderSize)
		{
			node_name = L"";
			msdos_dir_entry * pDirEnry = (msdos_dir_entry *) &pFolderData[current_entry];
			BYTE firstSymbol = pDirEnry->name[0];
			if ( firstSymbol == 0xE5 /*|| firstSymbol == 0*/)
			{
				// it's recovery algorithm
			}
			else
			if (pDirEnry->attr == attr_long_name )
			{
				msdos_long_dir_entry * pLongEntry = (msdos_long_dir_entry*)&pFolderData[current_entry];
				if (pLongEntry->attr == attr_long_name)
				{
					appendString(node_name,pLongEntry);

					BYTE countEntries = pLongEntry->order_number ^ attr_last_log_entry;

					current_entry += dir_entry_size;
					for (BYTE iEntry = 1 ; iEntry < countEntries; ++iEntry)
					{
						pLongEntry = (msdos_long_dir_entry*)&pFolderData[current_entry];
						if (pLongEntry->attr != attr_long_name)
							break;
						appendString( node_name , pLongEntry );
						current_entry += dir_entry_size;
					}

					//for (std::size_t iPos = 0; iPos < node_name.length(); ++ iPos)
					//{
					//	if (node_name[iPos] == L'.')
					//		break;
					//	if (node_name[iPos] == 0)
					//		node_name[iPos] = L'.';
					//}
					//auto strIter = node_name.begin();
					//while (strIter != node_name.end())
					//{
					//	if ( *strIter == 0xFFFF)
					//		strIter = node_name.erase(strIter);
					//	else
					//		if ( *strIter == 0)
					//			strIter = node_name.erase(strIter);
					//		else
					//			++strIter;
					//}
					//node_name.append(L"\0");
					pDirEnry = (msdos_dir_entry *) &pFolderData[current_entry];

					addEntry(node_name,pDirEnry);
				}

			}
			else
			{
				string fileName;
				string fileExt;
				string strName(pDirEnry->name, 8);
				strName.append("\0");
				std::size_t left_pos = strName.find(0x20);
				if (left_pos != strName.npos)
				{
					fileName = strName.substr(0,left_pos);
				}
				else
					fileName = strName;

				string strExt(&pDirEnry->name[8],3);
				strExt.append("\0");
				
				left_pos = strExt.find(0x20);
				if (left_pos != strExt.npos)
				{
					fileExt = strExt.substr(0,left_pos);
				}
				else
					fileExt = strExt;
				//string strName(pDirEnry->name, 8);
				//strName.append("\0");
				//string tempStr;
				//std::size_t left_pos = strName.find(0x20);
				//if (left_pos != strName.npos)
				//{
				//	std::size_t right_pos = strName.rfind(0x20);
				//	if (right_pos == strName.length() - 1)
				//		tempStr = strName.substr(0,left_pos);
				//	else
				//		tempStr = strName.substr(0,left_pos) + '.' + strName.substr(right_pos+1,strName.length()-1);
				//}
				//else
				string tempStr;
				if (fileExt.size() > 0)
					tempStr = fileName + '.' + fileExt;
				else
					tempStr = fileName;

				std::transform(tempStr.begin(),tempStr.end(),tempStr.begin(),::tolower);

				addEntry(tempStr,pDirEnry);

			}

			current_entry += dir_entry_size;
		}

		delete pFolderData;
		pFolderData = NULL;
}
bool FileSystem::FatFileSystem::ReadCluster(Cluster & cluster)
{
	DWORD sector = partition_entry->start_sector();
	sector += clusterSector(cluster.number());
	return reader_->ReadSectors(cluster.data(),sector,BootSector_->sectors_per_cluster());
}
DWORD FileSystem::FatFileSystem::clusterSector(const DWORD cluster) const
{
	return ( cluster - 2 ) * BootSector_->sectors_per_cluster() + BootSector_->data_sectors();
}
LONGLONG FileSystem::FatFileSystem::clusterSector(const LONGLONG cluster) const
{
	return ( cluster - 2 ) * BootSector_->sectors_per_cluster() + BootSector_->data_sectors();
}
void FileSystem::FatFileSystem::addEntry(const wstring & nameEntry , msdos_dir_entry * pDirEntry)
{
	assert(pDirEntry != NULL);
	DWORD clusterNumber = toDWORD( pDirEntry->cluster_low, pDirEntry->cluster_high );
	if (pDirEntry->attr == attr_directory)
	{
		FileSystem::DirectoryEntry newDirectory(new DirectoryNode(nameEntry.c_str()) );
		newDirectory->setCluster(clusterNumber);
		newDirectory->setFolderDates( new EntryTimes( pDirEntry->data_create, pDirEntry->time_create,
			pDirEntry->date_modify, pDirEntry->time_modify,
			pDirEntry->date_access, 0, pDirEntry->time_create_ms ) );
		current_folder_->add_folder(newDirectory);
	}
	if ( pDirEntry->attr == attr_file || pDirEntry->attr == attr_archive || pDirEntry->attr == 0)
	{
		FileSystem::FileEntry newFileEntry(new FileNode(nameEntry.c_str(),pDirEntry->file_size));
		newFileEntry->setCluster(clusterNumber);
		newFileEntry->setFileDates( new EntryTimes( pDirEntry->data_create, pDirEntry->time_create,
			pDirEntry->date_modify, pDirEntry->time_modify,
			pDirEntry->date_access, 0, pDirEntry->time_create_ms ) );
		current_folder_->add_file(newFileEntry);
	}

}
void FileSystem::FatFileSystem::addEntry(const string & nameEntry , msdos_dir_entry * pDirEntry)
{
	wstring tempString(ToWideString(nameEntry));
	addEntry(tempString,pDirEntry);
}
bool FileSystem::FatFileSystem::setOffset(FileEntry file_entry, LONGLONG offset)
{
	//assert(file_entry != nullptr);

	if (File_Handle * pFileHanble = file_entry->getFileHandle())
	{
		if ( offset < file_entry->size() ) 
		{
			pFileHanble->setPosition( offset );
			return true;
		}
	}
	return false;
}
bool FileSystem::FatFileSystem::ReadFile( const FileEntry file_entry, BYTE * data, DWORD data_size, DWORD &bytes_read)
{
	////assert(file_entry != nullptr);
	////assert( data != nullptr );

	////assert ( (data_size % BootSector_->bytes_per_sector()) != 0);
	//bytes_read = 0;
	//DWORD remainingToRead = sectorsFromSize(data_size,BootSector_->bytes_per_sector());

	//if (File_Handle * pFileHandle = file_entry->getFileHandle())
	//{
	//	if (pFileHandle->getSize() == 0)
	//		FileSystem::ReadListClusters(pFileHandle,FatTable_,file_entry->cluster());

	//	auto list_cluster = pFileHandle->getClusterList();
	//	auto clusterIter = list_cluster->begin();
	//	if ( clusterIter == list_cluster->end() )
	//		return false;
	//	LONGLONG currentCluster = 0;
	//	LONGLONG fileCluster = 0;
	//	do
	//	{
	//		if (pFileHandle->getPosition() >= file_entry->size())
	//			break;

	//		//fileCluster = pFileHandle->getPosition();
	//		//fileCluster /= BootSector_->cluster_size();

	//		currentCluster = clusterIter->getNumber() /*+ fileCluster*/;

	//		//	set first current cluster
	//		//while ( clusterIter != pFileHandle->clusterList_.end() )
	//		//{
	//		//	currentCluster = (LONGLONG) (clusterIter->getNumber() + pFileHandle->position_ / BootSector_->cluster_size() );
	//		//	if ( currentCluster < (clusterIter->getNumber() + clusterIter->getCount()) )
	//		//		break;

	//		//	++clusterIter;
	//		//}

	//		// offset in Cluster...
	//		DWORD sectorPos = pFileHandle->getPosition() % BootSector_->cluster_size();
	//		LONGLONG logical_sector = clusterSector(currentCluster);
	//		LONGLONG next_sector = clusterSector(currentCluster + clusterIter->getCount());
	//		DWORD sectorsToRead = (DWORD)(next_sector - logical_sector);
	//		logical_sector += partition_entry->start_sector();

	//		// read file
	//		DWORD bytesToRead = sectorsToRead * BootSector_->bytes_per_sector();
	//		BYTE *pReadBuffer = new BYTE[bytesToRead];

	//		LONGLONG allToRead = pFileHandle->getPosition() + sectorsToRead * BootSector_->bytes_per_sector();
	//		if (allToRead > file_entry->size())
	//		{
	//			bytesToRead = static_cast<DWORD> (bytesToRead - (allToRead - file_entry->size()));
	//		}

	//		if (reader_->ReadSectors(pReadBuffer, logical_sector, sectorsToRead))
	//		{
	//		}
	//			//DEBUG_SHOW("read ok ");

	//		memcpy(data + bytes_read, pReadBuffer,bytesToRead);
	//		bytes_read += bytesToRead;

	//		delete pReadBuffer;

	//		pFileHandle->addPosition(bytesToRead);
	//		++clusterIter;
	//		if ( clusterIter == list_cluster->end() )
	//			break;


	//	} while ( bytes_read < data_size );
	//return true;
	//}
	//BYTE * pAlloc_data = nullptr;
	//DWORD bytes_returned = 0;
	ReadUsingFatTable(file_entry, data, bytes_read);
	return true;

return false;
}
DWORD FileSystem::FatFileSystem::readSizeUsingTable(const FileEntry file_entry)
{
	DWORD numClusters = 0;
	if (File_Handle * pFileHandle = file_entry->getFileHandle())
	{
		if (pFileHandle->getSize() == 0)
			if (file_entry->cluster() < BootSector_->count_cluster())
				FileSystem::ReadListClusters(pFileHandle, FatTable_, file_entry->cluster());

		auto cluster_list = pFileHandle->getClusterList();
		for (auto iter = cluster_list->begin(); iter != cluster_list->end(); ++iter)
			numClusters += iter->getCount();

		numClusters *= BootSector_->cluster_size();
	}
	return numClusters;
}
void FileSystem::FatFileSystem::ReadUsingFatTable(const FileEntry file_entry, BYTE * returned_data, DWORD & bytes_read)
{
	if (File_Handle * pFileHandle = file_entry->getFileHandle())
	{
		if (pFileHandle->getSize() == 0)
			FileSystem::ReadListClusters(pFileHandle, FatTable_, file_entry->cluster());


		LONGLONG currentCluster = 0;


		DWORD mem_offset = 0;

		uint64_t position = 0;
		auto cluster_list = pFileHandle->getClusterList();
		for (auto iter = cluster_list->begin(); iter != cluster_list->end(); ++iter)
		{
			currentCluster = iter->getNumber();
			DWORD sectorPos = pFileHandle->getPosition() % BootSector_->cluster_size();
			LONGLONG logical_sector = clusterSector(currentCluster);
			LONGLONG next_sector = clusterSector(currentCluster + iter->getCount());
			DWORD sectorsToRead = (DWORD)(next_sector - logical_sector);
			logical_sector += partition_entry->start_sector();

			if (logical_sector + sectorsToRead < BootSector_->sectors())
			{
				reader_->ReadSectors(returned_data + mem_offset, logical_sector, sectorsToRead);
				bytes_read += sectorsToRead * 512;
				mem_offset += (iter->getCount()* BootSector_->cluster_size());
			}
		}


	}

}
void FileSystem::appendString(wstring & currentString, msdos_long_dir_entry* pLogDirEntry)
{
	WCHAR entry_array[size_long_name ];
	ZeroMemory( entry_array , (size_long_name )*sizeof(WCHAR));
	wmemcpy( entry_array, pLogDirEntry->first_name, size_first_name );
	wmemcpy( entry_array + size_first_name, pLogDirEntry->second_name, size_second_name );
	wmemcpy( entry_array + (size_first_name + size_second_name) , pLogDirEntry->third_name, size_third_name );

	wstring tempStr;
	tempStr.append(entry_array,size_long_name);

	currentString.insert(0,tempStr.c_str());

}
DWORD FileSystem::sectorsFromSize(const DWORD data_size, const DWORD sector_size)
{
	assert(sector_size != 0);
	double count = (double)((double)data_size / (double)sector_size);
	return (DWORD) ceil (count);
}
bool FileSystem::ReadListClusters(File_Handle * pFileHandle, FatTable & fat_table, DWORD cluster)
{
	pFileHandle->clearList();
	ClusterEntry clusterEntry(cluster , 0);

	DWORD tempCluster = 0;
	while (cluster != end_cluster_fat32)
	{
		tempCluster = clusterEntry.getNumber() + clusterEntry.getCount();
		if ( tempCluster  == cluster)
			clusterEntry.inc();
		else
		{
			pFileHandle->add(clusterEntry);

			clusterEntry.setClusterData(cluster,0);
			clusterEntry.inc();
		}

		cluster = fat_table->Next_Cluster(cluster);
		if (cluster == bad_cluster_fat32)
			break;
	}
	if (clusterEntry.getCount() > 0)
		pFileHandle->add(clusterEntry);
	return true;
}

//
//FileSystem::Fat_Reader::Fat_Reader(const VirtualReader & pVirualReader, const Boot_Fat32 & boot_sector)
//	: VirtualReader_(pVirualReader)
//	, BootSector_(boot_sector)
//{
//	pVirualReader->SetDataOffset(boot_sector->data_sectors());
//	FatTable_ = FatTable( new Fat_Table(pVirualReader , boot_sector) );
//}
//FileSystem::Fat_Reader::~Fat_Reader()
//{
//}
//
//bool FileSystem::Fat_Reader::ReadRootDir(DirectoryEntry & root_directory)
//{
//	// Testing to EOC
//
//	Cluster rootCluster(BootSector_->cluster_size());
//	DWORD currentCluster = BootSector_->root_cluster();
//
//
//	//DirectoryEntry dir1(new DirectoryNode() );
//
//	//DirectoryEntry dir2 ;
//	//dir2 = dir1->parent();
//	//
//	DWORD dataSector = 0;
//
//	do 
//	{
//		dataSector = clusterSector(currentCluster);
//		VirtualReader_->read_cluster(&rootCluster,dataSector);	
//
//		DWORD current_entry = 0;
//		while (current_entry < rootCluster.size())
//		{
//			BYTE * pData = rootCluster.data();
//			msdos_dir_entry * pDirEnry = (msdos_dir_entry *) &pData[current_entry];
//
//			//switch (pDirEnry->attr)
//			//{
//			if (pDirEnry->attr == attr_read_only ||
//				pDirEnry->attr == attr_hidden ||
//				pDirEnry->attr == attr_system ||
//				pDirEnry->attr == attr_volume_id ||
//				pDirEnry->attr == attr_archive
//				)
//			{
//				FileSystem::FileEntry newFileEntry(new FileNode(pDirEnry->name,pDirEnry->file_size));
//				root_directory->add_file(newFileEntry);
//
//			}
//			else
//			if (pDirEnry->attr == attr_directory)
//			{
//				FileSystem::DirectoryEntry newDirectory(new DirectoryNode(pDirEnry->name) );
//				root_directory->add_folder(newDirectory);
//
//			}
//
//			current_entry += dir_entry_size;
//		}
//		currentCluster = FatTable_->Next_Cluster(currentCluster);
//	} while (currentCluster != end_cluster_fat32);
//
//	return ReadDir(root_directory);
//}
//bool FileSystem::Fat_Reader::ReadDir(DirectoryEntry & directory)
//{
//
//
//
//	return true;
//}

