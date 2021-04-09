#include "xoranalyzer.h"

int XorAnalyzerFunc(int argc, wchar_t* argv[])
{
	if (argc == 4)
	{
		std::wstring dumpFilename = argv[1];
		std::wstring xorFilename = argv[2];
		int block_size = std::stoi(argv[3]);

		IO::File dumpFile(dumpFilename);
		dumpFile.OpenRead();

		IO::File xorFile(xorFilename);
		xorFile.OpenCreate();

		XorAnalyzer xor_analyzer(dumpFile, xorFilename, block_size);
		xor_analyzer.Analize();

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


int wmain(int argc, wchar_t* argv[])
{
	auto result = XorAnalyzerFunc(argc , argv);
	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return result;
}