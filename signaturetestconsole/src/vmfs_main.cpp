#include <QtCore/QCoreApplication>

#include "io/file.h"
#include "io/diskdevice.h"
#include "io/finder.h"
#include <sstream>
#include <iostream>

#include "json/signaturereader.h"
#include "extensionbase.h"
#include "signatureTester.h"

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

class FinderByExtension
{
	ExtensionBase extensionsBase_;
	SignatureBase signatureBase_;
public:
	FinderByExtension(const ExtensionBase& extensionsBase, const SignatureBase& signatureBase)
		:extensionsBase_(extensionsBase)
		, signatureBase_(signatureBase)
	{

	}
	auto getListFileStruectFromBase(const std::string& extension)
	{
		std::list<RAW::FileStruct> listFileStruct;
		auto listFormatName = extensionsBase_.find(extension);
		if (!listFormatName.empty())
			 listFileStruct = getListFileStructFromSingatureListFormatName(listFormatName, signatureBase_);
		return listFileStruct;
	}

	bool findHeaderByExt_SectorStep(const std::string& extension, const DataArray buff , const uint32_t block_size , uint32_t &found_pos)
	{

		auto listFileStruct = getListFileStruectFromBase(extension);
		if (!listFileStruct.empty())
			for (uint32_t iSector = 0; iSector < block_size; iSector += default_sector_size)
			if (cmpHeaderWithFileStruct(listFileStruct.front(), buff.data() + iSector, default_sector_size))
			{
				found_pos = iSector;
				return true;
			}
		return false;
	}

	bool cmpHeaderWithFileStruct(const RAW::FileStruct& fileStruct , const ByteArray data , uint32_t size)
	{
		return fileStruct.compareWithAllHeaders(data, size);
	}
	bool cmpHeaderWithFirstFileStruct(const std::list<RAW::FileStruct>& listFileStruct, const ByteArray data, uint32_t size)
	{
		if (!listFileStruct.empty())
			return listFileStruct.front().compareWithAllHeaders(data, size);
		return false;
	}

};



void findFilesWithOffsets(const IO::path_string & folderForParts , const std::list<OffsetExtension> & listOffsetExt)
{
	std::filesystem::path current_folder = R"(d:\backup\)"; // fs::current_path() ;

	auto singature_path = current_folder / "signatures";
	std::cout << singature_path.string() << std::endl;
	//singature_path += "signatures";
	SignatureReader signReader;
	signReader.loadAllSignatures(singature_path.wstring(), L".json");

	auto extension_path = current_folder / "extensions";
	std::cout << extension_path.string() << std::endl;
	//extension_path += "extensions";
	ExtensionReader extReader;
	extReader.loadAllExtensions(extension_path.wstring(), L".json");

	ExtensionBase extBase;
	auto listJsonExtension = extReader.getAllSignatures();
	for (auto jsonExtension : listJsonExtension)
	{
		ListFormatName listFormatName;
		for (auto formatName : jsonExtension.listFormatName)
		{
			listFormatName.emplace_back(formatName.toStdString());
		}
		extBase.add(jsonExtension.extensionName.toStdString(), listFormatName);
	}

	SignatureBase signBase;
	for (auto jsonFileStruct : signReader.getAllSignatures())
		signBase.add(jsonFileStruct);

	FinderByExtension extfinder(extBase, signBase);

	IO::Finder finder;
	finder.add_extension(L".part");
	finder.FindFiles(folderForParts);
	auto listFiles = finder.getFiles();

	for (const auto& filename : listFiles)
	{
		uint64_t found_offset = 0;
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

			auto iter = listOffsetExt.begin();
			uint64_t first_postition = iter->offset;
			uint32_t sector_pos = 0;
			if (extfinder.findHeaderByExt_SectorStep(iter->extension, buff, block_size, sector_pos))
			{
				found_offset = offset + sector_pos;
				uint64_t next_offset = found_offset;
				uint64_t prev_offset = iter->offset;

				bool bFound = true;
				while (++iter != listOffsetExt.end())
				{
					uint64_t distance = iter->offset - prev_offset;
					distance *= default_sector_size;
					next_offset += distance;
					if (next_offset > theFile.Size())
					{
						bFound = false;
						break;
					}
					theFile.setPosition(next_offset);
					theFile.ReadData(buff.data(), default_sector_size);

					auto listFileStruct = extfinder.getListFileStruectFromBase(iter->extension);
					if (!extfinder.cmpHeaderWithFirstFileStruct(listFileStruct, buff.data(), default_sector_size))
					{
						bFound = false;
						break;

					}
					prev_offset = iter->offset;

				}
				if (bFound == true)
				{
					int k = 64;
					k = 1;
					std::wcout << filename.c_str() << " " << found_offset << " " << first_postition<< std::endl;
					break;
				}
			}

			offset += block_size;
		}

	}

	int k = 1;
	k = 2;

}




//
//int main(int argc, char* argv[])
//{
//
//	QCoreApplication a(argc, argv);
//
//	IO::path_string folderForParts = LR"(c:\53035\SSD_RAID_0_done\)";
//
//	IO::path_string fileOffsetExtTxt = LR"(c:\53035\listOffsetExtension.txt)";
//	auto listOffstExt = readFileOffsetExtension(fileOffsetExtTxt);
//
//	findFilesWithOffsets(folderForParts, listOffstExt);
//
//	qDebug() << "Finished.";
//	return a.exec();
//}
