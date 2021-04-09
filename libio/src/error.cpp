#include "IO\error.h"

namespace IO 
{
	namespace Error
	{
		IOStatus::IOStatus(IOErrorsType error_code, const std::string& error_message, const uint32_t last_error)
			: error_code_(error_code)
			, last_error_(last_error)
			, error_message_(error_message)
		{
		}
		IOStatus::IOStatus(IOErrorsType error_code, uint32_t last_error)
			: error_code_(error_code)
			, last_error_(last_error)
		{
		}
		IOStatus::IOStatus()
		{

		}
		bool IOStatus::isOK()
		{
			return error_code_ == IOErrorsType::OK;
		}
		bool IOStatus::isFailed()
		{
			return error_code_ != IOErrorsType::OK;
		}
		IOErrorsType IOStatus::code() const
		{
			return error_code_;
		}
		void IOStatus::setLastError(uint32_t last_error)
		{
			last_error_ = last_error;
		}
		uint32_t IOStatus::lastError() const
		{
			return last_error_;
		}
		std::string IOStatus::error_message() const
		{
			return error_message_;
		}
		IOErrorException::IOErrorException(IOStatus error_status)
			:error_status_(error_status)
		{

		}

		//}

		IOStatus IOErrorException::getStatus() const
		{
			return error_status_;
		}
		const char* IOErrorException::what() const
		{
			tmp_str = error_status_.error_message() + LastErrorMessage(error_status_.lastError());
			return tmp_str.c_str();
		}
	}

	ErrorHandler* ErrorHandler::error_handler_ = ErrorHandler::defaultHandler();

	ErrorHandler* ErrorHandler::defaultHandler()
	{
		static SingletonHolder<ErrorHandler> sh;
		return sh.get();
	}


}