#include <QtCore/QCoreApplication>




#include "io/Finder.h"
#include "raw/AbstractRaw.h"
#include "json/signaturereader.h"
#include "extensionbase.h"
#include "signatureTester.h"


#include <filesystem>

namespace fs = std::filesystem;


std::wstring toWstring_cast(const std::string& s)
{
    int len;
    int slength = (int)s.length() ;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    std::wstring r(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
    return r;
}

std::string toString_cast(const std::wstring& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0); 
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0); 
    return r;
}

void rename_to_bad_file(const IO::path_string & file_path)
{
	fs::rename(file_path, file_path + L".bad_file");
}

void testMp3(const RAW::FileStruct & Mp3FileStruct, const IO::path_string & file_to_test)
{
	IO::File file(file_to_test);
	file.OpenRead();
	auto file_size = file.Size();

	const uint32_t cmp_size = 4096;
	const uint32_t cmp_nulls_size = 16;

	if (file_size < cmp_size)
	{
		file.Close();
		rename_to_bad_file(file_to_test);
		return;
	}

	IO::DataArray block_data(cmp_size);
	file.ReadData(block_data);
	file.Close();


	bool isNullsFromStart = true;

	uint32_t not_null_pos = 0;
	for (not_null_pos = 0; not_null_pos < cmp_size; ++not_null_pos)
	{
		if (block_data.data()[not_null_pos]!= 0)
		{
			isNullsFromStart = false;
			break;
		}
	}

	IO::ByteArray pMp3Start = block_data.data();

	uint32_t size_to_compare = cmp_size;
	if (not_null_pos > cmp_nulls_size)
	{
		pMp3Start = block_data.data() + not_null_pos;
		size_to_compare = cmp_size - not_null_pos;
	}


	auto bFound = Mp3FileStruct.compareWithAllHeaders(pMp3Start, size_to_compare);
	
	if (!bFound)
	{
		rename_to_bad_file(file_to_test);
	}
	

}

void testSignature(const RAW::FileStruct& fileStruct, const IO::path_string& file_to_test)
{
	IO::File file(file_to_test);
	file.OpenRead();
	auto file_size = file.Size();
	auto cmp_size = default_sector_size;

	if (file_size < cmp_size)
	{
		file.Close();
		rename_to_bad_file(file_to_test);
		return;
	}
	IO::DataArray buffer(cmp_size);
	file.ReadData(buffer);
	file.Close();
	auto bFound = fileStruct.compareWithAllHeaders(buffer.data(), buffer.size());

	if (!bFound)
	{
		rename_to_bad_file(file_to_test);
	}



}

inline void ReadSignatures(SignatureReader & signatureReader, RAW::HeaderBase::Ptr headeBase , const IO::path_string & folder)
{
	if (headeBase)
	{
		signatureReader.loadAllSignatures(folder , L".json");
		for (auto json_signature : signatureReader.getAllSignatures())
		{
			headeBase->addFileFormat(std::move(toUniqueFileStruct(json_signature)));
		}
	}
}



int test_signatures(const IO::path_string & folderToTest)
{
	std::filesystem::path current_folder = R"(c:\soft\!MyPrograms\SignatureTestConsole\)"; // fs::current_path() ;

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


	////////////////


	IO::Finder finder;
	//finder.add_extension(L".mp3");
	finder.FindFiles(folderToTest);

	std::wcout << folderToTest << std::endl;
	std::cout << "Found " << finder.numberOfFiles() << std::endl;

	SignatureTester signTester;
	signTester.setExtensionBase(extBase);
	signTester.setSignatureBase(signBase);


	for (auto fileToTest : finder.getFiles())
	{
		try
		{
			//std::wcout << fileToTest << " ";
			signTester.testSigantures(fileToTest);
			//std::wcout << std::endl;
		}
		catch (IO::IOErrorException ex)
		{
			std::cout << ex.what();
		}
		catch (...)
		{
			std::wcout << fileToTest << std::endl;
			std::cout << "Caught unknown exception" << std::endl;
		}
	}

	auto unkownListExt = signTester.getUnknownExtensions();

	IO::File fileTxt(L"unknown.txt");
	fileTxt.OpenCreate();

	for (const auto & ext : unkownListExt )
	{
		std::string writeText = ext;
		writeText += "\r\n";

		fileTxt.WriteText(writeText);
	}






	return 0;
}



#include "io\utility.h"

int main(int argc, char* argv[])
{
	
	QCoreApplication a(argc, argv);
	if (argc == 2)
	{
		const auto folder_value = argv[1];
		std::string folder_string = folder_value;
		IO::path_string folderToTest = toWstring_cast(folder_string);

		setlocale(LC_ALL, "Russian");
		try
		{
			test_signatures(folderToTest);

		}
		catch (const Error::IOErrorException& ex)
		{
			std::cout << "Caught exception " << ex.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "Caught unknown exception" << std::endl;
		}

	}
	else
		std::cout << "Wrong params" << std::endl;


	//	IO::Finder finder;
	////finder.add_extension(L".mp3");
	//finder.FindFiles(folderToTest);

	//SignatureTester signTester;
	//signTester.setExtensionBase(extBase);
	//signTester.setSignatureBase(signBase);

	//IO::DataArray buffer(default_sector_size);

	//for (auto fileToTest : finder.getFiles())
	//{
	//	IO::File file(fileToTest);
	//	file.OpenRead();
	//	if (file.Size() > default_sector_size)
	//	{
	//		file.ReadData(buffer);
	//		file.Close();

	//		if (IO::isBlockContainsValue(buffer.data() , buffer.size(), 0x00) )
	//			rename_to_bad_file(fileToTest);
	//	}
	//}


	qDebug() << "Finished.";
	return a.exec();
}

//
//int main(int argc, char *argv[])
//{
//	QCoreApplication a(argc, argv);
//
//	QString jsonFile("mts.json");
//
//	QFile file(jsonFile);
//	if (!file.open(QIODevice::ReadOnly))
//	{
//		qInfo() << "Error to open file. \"" << file.fileName() << "\"";
//		return -1;
//	}
//
//	auto json_str = file.readAll();
//	QList<JsonFileStruct> listFileStruct;
//	ReadJsonFIle(json_str, listFileStruct);
//
//	IO::Finder finder;
//	finder.add_extension(L".m2ts");
//
//	finder.FindFiles(LR"(f:\46976\)");
//	auto fileList = finder.getFiles();
//
//	if (!listFileStruct.empty())
//	{
//		auto fileStructQt = listFileStruct.first();
//		auto fileStruct = toFileStruct(fileStructQt);
//
//		for (auto & theFile : fileList)
//		{
//			testSignature(*fileStruct.get(),theFile);
//		}
//	}
//	
//
//	qDebug() << "Finished.";
//	return a.exec();
//}
