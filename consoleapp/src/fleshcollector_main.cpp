#include "fleshcollector.h"


int wmain(int argc, wchar_t* argv[])
{
	IO::path_string dump =LR"(e:\tmp\test)";
	IO::path_string image = LR"(e:\tmp\image)";

    FleshCollector fleshCollector(dump);
    fleshCollector.SaveImage(image);
}
