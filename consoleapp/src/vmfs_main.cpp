#include "io/file.h"
#include "io/finder.h"
#include <sstream>
#include <iostream>

struct OffsetExtension
{
	uint64_t offset;
	std::string extension;
};

std::list<std::string> splitString(const std::string & strData , const char delimiter)
{
	std::istringstream tmpStrStream(strData);
	std::string tmpStr;
	std::list<std::string> lines;
	while (std::getline(tmpStrStream, tmpStr, delimiter))
	{
		lines.push_back(tmpStr);
		std::cout << tmpStr << std::endl;
	}
	return lines;
}

std::list<OffsetExtension> readFileOffsetExtension(const IO::path_string& filepath)
{
	std::list<OffsetExtension> listOffsetExt;
	IO::File txtFile(filepath);
	txtFile.OpenRead();
	IO::DataArray buff(txtFile.Size() + 1);
	ZeroMemory(buff.data(), txtFile.Size() + 1);
	txtFile.ReadData(buff.data() , buff.size() - 1);

	auto listLines = splitString((const char*)buff.data(), '\n');
	for (const auto & line : listLines)
	{
		auto listOffsetExtStr = splitString(line, ',');
		if (!listOffsetExtStr.empty())
		{
			auto offsetStr = listOffsetExtStr.front();
			listOffsetExtStr.pop_front();
			auto extStr = listOffsetExtStr.front();

			OffsetExtension offExtStruct;
			extStr.erase(remove(extStr.begin(), extStr.end(), '\r'), extStr.end());
			offExtStruct.extension = extStr;
			offExtStruct.offset = std::stoll(offsetStr);
			listOffsetExt.push_back(offExtStruct);
		}
	}


	return listOffsetExt;
}

void findFilesWithOffsets(const IO::path_string & folderForParts , const std::list<OffsetExtension> & listOffsetExt)
{
	IO::Finder finder;
	finder.add_extension(L".part");
	finder.FindFiles(folderForParts);
	auto listFiles = finder.getFiles();

	for (const auto& filename : listFiles)
	{
		IO::File theFile(filename);
		theFile.OpenRead();

		uint64_t offset = 0;
		IO::DataArray buff(default_block_size);
		uint32_t block_size = 0;
		while (offset < theFile.Size())
		{
			block_size = IO::calcBlockSize(offset, theFile.Size(), default_block_size);
			theFile.setPosition(offset);
			theFile.ReadData(buff.data() , block_size);

			for (uint32_t iSector = 0; iSector < block_size; iSector += default_sector_size)
			{

			}


			offset += block_size;
		}

	}

	int k = 1;
	k = 2;

}

int wmain(int argc, wchar_t* argv[])
{
	IO::path_string folderForParts =LR"(c:\53035\SSD_RAID_0\)";

	IO::path_string fileOffsetExtTxt = LR"(c:\53035\listOffsetExtension.txt)";
	auto listOffstExt = readFileOffsetExtension(fileOffsetExtTxt);

	findFilesWithOffsets(folderForParts, listOffstExt);
	 
	return 0;
}