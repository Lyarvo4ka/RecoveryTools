#include "fleshcollector.h"


int wmain(int argc, wchar_t* argv[])
{
	IO::path_string dump_folder =LR"(y:\50255\ECC_OK\)";
	IO::path_string image = LR"(y:\50255\image)";

    FleshCollector fleshCollector(dump_folder);
    fleshCollector.SaveImage(image);
}
