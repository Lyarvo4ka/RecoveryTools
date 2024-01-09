#include "io/utility.h"

enum  ReplaceBadsParams { WITH_BADS = 1, WITHOUT_BADS, TARGET, PARAM_COUNT };


//#include "io/onec.h"
#include <map>

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

const uint32_t PAGE_SIZE = 2112;




auto calcPopularValue(const IO::DataArray& block , const uint32_t pageOffset)
{
	std::map<uint16_t, int > values;

	uint16_t *pMarker = 0;
	for (auto i = 0; i < block.size(); i += PAGE_SIZE)
	{
		pMarker = (uint16_t*)(block.data() + i + pageOffset);
		if (*pMarker != 0xFFFF)
		{
			auto iter = values.find(*pMarker);
			if (iter == values.end())
			{
				values.insert(std::make_pair(*pMarker, 0));
			}
			else
				++iter->second;
		}

	}
	//auto value_pair = values.begin();
	uint16_t result = 0;
	int prev = 0;
	for (const auto& mapPair : values)
	{
		if (prev < mapPair.second)
			result = mapPair.first;
	}
	return result;
}


void yaffsPageNumberModify(const IO::path_string& main_filename , const IO::path_string& next_filename, const IO::path_string &result_filename)
{

	IO::File mainFile(main_filename);
	mainFile.OpenRead();

	IO::File nextFile(next_filename);
	nextFile.OpenRead();

	IO::File targetFile(result_filename);
	targetFile.OpenCreate();

	IO::File logFile(main_filename + L".txt");
	logFile.OpenCreate();

	const uint32_t NUM_BLCOKS = 64;
	constexpr uint32_t YAFFSBLOCKSIZE = PAGE_SIZE * NUM_BLCOKS;
	const uint64_t start_offset = 75153408;

	const uint32_t MARKER_OFFSET = 2086;

	IO::DataArray mainBlock(YAFFSBLOCKSIZE);
	IO::DataArray nextBlock(YAFFSBLOCKSIZE);
	IO::DataArray resultBlock(YAFFSBLOCKSIZE);

	uint64_t offset = 0;
	while (offset < start_offset)
	{
		mainFile.setPosition(offset);
		mainFile.ReadData(mainBlock);

		targetFile.setPosition(offset);
		targetFile.WriteData(mainBlock.data(), mainBlock.size());
		offset += mainBlock.size();
	}


	offset = start_offset;
	


	while (offset < mainFile.Size())
	{

		mainFile.setPosition(offset);
		mainFile.ReadData(mainBlock);

		nextFile.setPosition(offset);
		nextFile.ReadData(nextBlock);
		auto marker = calcPopularValue(mainBlock, MARKER_OFFSET);

		for (auto i = 0; i < mainBlock.size(); i += PAGE_SIZE)
		{
			auto pMainMarker = (uint16_t*)(mainBlock.data() + i + MARKER_OFFSET);
			auto pNextMarker = (uint16_t*)(nextBlock.data() + i + MARKER_OFFSET);
			auto offset_tmp = offset + i;

			targetFile.setPosition(offset_tmp);
			targetFile.WriteData(mainBlock.data() + i, PAGE_SIZE);



			if (*pMainMarker != marker)
			if (*pNextMarker == marker)
			{
				targetFile.setPosition(offset_tmp);
				targetFile.WriteData(nextBlock.data() + i, PAGE_SIZE);
			}
			else
			{
				std::stringstream sstream;
				sstream << std::hex << offset_tmp + MARKER_OFFSET;
				std::string result = sstream.str();
				logFile.WriteText(result + "\r\n");
			}
			


		}



		offset += mainBlock.size();
	}
	

}

void saveOnlyWithID(const IO::path_string& src_filename, const IO::path_string& dst_filename)
{
	IO::File srcFile(src_filename);
	srcFile.OpenRead();

	IO::File dstFile(dst_filename);
	dstFile.OpenCreate();


	IO::DataArray buff(2112);
	uint64_t offset = 0;

	const uint16_t idToCMP = 0x013E;
	while (offset < srcFile.Size())
	{
		srcFile.setPosition(offset);
		srcFile.ReadData(buff);
		auto pID = (uint16_t * ) (buff.data() + 2090);
		auto id = *pID;
		if (id == idToCMP)
			dstFile.WriteData(buff.data(), buff.size());


		offset += buff.size();
	}
}

int yaffs_main(int argc, wchar_t* argv[])
{
	if ( argc == PARAM_COUNT)
	{
	
		IO::path_string main_filename = argv[WITH_BADS];
		IO::path_string next_filename = argv[WITHOUT_BADS];
		IO::path_string target_filename = argv[TARGET];

		//fixAllDbfFiles(foldername);
		yaffsPageNumberModify(main_filename, next_filename, target_filename);
	 
		_CrtDumpMemoryLeaks();
		std::cout << std::endl << " FINISHED "; 
	}
	else
	{
		std::cout << " Wrong params. " << std::endl ;
		std::cout << std::endl << " AppName.exe withBads withoutBads target " << std::endl ;
	}
		return 0;
}

#include "io/finder.h"

int wmain(int argc, wchar_t* argv[])
{
	//IO::path_string src =LR"(c:\tmp\Mutate_51403_result.dump)";
	//IO::path_string dst = LR"(c:\tmp\result)";
	//saveOnlyWithID(src, dst);

	//yaffs_main(argc, argv);



		//IO::path_string enc =LR"(g:\50658\BASE_ANR_OFFICE.mdf)";
		//IO::path_string backup = LR"(g:\50658\BASE_ANR_OFFICE.bak)";

		//replaceEncryptWithBackup(enc, backup);
	const char bad_sector_array[] = { 0x62 , 0x61 , 0x64 , 0x20 , 0x73 , 0x65 , 0x63 , 0x74 , 0x6F , 0x72 };
	constexpr uint32_t bad_sector_array_size = SIZEOF_ARRAY(bad_sector_array);
	const char unreadable_marker[] = {0x55 , 0x4E , 0x52 , 0x45 , 0x41 , 0x44 , 0x41 , 0x42 , 0x4C , 0x45 , 0x53 , 0x45 , 0x43 , 0x54 , 0x4F , 0x52};
	constexpr uint32_t unreadable_marker_size = SIZEOF_ARRAY(unreadable_marker);
	IO::DataArray bad_secotr_marker(bad_sector_array_size);
	std::memcpy(bad_secotr_marker.data(), bad_sector_array, bad_secotr_marker.size());
	//if ( argc == PARAM_COUNT)
	{
		
		//IO::path_string withBads = LR"(i:\422\DataBase\WorkBase\!Problem\1SCRDOC.DBF)";//argv[WITH_BADS];
		//IO::path_string withoutBads = LR"(i:\422\DataBase\WorkBase\!Problem\old\1SCRDOC.DBF)";//argv[WITHOUT_BADS];
		//IO::path_string target = withBads + L".fixed";//argv[TARGET];
		//IO::Finder finder;
		//finder.add_extension(L".dbf");
		//finder.FindFiles(LR"(i:\422\DataBase\WorkBase\!Problem\)");
		//IO::path_string oldFolder = LR"(i:\422\DataBase\WorkBase\old\)";
		//auto listFiles = finder.getFiles();
		//for (auto & withBads: listFiles)
		{ 
			//std::filesystem::path srcpath = withBads;
			//IO::path_string withoutBads = oldFolder + srcpath.stem().generic_wstring() + L".dbf";

			//IO::path_string target = withBads + L".fixed";
			IO::path_string withBads = LR"(x:\53467\Data\!Problem\База 1С ФОП нова\1Cv8.1CD)";
			IO::path_string withoutBads = LR"(x:\53467\Data\!Problem\База 1С ФОП нова\old\1Cv8.1CD)";
			IO::path_string target = LR"(x:\53467\Data\!Problem\База 1С ФОП нова\result)";
			IO::replaceBadsFromOtherFile(withBads, withoutBads, target, bad_secotr_marker);
		}
		//fixAllDbfFiles(foldername);
		 
		_CrtDumpMemoryLeaks();
		std::cout << std::endl << " FINISHED "; 
	}
//	else
	{
		//std::cout << " Wrong params. " << std::endl ;
		//std::cout << std::endl << " AppName.exe withBads withoutBads target " << std::endl ;
	}
	 
	return 0;
}
