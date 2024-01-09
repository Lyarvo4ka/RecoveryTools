
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
#include "io/constants.h"

void saveByPageNumber(const IO::path_string& sourcepath, const IO::path_string& tagetpath ,const uint64_t start_offset)
{
	const uint32_t PAGESIZE = 8192;

	IO::File sourceFile(sourcepath);
	sourceFile.OpenRead();

	IO::File targetFile(tagetpath);
	targetFile.OpenCreate();

	uint64_t offset = start_offset;

	IO::DataArray buff(PAGESIZE);
	sourceFile.setPosition(offset);
	sourceFile.ReadData(buff);
	uint32_t* page_idPTR = (uint32_t * ) (buff.data() + 4);
	uint32_t pageID = *page_idPTR;
	IO::toBE32(pageID);

	offset += buff.size();

	while (offset < sourceFile.Size())
	{
		sourceFile.setPosition(offset);
		sourceFile.ReadData(buff);
		page_idPTR = (uint32_t*)(buff.data() + 4);
		uint32_t nextID = *page_idPTR;
		IO::toBE32(nextID);
		if (nextID == (pageID + 1))
		{
			targetFile.WriteData(buff.data(), buff.size());
			pageID = nextID;
		}

		offset += buff.size();
	}





}

#pragma pack(1)
struct XVAStruct
{
	char name[7];
	char end1;
	char number_name[8];
	char end2;
};
#pragma pack()

bool findXVABlock(const IO::DataArray& block, const std::string_view textValue, uint32_t & pos)
{
	for (uint32_t i = 0; i < block.size(); i += default_sector_size)
	{
		XVAStruct* findxvaStruct = (XVAStruct*)(block.data() + i);
		if (memcmp(findxvaStruct->name, textValue.data(), 7) == 0)
		{
			pos = i;
			return true;
		}
	}
	return false;
}

void saveXVA(const IO::path_string& sourcepath , const IO::path_string& targetpath)
{
	//Ref:422
	const uint32_t REF_NAME_SIZE = 8;
	const uint32_t FULL_DATABLOCK_SUZE = 1050112;
	const uint32_t FIRST_PART_SIZE = 512;
	const uint32_t USERDATA_SIZE = 1048576;
	const uint32_t END_PART_SIZE = 1024;
	const uint32_t THE_OFFSET = 58368;
	try
	{
		IO::File sourceFile(sourcepath);
		sourceFile.OpenRead();
		uint64_t offset = 0;

		IO::File targetFile(targetpath);
		targetFile.OpenWrite();

		const std::string_view textValue = "Ref:430";

		while (offset < sourceFile.Size())
		{

			IO::DataArray buff(FULL_DATABLOCK_SUZE);

			sourceFile.setPosition(offset);
			sourceFile.ReadData(buff);
			if (memcmp(buff.data(), Signatures::bad_sector_marker, Signatures::bad_sector_marker_size) != 0)
			{
				XVAStruct* xvaStruct = (XVAStruct*)buff.data();
				xvaStruct->end1 = 0;
				xvaStruct->end2 = 0;

				std::string fileName = xvaStruct->name;
				std::string numberName = xvaStruct->number_name;

				if (fileName.compare(textValue) == 0)
				{

					uint64_t block_number = stoll(numberName);


					uint64_t target_offset = block_number * USERDATA_SIZE;
					std::wcout << "block_number " << block_number << "\r\n";

					if (target_offset < (uint64_t)1224 * 1024 * 1024 * 1024)
					{
						targetFile.setPosition(target_offset);
						targetFile.WriteData(buff.data() + FIRST_PART_SIZE, USERDATA_SIZE);
					}
				}
				else
				{
					uint32_t pos = 0;
					if (findXVABlock(buff, textValue, pos))
					{
						offset += pos;
						continue;
					}
				}
			}

			offset += FULL_DATABLOCK_SUZE;
		}

	}
	catch (IO::Error::IOErrorException& ec)
	{
		std::wcout << ec.what();
	}


	
}

void createFileFillMarker(const IO::path_string& targetpath)
{
	IO::File targetFile(targetpath);
	targetFile.OpenWrite();

	IO::DataArray buff(default_block_size);
	uint64_t MAXFILESIZE = 1181144317952;//(uint64_t)300 * 1024 * 1024 * 1024;
	for (uint32_t i = 0; i < buff.size() ; i += default_sector_size)
	{
		memcpy(buff.data() + i, Signatures::bad_sector_marker, Signatures::bad_sector_marker_size);
	}
	uint64_t offset = 0;
	while (offset < MAXFILESIZE)
	{
		targetFile.setPosition(offset);
		targetFile.WriteData(buff.data(), buff.size());

		offset += buff.size();
	}

}

#include "vxfs_inode.h"

void readVXFS_inode()
{
	IO::File sourceFile(LR"(c:\53208\NEW_RAID5_miss1.dsk)");
	sourceFile.OpenRead();
	uint64_t offset = 22866176;
	IO::DataArray buff(default_block_size);
	sourceFile.setPosition(offset);
	sourceFile.ReadData(buff);

	vxfs_dinode* pvxfsInode = (vxfs_dinode*)buff.data();
	auto direct0 = pvxfsInode->vdi_org.ext4.ve4_direct[0];
	auto direct1 = pvxfsInode->vdi_org.ext4.ve4_direct[0];

	uint32_t extent0 = direct0.extent;
	uint32_t extent_size0 = direct0.size;

	uint32_t extent1 = direct1.extent;
	uint32_t extent_size1 = direct1.size;

	uint32_t indirect0 = pvxfsInode->vdi_org.ext4.ve4_indir[0];
	uint32_t indirect1 = pvxfsInode->vdi_org.ext4.ve4_indir[1];

	int k = 1;
	k = 2;

	IO::toBE32(extent0);
	IO::toBE32(extent_size0);

	IO::toBE32(extent1);
	IO::toBE32(extent_size1);
	IO::toBE32(indirect0);
	IO::toBE32(indirect1);
	//constexpr uint32_t vxfs_dinode_size = sizeof(vxfs_dinode);
	k = 3;

	
}

void saveByVxFSInode(const IO::path_string& sourcepath, const IO::path_string& targetpath)
{
	const uint64_t partition_offset = 5963776;
	const uint64_t inode_offset = 26812416;
	// part 1 26304512(bytes)
	// part 2 26812416 (bytes) . From 0 to 3344 (bytes)

	IO::File sourceFile(sourcepath);
	sourceFile.OpenRead();

	IO::File targetFile(targetpath);
	targetFile.OpenWrite();

	uint64_t offset = inode_offset + partition_offset;
	IO::DataArray inode(8192);
	sourceFile.setPosition(offset);
	sourceFile.ReadData(inode);
	
	constexpr uint32_t vxfs_typed_size = sizeof(vxfs_typed);

	uint64_t sourceOffset = 0;
	uint64_t lastOffset = 0;
	vxfs_typed* pVXFS_typed = (vxfs_typed*)inode.data();

	for (uint32_t i = 0; i < 3344; i += vxfs_typed_size)
	{
		uint32_t dataOffset = pVXFS_typed->vt_block;
		uint32_t dataSize = pVXFS_typed->vt_size;
		IO::toBE32(dataOffset);
		IO::toBE32(dataSize);

		sourceOffset = partition_offset + (uint64_t)dataOffset * 1024/* + 1024*/;
		uint64_t writeSize = (uint64_t)dataSize * 1024;
		uint64_t target_offset = pVXFS_typed->vt_hdr;
		IO::toBE64(target_offset);
		target_offset = target_offset & 0x00000000FFFFFFFF;
		target_offset *= 1024;
		//qtRaw.appendToFile(targetFile, soruceOffset, (uint64_t)dataSize*1024);
		IO::appendDataToFile(sourceFile, sourceOffset, targetFile, writeSize, target_offset);
		//lastOffset = sourceOffset + ;
		pVXFS_typed++;
	}
	int k = 1;
	k = 2;

	//vxfs_typed


}

void setFreePageNumber(const IO::path_string& sourcepath , const IO::path_string& targeFilename)
{
	IO::File sourceFile(sourcepath);
	sourceFile.OpenRead();

	//IO::path_string targeFilename = sourcepath + L".result";
	IO::File targetFile(targeFilename);
	targetFile.OpenWrite();
	targetFile.setPosition(targetFile.Size());

	IO::DataArray buff(8192);
	sourceFile.ReadData(buff);


	uint32_t valueStart = 0x022CAF60;
	//IO::toBE32(valueStart);

	for (auto i = 0; i < 1120; ++i)
	{
		++valueStart;
		auto beVal = valueStart;
		IO::toBE32(beVal);

		memcpy(buff.data() + 4, &beVal, 4);
		targetFile.WriteData(buff.data() , buff.size());
	}


}


#include "raw/gopronew.h"

void saveMDAT(const IO::path_string& filename,
			  const IO::path_string& mdat_txt,
			  const IO::path_string& target_folder)
{
	RAW::ext4_raw ext4(nullptr);

	auto mdatList = ext4.readOffsetsFromFile(mdat_txt);

	auto sourcePtr = IO::makeFilePtr(filename);
	sourcePtr->OpenRead();

	for (auto mdatOffset : mdatList)
	{
		RAW::QuickTimeRaw qtRaw(sourcePtr);
		auto mdatHandle = qtRaw.readQtAtom(mdatOffset);
		if (mdatHandle.isValid())
		{
			auto fileName = target_folder + IO::toHexString(mdatOffset) + L".mov";
			IO::File targetFile(fileName);
			targetFile.OpenCreate();
			auto mdat_size = mdatHandle.size();
			qtRaw.appendToFile(targetFile, mdatOffset, mdatHandle.size());
		}
	}

}

void moovToDateSubfolders(const IO::path_string& folderfilename)
{
	IO::Finder finder;
	finder.FindFiles(folderfilename);

	for (const auto& filepath : finder.getFiles())
	{
		fs::path mainPath(filepath);
		auto filename = mainPath.filename().generic_wstring();

		//fs::create_directories
		auto yearName = filename.substr(0, 4);
		auto monthName = filename.substr(5, 2);

		auto newFolder = folderfilename  + yearName + L"\\" + monthName;

		fs::create_directories(newFolder);

		auto newFilename = newFolder + L"\\" + filename;

		fs::rename(filepath, newFilename);
		int k = 1;
		k = 1;
	}
}

int wmain(int argc, wchar_t* argv[])
{
	//IO::path_string sourcefilename = LR"(f:\$Folder3DCFE9900\SERVER011.xva)";
	//IO::path_string targefilename = LR"(g:\53242\430_31_07.bin )";

	//setFreePageNumber(sourcefilename , targefilename);

	//saveByVxFSInode(sourcefilename, targefilename );
	//readVXFS_inode();
	//IO::path_string sourcefilename = LR"(c:\53208\NEW_RAID5_miss1.dsk )";
	//IO::path_string targefilename = LR"(d:\53242\425.bin )";
	//saveXVA(sourcefilename, targefilename);

	//createFileFillMarker(targefilename);

	//uint64_t start_offset = 0xB4559E000;
	//IO::path_string sourcefile = LR"(c:\53208\jbod.dsk)";
	//IO::path_string targetfile = LR"(c:\53208\page)";
	//saveByPageNumber(sourcefile, targetfile, start_offset);

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

		//IO::path_string filepath = LR"(c:\53443\53443.img)";
		//RAW::GoProRawNew gpRawNew(filepath);
		//gpRawNew.SaveByMarkers(LR"(c:\53443\53443.img.ftyp)", LR"(d:\53443\result\)");

	IO::path_string sourcefile = LR"(f:\CR3\)";
	moovToDateSubfolders(sourcefile);
	//IO::path_string ftyppath = LR"(c:\53443\53443.img.ftyp)";
	//IO::path_string mdatpath = LR"(c:\53443\53443.img.mdat)";
	//IO::path_string targetFolder = LR"(x:\53443\video1\)";
	//RAW::SavePos_ftyp_mdat(sourcefile);
	//RAW::saveQtFragment(sourcefile, ftyppath, mdatpath, targetFolder);
	//saveMDAT(sourcefile,  mdatpath, targetFolder);


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


	//IO::path_string foldername = LR"(x:\53239\result\)";
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
	//IO::path_string foldername = LR"(i:\422\DataBase\WorkBase\!Problem\fixed\)";
	//fixAllDbfFiles(foldername);

	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return 0;
}
