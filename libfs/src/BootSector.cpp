
#include "fs/BootSector.h"
//#include "io/constants.h"
#include "io/constants.h"


bool FileSystem::CheckSignature(const BYTE *BootSector)
{
	WORD * pSignature = (WORD *) &BootSector[boot_signature_offset];
	return (*pSignature == boot_signature) ? true : false;
}

FileSystem::FSType FileSystem::GetFATType(const fat_boot_sector & BootSector)
{
	if (BootSector.BytesPerSector == 0)
		return Unknown;
	if (BootSector.sec_per_clus == 0)
		return Unknown;
	DWORD RootDirSectors = ( (BootSector.root_entries * 32) + (BootSector.BytesPerSector - 1) ) / BootSector.BytesPerSector;

	DWORD fat_sectors = ( BootSector.fat_length_small == 0 ) ? BootSector.fat_length_large : BootSector.fat_length_small;
	DWORD total_sectors = ( BootSector.total_sect_small == 0 ) ? BootSector.total_sect_large : BootSector.total_sect_small;

	DWORD Data_Sectors = total_sectors - (BootSector.reserved_sect + (BootSector.number_fats * fat_sectors)) + RootDirSectors;


	DWORD count_clusters = Data_Sectors / BootSector.sec_per_clus;

	FSType iFatType = FAT12;

	if (count_clusters < 4085)
		iFatType = FAT12;
	else
		if (count_clusters < 65525)
			iFatType = FAT16;
		else
			iFatType = FAT32;

	return iFatType;
}

FileSystem::Fat16_boot::Fat16_boot(const fat_boot_sector & BootSector)
	: BootSector_(BootSector)
{
}
void FileSystem::Fat16_boot::InitBoot()
{
	Sectors_ = BootSector_.total_sect_small;
	FatSectors_ = BootSector_.fat_length_small;
}
string FileSystem::Fat16_boot::TypeName() const
{
	return fat16_name;
}
WORD FileSystem::Fat16_boot::Sectors() const
{
	return Sectors_;
}
WORD FileSystem::Fat16_boot::FatSectors() const
{
	return FatSectors_;
}


FileSystem::Fat32_boot::Fat32_boot(const fat_boot_sector & BootSector)
	: BootSector_(BootSector)
	, CountCluster_(0)
	, DataSectors_(0)
	, RootEntries_(0)
{
}
FileSystem::Fat32_boot::Fat32_boot(const BYTE *BootSector )
	: CountCluster_(0)
	, DataSectors_(0)
	, RootEntries_(0)
{
	fat_boot_sector * pBootSector = ( fat_boot_sector *) BootSector;
	BootSector_ = *pBootSector;
}

void FileSystem::Fat32_boot::InitBoot()
{
	DWORD total_sectors = sectors();
	if (BootSector_.BytesPerSector == 0)
		BootSector_.BytesPerSector = default_sector_size;

	RootEntries_ = ( (BootSector_.root_entries * 32) + (BootSector_.BytesPerSector - 1) ) / BootSector_.BytesPerSector;

	DataSectors_ =  total_sectors - (BootSector_.reserved_sect + (BootSector_.number_fats * BootSector_.fat_length_large)) + RootEntries_;
	CountCluster_ = DataSectors_ / BootSector_.sec_per_clus  ;
}
string FileSystem::Fat32_boot::TypeName() const
{
	return fat32_name;
}
DWORD FileSystem::Fat32_boot::sectors() const
{
	return BootSector_.total_sect_large;
}
DWORD FileSystem::Fat32_boot::sectors_per_fat() const
{
	return BootSector_.fat_length_large;
}
DWORD FileSystem::Fat32_boot::count_cluster() const
{
	return CountCluster_;
}
DWORD FileSystem::Fat32_boot::root_cluster() const
{
	return BootSector_.root_cluster;
}
DWORD FileSystem::Fat32_boot::data_sectors() const
{
	return BootSector_.reserved_sect + (BootSector_.number_fats * BootSector_.fat_length_large);
}
DWORD FileSystem::Fat32_boot::cluster_size() const
{
	return (BootSector_.BytesPerSector * BootSector_.sec_per_clus);
}
DWORD FileSystem::Fat32_boot::bytes_per_sector() const
{
	return BootSector_.BytesPerSector;
}
DWORD FileSystem::Fat32_boot::reserved_sectors() const
{
	return BootSector_.reserved_sect;
}
DWORD FileSystem::Fat32_boot::hidden() const
{
	return BootSector_.hidden;
}
BYTE FileSystem::Fat32_boot::sectors_per_cluster() const
{
	return BootSector_.sec_per_clus;
}