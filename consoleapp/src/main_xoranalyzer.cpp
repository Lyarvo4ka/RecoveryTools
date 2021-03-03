#include "io/XorAnalyzer.h"

int XorAnalyzer(int argc, wchar_t* argv[])
{
	if (argc == 4)
	{
		std::wstring srcFileName = argv[1];
		std::wstring xorFileName = argv[2];
		int block_size = std::stoi(argv[3]);


		IO::XorAnalyzer xor_analyzer(srcFileName);
		xor_analyzer.Analize(xorFileName, block_size);

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
	auto result = XorAnalyzer(argc , argv);
	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return result;
}