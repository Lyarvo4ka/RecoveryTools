#pragma once

#include "../libjsonreader/include/jsonreader.h"

#include <map>
#include "io/file.h"
#include "raw/AbstractRaw.h"

#include <filesystem>
namespace fs = std::filesystem;

using FormatName = std::string;
using ListFormatName = std::list<FormatName>;
using ExtensionName = std::string;

class ExtensionBase
{
	std::map<ExtensionName, ListFormatName > extensionsMap_;

public:
	void add(const ExtensionName& extensionName, const ListFormatName& listFormantName)
	{
		auto iter = extensionsMap_.find(extensionName);
		if (iter == extensionsMap_.end())
			extensionsMap_.emplace(extensionName, listFormantName);
	}

	ListFormatName find(const ExtensionName& extensionName)
	{
		ListFormatName listFormatName;
		auto iter = extensionsMap_.find(extensionName);
		if (iter != extensionsMap_.end())
			listFormatName = iter->second;
		return listFormatName;
	}
};

class SignatureBase
{
	std::map< FormatName, RAW::FileStruct> headerBase_;

public:
	void add(const JsonFileStruct & jsonFileStruct)
	{
		auto fileStruct = toUniqueFileStruct(jsonFileStruct);
		headerBase_.emplace(jsonFileStruct.name.toStdString(), *fileStruct);
	}
	RAW::FileStruct find( const FormatName & formatName)
	{
		auto findIter = headerBase_.find(formatName);
		if (findIter != headerBase_.end())
			return findIter->second;
		throw;
	}
};

class SignatureTester
{
	ExtensionBase extensionsBase_;
	SignatureBase signatureBase_;
public:
	void setExtensionBase(const ExtensionBase& extensionsBase)
	{
		extensionsBase_ = extensionsBase;
	}
	void setSignatureBase( const SignatureBase & signatureBase)
	{
		signatureBase_ = signatureBase;
	}
	void rename_file(const IO::path_string& filename, const IO::path_string& ext_result)
	{
		try
		{
			std::wcout << filename.c_str();

			auto new_filename = filename + ext_result;

			fs::rename(filename, new_filename);
		}
		catch (IO::Error::IOErrorException& ex)
		{
			const char* text = ex.what();
			std::cout << " Cougth exception " << text;

		}
		catch (fs::filesystem_error& fs_error)
		{
			std::cout << " Cougth exception " << fs_error.what();
			int k = 1;
			k = 2;

		}
	}
	void renameToBadfile(const IO::path_string& filename)
	{
		rename_file(filename, L".bad_file");
		std::cout << " - BAD" << std::endl;

	}
	ExtensionName getExtension(const IO::path_string& filename)
	{
		fs::path filePath(filename);
		return filePath.extension().generic_string();
	}

	std::list<RAW::FileStruct> getListFileStructFromListFormatName(const ListFormatName& listFormatName)
	{
		std::list<RAW::FileStruct> listFileStruct;
		for (auto formatName : listFormatName)
		{
			auto fileStruct = signatureBase_.find(formatName);
			if (fileStruct.isValid())
				listFileStruct.emplace_back(fileStruct);
		}
		return listFileStruct;
	}


	bool testSignatureWithList(const IO::path_string& filename , const std::list<RAW::FileStruct> & listFileStruct)
	{
		IO::File file(filename);
		file.OpenRead();
		if (file.Size() == 0)
			return false;

		// Here could discover size to test

		IO::DataArray buffer(default_sector_size);
		uint32_t read_size = buffer.size();

		if (file.Size() < buffer.size())
			read_size = file.Size();

		file.ReadData(buffer.data(), read_size);
		file.Close();

		for (const auto & fileStruct : listFileStruct)
		{
			if (fileStruct.compareWithAllHeaders(buffer.data(), buffer.size()))
				return true;
		}
		return false;
	}

	void testSigantures(const IO::path_string& filename)
	{


		auto extension = getExtension(filename);

		auto listFormatName = extensionsBase_.find(extension);
		auto listFileStruct = getListFileStructFromListFormatName(listFormatName);

		if (!testSignatureWithList(filename, listFileStruct))
		{
			renameToBadfile(filename);
		}





	}



	//void testSigantures(const IO::path_list& listFiles)
	//{
	//	const uint32_t DefaultReadSize = 33280;
	//	DataArray buffer(DefaultReadSize);
	//	for (auto filepath : listFiles)
	//	{
	//		uint32_t read_size = DefaultReadSize;
	//		File file(filepath);
	//		file.OpenRead();
	//		if (file.Size() < buffer.size())
	//			read_size = file.Size();

	//		file.ReadData(buffer.data(), read_size);
	//		file.Close();

	//		auto file_struct = headerBase_->find(buffer.data(), read_size);
	//		if (file_struct)
	//		{
	//			qInfo() << filepath << "-->" << QString::fromStdWString(file_struct->getExtension());
	//			auto ext = file_struct->getExtension();
	//			auto filePathWithExt = filepath + file_struct->getExtension();
	//			fs::rename(filepath, filePathWithExt);
	//		}
	//	}

	//}
};



