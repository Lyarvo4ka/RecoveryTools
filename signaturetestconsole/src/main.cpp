#include <QtCore/QCoreApplication>

#include "../libjsonreader/include/jsonreader.h"
#include "../libjsonreader/include/signaturereader.h"

//#include "../JsonReader/JsonReader.h"
//#include "../JsonReader/SignatureReader.h"
#include "extensionextractor.h"

#include "io/Finder.h"
#include "raw/AbstractRaw.h"

#include <filesystem>

namespace fs = std::filesystem;

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


#include "signatureTester.h"

int extract_extension()
{

	ExtensionExtractor extExtractor;

	IO::path_string singFolder = LR"(d:\develop\RecoveryProjects\SignatureTestConsole\signatures\)";
	SignatureReader signReader;
	signReader.loadAllSignatures(singFolder, L".json");

	auto headerBase = std::make_shared<RAW::HeaderBase>();
	for (const auto& jsonFileStruct : signReader.getAllSignatures())
	{
		headerBase->addFileFormat(toUniqueFileStruct(jsonFileStruct));
	}

	extExtractor.setHeaderBase(headerBase);

	IO::Finder finder;
	finder.FindFiles(LR"(f:\49304\!NoName\!!!!\)");
	//finder.add_extension(L"*.chk");
	auto all_files = finder.getFiles();
	//auto listFiles = getFilesWithoutExtension(all_files);

	extExtractor.extract_extensions(all_files);
	return 0;
}

int test_signatures()
{
	////////////////////
	IO::path_string singFolder = LR"(d:\develop\RecoveryProjects\SignatureTestConsole\signatures\)";
	SignatureReader signReader;
	signReader.loadAllSignatures(singFolder, L".json");

	IO::path_string extFolder = LR"(d:\develop\RecoveryProjects\SignatureTestConsole\extensions\)";
	ExtensionReader extReader;
	extReader.loadAllExtensions(extFolder, L".json");

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

	IO::path_string folderToTest = LR"(g:\49888\)";

	IO::Finder finder;
	//finder.add_extension(L".mp3");
	finder.FindFiles(folderToTest);

	SignatureTester signTester;
	signTester.setExtensionBase(extBase);
	signTester.setSignatureBase(signBase);


	for (auto fileToTest : finder.getFiles())
	{
		signTester.testSigantures(fileToTest);
	}







	return 0;
}





int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	test_signatures();


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
