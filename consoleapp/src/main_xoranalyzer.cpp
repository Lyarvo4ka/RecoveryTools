#include "xoranalyzer.h"

#include "io/entropy.h"

void xorDiskWithSector(const uint32_t disk_number, const IO::path_string& filepath)
{
	auto listDisk = IO::ReadPhysicalDrives();
	auto physicalDrive = listDisk.find_by_number(disk_number);
	if (physicalDrive == nullptr)
		return;

	IO::DiskDevice disk(physicalDrive);
	disk.Open(IO::OpenMode::OpenRead);

	IO::File fileSector(filepath);
	fileSector.OpenRead();
	IO::DataArray sector(default_sector_size);
	fileSector.ReadData(sector);

	IO::File target(filepath + L".result");
	target.OpenCreate();

	uint64_t offset = 0;
	IO::DataArray buff(default_block_size);

	while (offset < disk.Size())
	{
		disk.setPosition(offset);
		disk.ReadData(buff.data(), buff.size());

		for (uint32_t i = 0; i < buff.size(); ++i)
		{
			buff[i] = buff[i] ^ sector[i % 512];
		}

		target.WriteData(buff.data(), buff.size());

		offset += buff.size();
	}

}

//int XorAnalyzerFunc(int argc, wchar_t* argv[])
//{
//	if (argc == 4)
//	{
//		std::wstring dumpFilename = argv[1];
//		std::wstring xorFilename = argv[2];
//		int block_size = std::stoi(argv[3]);
//
//		IO::File dumpFile(dumpFilename);
//		dumpFile.OpenRead();
//
//		IO::File xorFile(xorFilename);
//		xorFile.OpenCreate();
//
//		XorAnalyzer xor_analyzer(dumpFile, xorFilename, block_size);
//		xor_analyzer.Analize();
//
//	}
//	else
//	{
//		printf_s("Wrong params\r\n");
//		printf_s("1 - source file\r\n");
//		printf_s("2 - xor file\r\n");
//		printf_s("3 - xor size\r\n");
//
//	}
//	return 0;
//}

int XorFilesFunc(int argc, wchar_t* argv[])
{
	if (argc == 4)
	{
		std::wstring filename1 = argv[1];
		std::wstring filename2 = argv[2];
		std::wstring resultname = argv[3];
		xor_files(filename1, filename2, resultname);


	}
	else
	{
		printf_s("Wrong params\r\n");
		printf_s("1 - source file\r\n");
		printf_s("2 - xor file\r\n");
		printf_s("3 - xor size\r\n");

	}
	return 0;
}

void myltipyBy(IO::path_string & filename , uint64_t nValue)
{
	IO::File srcFile(filename);
	srcFile.OpenRead();

	IO::path_string targetFilename = filename + L".file";
	IO::File targetFile(targetFilename);
	targetFile.OpenCreate();

	IO::DataArray buff(srcFile.Size());
	srcFile.ReadData(buff);

	for (uint64_t i = 0; i < nValue; ++i)
	{
		targetFile.WriteData(buff.data(), buff.size());
	}
	

}

void myltipyBy_mainFunc(int argc, wchar_t* argv[])
{
	if (argc == 3)
	{
		std::wstring filename = argv[1];
		uint64_t mulValue = std::stoll(argv[2]);
		myltipyBy(filename, mulValue);
	}

}


void splitByPages(const IO::path_string& filename , const IO::path_string folder)
{
	fs::path filePath(filename);
	IO::File source(filename);
	source.OpenRead();

	//auto folderPath = filePath.parent_path();
	IO::File file0(folder + L"file0");
	file0.OpenCreate();

	IO::File file1(folder + L"file1");
	file1.OpenCreate();

	IO::File file2(folder + L"file2");
	file2.OpenCreate();
	
	IO::DataArray buffer(18432);

	uint64_t offset = 0;
	uint64_t nCount = 0;
	while (offset < source.Size())
	{
		source.setPosition(offset);
		source.ReadData(buffer);

		switch (nCount % 3)
		{
		case 0 :
			file0.WriteData(buffer.data(), buffer.size());
			break;
		case 1:
			file1.WriteData(buffer.data(), buffer.size());
			break;
		case 2:
			file2.WriteData(buffer.data(), buffer.size());
			break;
		default:
			throw "should never call";
		}

		++nCount;
		offset += buffer.size();
	}

}


int wmain(int argc, wchar_t* argv[])
{
	//xorWithBlock_main(argc, argv);
	IO::path_string filename = LR"(d:\52729\102481)";
	xorDiskWithSector(4, filename);
	//myltipyBy_mainFunc(argc , argv);
	//XorFilesFunc(argc, argv);
	//IO::path_string filename = LR"(d:\52729\102351)";
	//IO::path_string folder = LR"(y:\50452\)";
	//splitByPages(filename, folder);
	//IO::calcEntropyForFolder(folder, 32768);
	

	//auto result = XorAnalyzerFunc(argc , argv);
	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return 0;
}