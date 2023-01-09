
#include "signatureTester.h"
#include "extensionextractor.h"
#include "json/signaturereader.h"

int extract_extension()
{

	ExtensionExtractor extExtractor;

	IO::path_string singFolder = LR"(c:\soft\!MyPrograms\SignatureTestConsole\signatures\)";
	SignatureReader signReader;
	signReader.loadAllSignatures(singFolder, L".json");

	auto headerBase = std::make_shared<RAW::HeaderBase>();
	for (const auto& jsonFileStruct : signReader.getAllSignatures())
	{
		headerBase->addFileFormat(toUniqueFileStruct(jsonFileStruct));
	}

	extExtractor.setHeaderBase(headerBase);

	IO::Finder finder;
	finder.FindFiles(LR"(e:\lost+found\)");
	//finder.add_extension(L"*.chk");
	auto all_files = finder.getFiles();
	//auto listFiles = getFilesWithoutExtension(all_files);

	extExtractor.extract_extensions(all_files);
	return 0;
}

inline int NotNullPosFromEnd(const ByteArray data, const uint32_t size)
{
	int pos = size - 1;
	while (pos >= 0)
	{
		if (data[pos] != 0)
			return (size - pos);
		--pos;
	}
	return 0;
}

inline void removeNullsFromEndFile(const path_string& file_path, uint32_t sizeToTest = default_block_size)
{
	File file(file_path);
	if (!file.Open(OpenMode::OpenWrite))
	{
		wprintf_s(L"Error open file.\n");
		return;
	}
	//sizeToTest = default_block_size;
	auto file_size = file.Size();
	DataArray buffer(sizeToTest);
	//	if (file_size >= sizeToTest)
	{
		uint32_t lastBlock = sizeToTest;
		if (file_size < sizeToTest)
			lastBlock = static_cast<uint32_t>(file_size);

		uint64_t offset = file_size - lastBlock;
		file.setPosition(offset);
		auto bytesRead = file.ReadData(buffer.data(), lastBlock);
		if (bytesRead == lastBlock)
		{
			int not_null = NotNullPosFromEnd(buffer.data(), lastBlock);
			if (not_null > 0)
			{
				uint64_t new_size = file_size - not_null + 1;
				file.setSize(new_size);
				wprintf_s(L"FIle size has been changed %s.\n", file_path.c_str());
			}
		}
	}

}


	//int main(int argc, char* argv[])
	//{

	//	QCoreApplication a(argc, argv);

	//	extract_extension();
	//	//IO::Finder finder;
	//	//finder.FindFiles(LR"(d:\51098\result\pdf\)");
	//	//auto fileList = finder.getFiles();

	//	//for (const auto& filename : fileList)
	//	//{
	//	//	removeNullsFromEndFile(filename, 4096);
	//	//}

	//	qDebug() << "Finished.";
	//	return a.exec();
	//}
