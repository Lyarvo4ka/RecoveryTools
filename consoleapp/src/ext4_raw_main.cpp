
#include "raw/ext4_raw.h"
#include "consolerawparser.h"

#include <boost/lexical_cast.hpp>

#include "raw/quicktime.h"

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


int saveOffsetOfExtents(int argc, char* argv[])
{
	/*
	ext4_raw.exe 1. - offset 2.offset_val 3.(-d - f) 4.path 5.target
*/

	int argc_new = argc;
	uint16_t depth = 0;
	if (argc_new == ConsoleRawParser::param_count + 1)
	{
		depth = boost::lexical_cast<uint16_t> (argv[ConsoleRawParser::param_count]);
		--argc_new;
	}
	ConsoleRawParser ext4ConsoleParser;
	ext4ConsoleParser.parse(argc_new, argv);
	if (ext4ConsoleParser.isValid())
	{
		searchExtents_main(ext4ConsoleParser , depth);
	}
	else
		printErrorWrongParams();
	
	return 0;
}


int caculateEachExtentSize(int argc, char* argv[])
{
	ConsoleRawParser ext4ConsoleParser;
	ext4ConsoleParser.parse(argc, argv);
	if(!ext4ConsoleParser.isValid())
		return 0;

	auto iodevice = ext4ConsoleParser.getDevice();
	if (!iodevice->isOpen())
		iodevice->Open(IO::OpenMode::OpenRead);

	RAW::ext4_raw ext4_recovery(iodevice);
	auto listOffsets = ext4_recovery.readOffsetsFromFile(ext4ConsoleParser.getTargetValue());
		
	IO::DataArray extentBuffer(4096);
	for (const auto& offset : listOffsets)
	{
		auto block_number = offset / 4096;
		ext4_recovery.readExtent(block_number, extentBuffer);
		auto target_size = ext4_recovery.CalculateExtentSize(extentBuffer);
		std::cout <<std::endl << " offset " << offset << " size : " << target_size << std::endl;
		int k = 1;
		k = 2;
	}
	return 0;
}

int saveFilesWithExt4Extent(int argc, char* argv[])
{

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

	//IO::path_string targetfilename = LR"(d:\51098\file.raw)";
	//IO::File target_file(targetfilename);
	//target_file.OpenCreate();

	IO::path_string offsetExtents_txt = LR"(o:\51214\depth\1.depth)";

	ext4ConsoleParser.parse(argc_new, argv);
	if (ext4ConsoleParser.isValid())
	{
		auto sourceDevice = ext4ConsoleParser.getDevice();
		auto offsetsFileName = ext4ConsoleParser.getTargetValue();
		sourceDevice->Open(IO::OpenMode::OpenRead);
		RAW::ext4_raw ext4_recovery(sourceDevice);
		
		//ext4_recovery.setOffsetsFileName(offsetExtents_txt);

		IO::path_string target_folder = LR"(d:\51210\)";
		//uint64_t offset = 0x2a02020000; 
	//0x19090F8000; - file 80 Gb
		 
		auto listOffsets = ext4_recovery.readOffsetsFromFile(offsetsFileName);
		for (const auto& offset : listOffsets)
		{
			auto block_number = offset / 4096;
			auto targetfilename = IO::offsetToPath(target_folder, offset, L".ext");
			IO::File target_file(targetfilename);
			target_file.OpenCreate();
			ext4_recovery.saveToFile(block_number, target_file); 
		}
	}
	return 0;
}

int saveZipFilesWithExt4Extent(int argc, char* argv[])
{
	/*
	ext4_raw.exe 1. soruce 2. path_extents.txt 3. target_folder
	*/
	int argc_new = argc;
	IO::path_string target_folder = LR"(l:\20Tb\)";

	if (argc_new == ConsoleRawParser::param_count + 1)
	{
		std::string dst_folder = argv[ConsoleRawParser::param_count];
		--argc_new;
	}

	ConsoleRawParser ext4ConsoleParser;
	ext4ConsoleParser.parse(argc_new, argv);
	if (ext4ConsoleParser.isValid())
	{
		auto sourceDevice = ext4ConsoleParser.getDevice();
		auto offsetsFileName = ext4ConsoleParser.getTargetValue();
		sourceDevice->Open(IO::OpenMode::OpenRead);
		RAW::ext4_raw ext4_recovery(sourceDevice);

		//IO::path_string target_folder = LR"(g:\result\)";
		//uint64_t offset = 0x4c0202f000;
		auto blockSize = ext4_recovery.getBlockSize();
		IO::DataArray extent(blockSize);

		//std::string textToFind = "buh_75";
		//IO::path_string fileName = IO::path_string(textToFind.begin(), textToFind.end());

		//IO::path_string txtPath = LR"(d:\PaboTa\20Tb\)" + fileName + L".txt";
		//IO::File text_txt(txtPath);
		//text_txt.OpenCreate();

		uint64_t office05 = 3191326949376;
		uint64_t office02 = 3191327145984;  // 06_02_2021 --> 3191326941184; 
			


		//auto block_number = office02 / 4096;
		//auto targetfilename = IO::offsetToPath(target_folder, office02, L".zip");
		//IO::File target_file(targetfilename);
		//target_file.OpenCreate();
		//ext4_recovery.saveToFile(block_number, target_file);

		//target_file.Close();


		auto listOffsets = ext4_recovery.readOffsetsFromFile(offsetsFileName);

		for (const auto& offset : listOffsets)
		{
			ext4_recovery.readExtent(offset / blockSize, extent);
			RAW::EXTENT_BLOCK* extent_block = (RAW::EXTENT_BLOCK*)extent.data();
			if (extent_block->header.depth == 0) 
				if (extent_block->header.entries > 0)
				{
					auto target_offset = extent_block->extent[0].PysicalBlock() * blockSize;
					auto data_size = ext4_recovery.CalculateExtentSize(offset / blockSize);
					//////////////////////////////////////////////////
					auto block_number = offset / 4096;
					auto targetfilename = IO::offsetToPath(target_folder, offset, L".zip");
					IO::File target_file(targetfilename);
					target_file.OpenCreate();
					ext4_recovery.saveToFile(block_number, target_file);
					////////////////////////////////
					//IO::DataArray sector(512);
					//sourceDevice->setPosition(target_offset);
					//sourceDevice->ReadData(sector.data(), sector.size()); 

					//uint32_t pos = 0;
					//if (RAW::findTextTnBlock(sector, textToFind, pos))
					//{
					//	std::cout << "FOUND " << offset << std::endl;
					//	auto text = RAW::StringConverter::toString(offset);
					//	text_txt.WriteText(text + "\n");
					//}


					int k = 1;
				}


			//auto data_size = ext4_recovery.CalculateExtentSize(offset / blockSize);
			//if (data_size > 230000000 && data_size < 238000000)
			//{
	//			std::cout << "FOUND " << offset << std::endl;
	//			auto text = RAW::StringConverter::toString(offset);
	//			text_txt.WriteText(text + "\n");
				//auto block_number = offset / 4096;
				//auto targetfilename = IO::offsetToPath(target_folder, offset, L".zip");
				//IO::File target_file(targetfilename);
				//target_file.OpenCreate();
				//ext4_recovery.saveToFile(block_number, target_file);

			//}

			//if (ext4_recovery.readFirstSectorFromExtent(offset))
			//{
			//	std::cout  << "FOUND " << offset <<std::endl;
			//	auto text = RAW::StringConverter::toString(offset);
			//	zipHeader_txt.WriteText(text + "\n");
			//	//auto block_number = offset / 4096;
			//	//auto targetfilename = IO::offsetToPath(target_folder, offset, L".zip");
			//	//IO::File target_file(targetfilename);
			//	//target_file.OpenCreate();
			//	//ext4_recovery.saveToFile(block_number, target_file);
			//	//int k = 1;
			//}

		}
	}
	return 0;
}


int main(int argc, char* argv[])
{
	//ConsoleRawParser ext4ConsoleParser;
	//ext4ConsoleParser.parse(argc, argv);
	//if (ext4ConsoleParser.isValid())
	//{
	//	const uint16_t maxDepth = 65535;
	//	auto device = ext4ConsoleParser.getDevice();
	//	device->Open(IO::OpenMode::OpenRead);
	//	RAW::ext4_raw ext4Recovery(device);
	//	ext4Recovery.searchExtents(ext4ConsoleParser.getOffset(), ext4ConsoleParser.getTargetValue(), maxDepth);  
	//}

	saveFilesWithExt4Extent(argc, argv);
	//saveZipFilesWithExt4Extent(argc, argv);

	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return 0;
}
