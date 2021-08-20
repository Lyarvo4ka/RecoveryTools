
#include "signatureTester.h"
#include "extensionextractor.h"
#include "json/signaturereader.h"

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