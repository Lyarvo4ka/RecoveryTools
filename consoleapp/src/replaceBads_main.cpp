#include "io/utility.h"

enum  ReplaceBadsParams { WITH_BADS = 1, WITHOUT_BADS, TARGET, PARAM_COUNT };


#include "io/onec.h"

int wmain(int argc, wchar_t* argv[])
{

	IO::RestoreRootObject();
	//if ( argc == PARAM_COUNT)
	//{
	//	
	//	IO::path_string withBads = argv[WITH_BADS];
	//	IO::path_string withoutBads = argv[WITHOUT_BADS];
	//	IO::path_string target = argv[TARGET];

	//	//fixAllDbfFiles(foldername);
	//	IO::replaceBadsFromOtherFile(withBads, withoutBads, target);
	//	 
	//	_CrtDumpMemoryLeaks();
	//	std::cout << std::endl << " FINISHED "; 
	//}
	//else
	//{
	//	std::cout << " Wrong params. " << std::endl ;
	//	std::cout << std::endl << " AppName.exe withBads withoutBads target " << std::endl ;
	//}
	 
	return 0;
}
