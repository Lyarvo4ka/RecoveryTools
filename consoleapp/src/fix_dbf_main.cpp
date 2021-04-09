
#include "io/dbf.h"
#include "io/finder.h"

void fixAllDbfFiles(const IO::path_string & folder)
{
	IO::Finder finder;
	finder.add_extension(L".dbf");
	finder.FindFiles(folder);

	for (auto filename : finder.getFiles())
	{
		IO::fixDBF(filename);
	}


	//fixDBF
}

int wmain(int argc, wchar_t* argv[])
{
	auto foldername = LR"(d:\PaboTa\49529\to_fix\)";
	fixAllDbfFiles(foldername);

	_CrtDumpMemoryLeaks();
	std::cout << std::endl << " FINISHED ";
	return 0;
}
