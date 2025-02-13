
#include "fs/VirtualReader.h"

#include "fs/const_fs.h"
#include <assert.h>

FileSystem::Cluster::Cluster(const DWORD size, const DWORD number)
	: size_(size)
	, number_(number)
	, data_(NULL)

{
	data_ = new BYTE[size_];
}
FileSystem::Cluster::~Cluster()
{
	if (data_ != NULL)
	{
		delete [] data_;
	}
}
void FileSystem::Cluster::setNumber(const DWORD number)
{
	number_ = number;
}
BYTE * FileSystem::Cluster::data() 
{
	return data_;
}
DWORD FileSystem::Cluster::size() const
{
	return size_;
}
DWORD FileSystem::Cluster::number() const
{
	return number_;
}


/************************************************************************/
/*							 CSectorReader								*/
/************************************************************************/

FileSystem::CSectorReader::CSectorReader(IO::IODevicePtr device, const DWORD sector_size)
	: device_(device)
	, sector_size_(sector_size)
	, position_(0)
{

}
FileSystem::CSectorReader::~CSectorReader()
{

}

bool FileSystem::CSectorReader::Open()
{
	return device_->Open(IO::OpenMode::OpenRead);
}
bool FileSystem::CSectorReader::isReady() const
{
	if (sector_size_ == 0)
		return false;
	return device_->isOpen() ;
}
bool FileSystem::CSectorReader::ReadSector(BYTE * sector_data, LONGLONG sector_number) 
{
	assert(sector_data != NULL);
	position_ = (LONGLONG)(sector_number * sector_size_);
	device_->setPosition(position_);
	device_->ReadData(sector_data, sector_size_);
	return true;
}
bool FileSystem::CSectorReader::ReadSectors(BYTE * sector_data, LONGLONG sector_number , const DWORD count_sectors) 
{
	assert(sector_data != NULL);
	position_ = (LONGLONG)(sector_number * sector_size_);
	DWORD readSize =  count_sectors * sector_size_;
	device_->setPosition(position_);
	device_->ReadData(sector_data, readSize);
	return true;
}
//FileSystem::CVirtualReader::CVirtualReader(IReader * pReader)
//	: Reader_(pReader)
//	, BytesPerSector_(default_sector_size)
//	, ReservSectors_(0)
//	, DataOffset_(0)
//{
//
//}
//FileSystem::CVirtualReader::~CVirtualReader()
//{
//
//}
//DWORD FileSystem::CVirtualReader::Read( BYTE * pReadBuffer, const DWORD ReadSize, const LONGLONG Position)
//{
//	assert(Reader_ != NULL);
//	return Reader_->Read(pReadBuffer,ReadSize, Position);
//}
//DWORD FileSystem::CVirtualReader::Read(CBuffer * pBuffer, const LONGLONG Position)
//{
//	return Reader_->Read(pBuffer,Position);
//}
//bool FileSystem::CVirtualReader::ReadSector(BYTE * pSector, LONGLONG SectorNumber)
//{
//	assert(pSector != NULL);
//	assert(BytesPerSector_ != 0);
//	SectorNumber *= BytesPerSector_;
//	return (Reader_->Read(pSector,BytesPerSector_,SectorNumber) != 0) ? true : false;
//}
//DWORD FileSystem::CVirtualReader::ReadSectors(BYTE * pSectorData, LONGLONG SectorNumber , const DWORD SectorCounts)
//{
//	SectorNumber *= BytesPerSector_;
//	DWORD bytesRead = Reader_->Read(pSectorData, SectorCounts , SectorNumber) ;
//
//	return bytesRead;
//}
//bool FileSystem::CVirtualReader::read_cluster(Cluster * pCluster, const DWORD sector_from)
//{
//	if (pCluster == NULL)
//		return false;
//
//	if (ReadSectors(pCluster->data(), sector_from, pCluster->size()) == 0)
//		return false;
//	
//	return true;
//}
//void FileSystem::CVirtualReader::SetSectorSize(const DWORD BytesPerSector)
//{
//	BytesPerSector_ = BytesPerSector;
//}
//void FileSystem::CVirtualReader::SetReservedSectors(const DWORD ReservedSectors)
//{
//	ReservSectors_ = ReservedSectors;
//}
//void FileSystem::CVirtualReader::SetDataOffset(const DWORD data_offset)
//{
//	DataOffset_ = data_offset;
//}