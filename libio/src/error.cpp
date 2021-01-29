#include "IO\error.h"

namespace IO 
{
	ErrorHandler* ErrorHandler::error_handler_ = ErrorHandler::defaultHandler();

	ErrorHandler* ErrorHandler::defaultHandler()
	{
		static SingletonHolder<ErrorHandler> sh;
		return sh.get();
	}


}