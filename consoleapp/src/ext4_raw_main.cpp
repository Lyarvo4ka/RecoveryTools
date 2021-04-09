
#include "raw/ext4_raw.h"
#include "consolerawparser.h"

#include <boost/lexical_cast.hpp>

void searchExtents_main(ConsoleRawParser& consoleExt4Parser , uint16_t depth)
{
	auto iodevice = consoleExt4Parser.getDevice();
	if (!iodevice->isOpen())
		iodevice->Open(IO::OpenMode::OpenRead);

	RAW::ext4_raw ext4_recovery(iodevice);
	try 
	{
		ext4_recovery.findExtentsWithDepth(depth, consoleExt4Parser.getTargetValue(), consoleExt4Parser.getOffset());
	}
	catch (const IO::IOErrorException& ex)
	{
		std::cout << "Cougth exception " << ex.what() << std::endl;
	}
}





int main(int argc, char* argv[])
{

/*
	ext4_raw.exe 1. - offset 2.offset_val 3.(-d - f) 4.path 5.target
*/

	//int argc_new = argc;
	//uint16_t depth = 0;
	//if (argc_new == ConsoleRawParser::param_count + 1)
	//{
	//	depth = boost::lexical_cast<uint16_t> (argv[ConsoleRawParser::param_count]);
	//	--argc_new;
	//}
	//ConsoleRawParser ext4ConsoleParser;
	//ext4ConsoleParser.parse(argc_new, argv);
	//if (ext4ConsoleParser.isValid())
	//{
	//	searchExtents_main(ext4ConsoleParser , depth);
	//}
	//else
	//	printErrorWrongParams();

/////////////////////////////////////////////////////////
	/*
	ext4_raw.exe 1. soruce 2. path_extents.txt 3. target_folder
	*/
	int argc_new = argc;

	if (argc_new == ConsoleRawParser::param_count + 1)
	{
		std::string target_folder = argv[ConsoleRawParser::param_count];
		--argc_new;
	}
	ConsoleRawParser ext4ConsoleParser;

	IO::path_string targetfilename = LR"(g:\result\new_new.raw)";
	IO::File target_file(targetfilename);
	target_file.OpenCreate();


	ext4ConsoleParser.parse(argc_new, argv);
	if (ext4ConsoleParser.isValid())
	{
		auto sourceDevice = ext4ConsoleParser.getDevice();
		auto offsetsFileName = ext4ConsoleParser.getTargetValue();
		sourceDevice->Open(IO::OpenMode::OpenRead);
		RAW::ext4_raw ext4_recovery(sourceDevice);


		//auto listOffsets = ext4_recovery.readOffsetsFromFile(offsetsFileName);
		//
		//IO::DataArray extentBuffer(4096);
		//for (const auto& offset : listOffsets)
		//{
		//	auto block_number = offset / 4096;
		//	ext4_recovery.readExtent(block_number, extentBuffer);
		//	auto target_size = ext4_recovery.CalculateExtentSize(extentBuffer);
		//	std::cout <<std::endl << " offset " << offset << " size : " << target_size << std::endl;
		//	int k = 1;
		//	k = 2;
		//}

		//IO::path_string target_folder = LR"(g:\result\)";
		//uint64_t offset = 0x4c0202f000;
		auto listOffsets = ext4_recovery.readOffsetsFromFile(offsetsFileName);
		for (const auto& offset : listOffsets)
		{
			auto block_number = offset / 4096;
			//auto targetfilename = IO::offsetToPath(target_folder, offset, L".bak");
			//IO::File target_file(targetfilename);
			//target_file.OpenCreate();
			ext4_recovery.saveToFile(block_number, target_file);
		}
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////
	//auto filename = LR"(f:\49417.img)";

	//auto target_folder = LR"(g:\raw_new\)";
	//auto listOffsets = ext4_recovery.readOffsetsFromFile(file0depth);
	//for (auto offset : listOffsets)
	//{
	                               
	//	if (ext4_recovery.readFirstSectorFromExtent(offset))
	//	{
	//		std::cout  << "FOUND " << offset <<std::endl;
	//		auto block_number = offset / 4096;
	//		auto targetfilename = IO::offsetToPath(target_folder, offset, L".zip");
	//		IO::File target_file(targetfilename);
	//		target_file.OpenCreate();
	//		ext4_recovery.saveToFile(block_number, target_file);
	//		int k = 1;
	//	}

	//}

	// RAW::ext4_raw ext4_recovery(src_file);
	// ext4_recovery.readExtentsListFromFile(offsetsFileName);

	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return 0;
}
