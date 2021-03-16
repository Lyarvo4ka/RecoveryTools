
#include "raw/ext4_raw.h"


int wmain(int argc, wchar_t* argv[])
{
	auto filename = LR"(f:\49417.img)";
	auto file0depth = LR"(f:\extents\0-depth.offsets)";
	auto filePtr = IO::makeFilePtr(filename);
	filePtr->OpenRead();

	RAW::ext4_raw ext4_recovery(filePtr);
	//
	//ext4_recovery.findExtentsWithDepth(0, LR"(f:\extents\0-depth.offsets)");

	auto target_folder = LR"(h:\raw\)";
	auto listOffsets = ext4_recovery.readOffsetsFromFile(file0depth);
	for (auto offset : listOffsets)
	{
		if (ext4_recovery.readFirstSectorFromExtent(offset))
		{
			std::cout  << "FOUND " << offset <<std::endl;
			auto block_number = offset / 4096;
			auto targetfilename = IO::offsetToPath(target_folder, offset, L".zip");
			IO::File target_file(targetfilename);
			target_file.OpenCreate();
			ext4_recovery.saveToFile(block_number, target_file);
			int k = 1;
		}

	}

	// RAW::ext4_raw ext4_recovery(src_file);
	// ext4_recovery.readExtentsListFromFile(offsetsFileName);

	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return 0;
}
