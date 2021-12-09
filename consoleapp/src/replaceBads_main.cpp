#include "io/utility.h"

enum  ReplaceBadsParams { WITH_BADS = 1, WITHOUT_BADS, TARGET, PARAM_COUNT };


//#include "io/onec.h"

void replaceEncryptWithBackup(const IO::path_string& enc_filename, const IO::path_string& backup_filename)
{
	const uint32_t MaxEncryptBytes = 2605056;
	const uint32_t BackupOffset = 7680;
	const uint32_t EncyptedSize = 8192;

	IO::File encFile(enc_filename);
	encFile.OpenWrite();

	IO::File backupFile(backup_filename);
	backupFile.OpenRead();

	uint64_t offset = 0;
	uint64_t backup_offset = BackupOffset;

	IO::DataArray buffer(EncyptedSize);

	while (offset < MaxEncryptBytes)
	{
		backupFile.setPosition(backup_offset);
		backupFile.ReadData(buffer);

		encFile.setPosition(offset);
		encFile.WriteData(buffer.data(), buffer.size());


		backup_offset += EncyptedSize*2;
		offset += EncyptedSize*2;
	}
}


int wmain(int argc, wchar_t* argv[])
{
		IO::path_string enc =LR"(g:\50658\BASE_ANR_OFFICE.mdf)";
		IO::path_string backup = LR"(g:\50658\BASE_ANR_OFFICE.bak)";

		replaceEncryptWithBackup(enc, backup);
	//if ( argc == PARAM_COUNT)
	//{
	//	
	//	IO::path_string withBads = argv[WITH_BADS];
	//	IO::path_string withoutBads = argv[WITHOUT_BADS];
	//	IO::path_string target = argv[TARGET];

	//	//fixAllDbfFiles(foldername);
	//	IO::replaceBadsFromOtherFile(withBads, withoutBads, target);
	//	 
	//	_CrtDumpMemoryLeaks();
	//	std::cout << std::endl << " FINISHED "; 
	//}
	//else
	//{
	//	std::cout << " Wrong params. " << std::endl ;
	//	std::cout << std::endl << " AppName.exe withBads withoutBads target " << std::endl ;
	//}
	 
	return 0;
}
