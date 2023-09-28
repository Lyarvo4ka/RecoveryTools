
#include "io/dbf.h"
#include "io/finder.h"



void fixAllDbfFiles(const IO::path_string & folder)
{
	IO::Finder finder;
	finder.add_extension(L".dbf");
	finder.FindFiles(folder);

	for (auto filename : finder.getFiles())
	{
		IO::fixDBF(filename);
	}
}

void createManyExcelFiles()
{
	IO::path_string src_excel = LR"(d:\PaboTa\tmp\23.����� 2021 ����.xls)";
	IO::path_string target_folder = LR"(f:\xls\)";
	IO::path_string img_file = LR"(y:\49655\49655.img)";

	IO::File img(img_file);
	img.OpenRead();
	IO::File src(src_excel);
	src.OpenRead();

	const uint32_t write_pos = 20480;
	const uint32_t mod_count = 10000;

	const uint32_t cluster_size = 4096;

	uint64_t offset = 0;

	IO::DataArray excel_data(src.Size());
	src.ReadData(excel_data);

	IO::DataArray cluster(cluster_size);

	uint64_t conter = 0;

	uint64_t folder_counter = 0;


	while (offset < img.Size())
	{
		img.ReadData(cluster);
		memcpy(excel_data.data() + write_pos, cluster.data(), cluster.size());
		if (IO::isNot00orFF(cluster.data(), cluster.size()))
		{
			if (conter % mod_count == 0)
			{
				folder_counter++;
				IO::path_string new_folder = target_folder + std::to_wstring(folder_counter);
				fs::create_directories(new_folder);
			}

			IO::path_string target_filename = target_folder + std::to_wstring(folder_counter) + L"\\" + std::to_wstring(conter++) + L".xls";
			IO::File target(target_filename);
			target.OpenCreate();
			target.WriteData(excel_data.data(), excel_data.size());
			target.Close();
		}
		offset += cluster_size;
	}







}

#include "io/fs.h"

void splitToFiles(const IO::path_string & sourcePath , const IO::path_string& folder  )
{
	IO::File sourceFile(sourcePath);
	sourceFile.OpenRead();

	const uint32_t MaxFiles = 4;
	const uint32_t PageSize = 528;

	std::vector<IO::File > targetFiles;
	for (uint32_t i = 0; i < MaxFiles; ++i)
	{
		IO::File targetFile(folder + std::to_wstring(i));
		targetFiles.emplace_back(targetFile);
	}
	for (auto& targetFile : targetFiles)
	{
		targetFile.OpenCreate();
	}



	IO::DataArray buffer(PageSize);

	uint64_t offset = 0;
	while (offset < sourceFile.Size())
	{
		sourceFile.setPosition(offset);
		sourceFile.ReadData(buffer);
		
		uint8_t val0 = buffer[0];

		if (val0 < MaxFiles)
		{
			targetFiles[val0].WriteData(buffer.data() + 4, 512);
		}

		offset += PageSize;
	}

}

void splitByPage()
{
	const uint32_t fullSize = 4096;
	IO::DataArray fullPage(fullSize);

	const IO::path_string sourcePath = LR"(y:\51236\full\full_image.bin)";
	IO::File sourceFile(sourcePath);
	sourceFile.OpenRead();

	const IO::path_string smallPath = LR"(y:\51236\full\small.dump)";
	IO::File smallFile(smallPath);
	smallFile.OpenCreate();

	const IO::path_string bigPath = LR"(y:\51236\full\big.dump)";
	IO::File bigFile(bigPath);
	bigFile.OpenCreate();

	uint64_t offset = 0;
	while (offset < sourceFile.Size())
	{
		sourceFile.setPosition(offset);
		sourceFile.ReadData(fullPage);

		smallFile.WriteData(fullPage.data(), 96);
		bigFile.WriteData(fullPage.data() + 96, 4000);
		offset += fullSize;
	}
	
}

#include "io/onec.h"

#include <fstream>

void readAddressSaveToFile(std::string & addressFilename, const IO::path_string & sourceFilename, const IO::path_string & targetFilename)
{
	std::ifstream addrFile(addressFilename.c_str());

	std::string str;
	std::list<std::string> listAddr;
	while (std::getline(addrFile, str))
		listAddr.emplace_back(str);

	IO::File source(sourceFilename);
	source.OpenRead();

	IO::File target(targetFilename);
	target.OpenCreate();

	const uint32_t buff_size = 9437184*2;

	IO::DataArray buff(buff_size);

	for (const auto& offset_txt : listAddr)
	{
		auto offset = stoll(offset_txt, nullptr, 16);
		source.setPosition(offset);
		source.ReadData(buff);

		target.WriteData(buff.data(), buff.size());


	}

}

#include <map>

template<typename T>
typename T::value_type most_frequent_element(T const& v)
{    // Precondition: v is not empty
	std::map<typename T::value_type, int> frequencyMap;
	int maxFrequency = 0;
	typename T::value_type mostFrequentElement{};
	for (auto&& x : v)
	{
		int f = ++frequencyMap[x];
		if (f > maxFrequency)
		{
			maxFrequency = f;
			mostFrequentElement = x;
		}
	}

	return mostFrequentElement;
}

void savePopularBlock(const IO::path_string& source_filename, uint32_t block_size)
{
	const uint32_t kb = 1024;
	const uint32_t nCount = 16;

	IO::File source(source_filename);
	source.OpenRead();

	IO::File target(source_filename + L".target");
	target.OpenCreate();

	IO::DataArray src_buff(block_size);
	IO::DataArray target_buff(block_size);
	uint64_t offset = 0;



	while (offset < source.Size())
	{
		source.setPosition(offset);
		source.ReadData(src_buff);

		std::vector< uint8_t > arrVall(nCount , 0x00);


		for (uint32_t i = 0; i < nCount; i+= kb)
		{
			uint8_t val = 0;
			for (auto iByte = 0; iByte < kb; ++iByte)
				val ^= src_buff[i + iByte];

			arrVall.at(i) = val;
		}
		auto max_popular = most_frequent_element(arrVall);
		for ( auto i = 0 ; i < arrVall.size(); ++i)
			if (max_popular == arrVall.at(i))
			{
				for (auto j = 0; j < target_buff.size(); j += kb)
				{
					memcpy(target_buff.data() + j, src_buff.data() + i * kb, kb);
				}
			}

		target.WriteData(target_buff.data(), target_buff.size());
		offset += src_buff.size();
	}

}


void modify_mft_by_id(const IO::path_string& src_filename)
{
	IO::File source(src_filename);
	source.OpenRead();

	IO::File target(src_filename + L".result");
	target.OpenCreate();

	const uint32_t EntrySize = 1024;

	IO::DataArray buff(EntrySize);
	uint64_t offset = 0;
	IO::DataArray target_buff(EntrySize);

	const uint32_t ntfs_id_offset = 44;

	while (offset < source.Size())
	{
		source.setPosition(offset);
		source.ReadData(buff);

		if (memcmp(Signatures::FILE0, buff.data(), Signatures::sizeFILE0) == 0)
		{
			uint32_t* pNTFS_id = (uint32_t*)(buff.data() + ntfs_id_offset);
			if ((*pNTFS_id * 2) < source.Size())
			{
				target.setPosition(*pNTFS_id * EntrySize);
				target.WriteData(buff.data(), buff.size());
			}
		}
		offset += EntrySize;
	}


}

void modify_mft(const IO::path_string& src_filename)
{
	IO::File source(src_filename);
	source.OpenRead();

	IO::File target(src_filename + L".result");
	target.OpenCreate();

	const uint32_t EntrySize = 1024;

	IO::DataArray buff(EntrySize);
	uint64_t offset = 0;
	IO::DataArray target_buff(EntrySize);

	while (offset < source.Size())
	{
		source.setPosition(offset);
		source.ReadData(buff);

		if (memcmp(Signatures::FILE0, buff.data(), Signatures::sizeFILE0) == 0)
			memcpy(target_buff.data(), buff.data(), EntrySize);
		else
			memcpy(target_buff.data(), buff.data()+ Signatures::sizeFILE0, EntrySize- Signatures::sizeFILE0);

		target.WriteData(target_buff.data(), target_buff.size());
		offset += EntrySize;
	}
	

}

void modifyAVI(const IO::path_string& src_filename)
{
	IO::File source(src_filename);
	source.OpenWrite();

	auto new_size = source.Size() - 8;
	source.setSize(new_size);
	source.Close();


}

void renameToCR3(const IO::path_string& src_filename)
{
	const char CR3_Header[] = { 0x66 , 0x74 , 0x79 , 0x70 , 0x63 , 0x72 , 0x78 , 0x20 };
	const uint32_t CR3_Header_size = SIZEOF_ARRAY(CR3_Header);

	IO::File source(src_filename);
	source.OpenRead();

	IO::DataArray buff(default_sector_size);
	source.ReadData(buff);
	source.Close();

	if (memcmp(buff.data() + 4 , CR3_Header, CR3_Header_size) == 0)
		std::filesystem::rename(src_filename, src_filename + L".CR3");

	



}

#include "io/diskdevice.h"

void xor_disks( const std::vector<int> &nubers, const IO::path_string& outFile, const uint64_t offset , const uint32_t xor_size = 1024)
{
	auto listDisk = IO::ReadPhysicalDrives();
	IO::DataArray xor_buff (xor_size);
	ZeroMemory(xor_buff.data(), xor_buff.size());

	IO::DataArray buff(xor_size);
	for (auto diskNumber : nubers)
	{
		auto diskPtr = listDisk.find_by_number(diskNumber);
		if (diskPtr == nullptr)
		{
			printf("Error find disk %d", diskNumber);
			return;
		}

		auto disk = IO::DiskDevice(diskPtr);
		disk.Open(IO::OpenMode::OpenRead);

		disk.setPosition(offset);
		disk.ReadData(buff.data(), buff.size());

		for (auto i = 0; i < xor_buff.size(); ++i)
		{
			xor_buff[i] ^= buff[i];
		}
	}

	IO::File target(outFile);
	target.OpenCreate();
	target.WriteData(xor_buff.data(), xor_buff.size());


}



void WriteFileToHDD(const IO::path_string& sourceFilename,
	uint64_t filePosition,
	uint64_t partition_offset,
	uint64_t target_write_position,
	uint32_t drive_number)
{
	IO::File sourceFile(sourceFilename);
	sourceFile.OpenRead();

	auto listDisk = IO::ReadPhysicalDrives();
	auto diskPtr = listDisk.find_by_number(drive_number);
	if (diskPtr == nullptr)
	{
		printf("Error find disk %d", drive_number);
		return;
	}

	auto disk = IO::DiskDevice(diskPtr);
	disk.Open(IO::OpenMode::OpenWrite);

	uint64_t targetWriteOffset = partition_offset + target_write_position - filePosition/default_sector_size;

	uint64_t offset = 0;
	uint64_t write_offset = targetWriteOffset* default_sector_size;
	uint32_t block_size = 0;
	IO::DataArray buff(default_block_size);

	while (offset < sourceFile.Size())
	{
		block_size = IO::calcBlockSize(offset, sourceFile.Size(), default_block_size);
		sourceFile.setPosition(offset);
		sourceFile.ReadData(buff.data(), block_size);

		disk.setPosition(write_offset);
		disk.WriteBlock(buff.data(), block_size);

		offset += block_size;
		write_offset += block_size;
	}

}

#include "raw/qtfragment.h"
int wmain(int argc, wchar_t* argv[])
{

	//if (argc == 6)
	//{
	//	IO::path_string sourceFile = argv[1];
	//	uint64_t filePosition = std::stoll(argv[2]);
	//	uint64_t target_write_position = std::stoll(argv[3]);
	//	uint64_t partition_offset = std::stoll(argv[4]);
	//	uint32_t drive_number = std::stoll(argv[5]);

	//	WriteFileToHDD(sourceFile, filePosition, partition_offset, target_write_position, drive_number);
	//}
	//else
	//	std::cout << "wrong params" << std::endl;

	//IO::path_string filepath = LR"(c:\53239\53239.img )";
	//RAW::SavePos_ftyp_mdat(filepath);

	IO::path_string sourcefile = LR"(c:\53239\53239.img)";
	IO::path_string ftyppath = LR"(c:\53239\53239.img.ftyp)";
	IO::path_string mdatpath = LR"(c:\53239\53239.img.mdat)";
	IO::path_string targetFolder = LR"(z:\53239\Data\)";
	RAW::saveQtFragment(sourcefile, ftyppath, mdatpath, targetFolder);
	//std::vector<int> listDisk = { 3,4,5,10,8};
	//IO::path_string target = LR"(d:\52598\xor)";
	//uint64_t offset = (uint64_t)77323 * 64*1024;
	//xor_disks(listDisk, target, offset);
	// 
	//IO::path_string mft_filename = LR"(d:\52778\mft_part1)";
	//modify_mft_by_id(mft_filename);
	//IO::RestoreRootObject();

	//IO::path_string file1 = LR"(f:\51900\Program Files\Microsoft SQL Server\MSSQL11.MSSQLSERVER\MSSQL\Data\ANDBUH.mdf)";
	//IO::path_string file2 = LR"(f:\51900\Program Files\Microsoft SQL Server\MSSQL11.MSSQLSERVER\MSSQL\Data\ANDBUH.mdf_old)";
	//IO::path_string result = LR"(f:\51900\Program Files\Microsoft SQL Server\MSSQL11.MSSQLSERVER\MSSQL\Data\ANDBUH_result)";

	//modify_mft(file1);
	//IO::DataArray bad_sector_marker(Signatures::bad_sector_header_size);
	//memcpy(bad_sector_marker.data(), Signatures::bad_sector_header, Signatures::bad_sector_header_size);
	//IO::replaceBadsFromOtherFile(file1, file2, result , bad_sector_marker , 512);

	//readAddressSaveToFile(addrFile, src, dst);
	//savePopularBlock(src, 16 * 1024);

	//IO::RestoreRootObject();


	//IO::path_string foldername = LR"(e:\52394\video\mov\)";
	//IO::Finder finder;
	//finder.FindFiles(foldername);
	//for (const auto& fileName : finder.getFiles())
	//{
	//	renameToCR3(fileName);
	//	//modifyAVI(fileName);

	//}

	//IO::path_string filename = LR"(c:\tmp\audio.bin)";
	//IO::path_string foldername = LR"(z:\52696\Partition2\!Problem\)";
	//splitToFiles(filename, foldername);

	
	//createManyExcelFiles();
	//splitByPage();
	//IO::Finder finder;
	//finder.FindFiles(foldername);
	//for (const auto& fileName : finder.getFiles())
	//{
	//	IO::moveToDateFolder(fileName, foldername);
	//}
	//IO::path_string foldername = LR"(c:\tmp\dbf\)";
	//fixAllDbfFiles(foldername);

	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return 0;
}
