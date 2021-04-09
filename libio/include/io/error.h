#pragma once
#include <system_error>
#include "windows.h"
#include "constants.h"
//#include <string_view>

namespace IO
{
	


	namespace Error
	{
		const std::string OpenRead_str = "Error opening the file for reading.";
		const std::string OpenWrite_str = "Error opening file for writing.";
		const std::string Create_str = "Error creating file.";
		const std::string ReadData_str = "Error reading from file.";
		const std::string WriteData_str = "Error writing to file.";
		const std::string GetFileSize_str = "Error to get file size.";
		const std::string Unknown_str = "Unknown error";

		enum class IOErrorsType
		{
			OK = 0,
			kOpenRead ,
			kOpenWrite,
			kOpen,
			kCreate,
			kOpenPhysicalDrive,
			kReadData,
			kWriteData,
			kGetFileSize,
			kSetFileSize,
			kWrongParam,
			kUnknown

		};

		inline IOErrorsType OpenModeToError(IO::OpenMode open_mode)
		{
			switch (open_mode)
			{
			case IO::OpenMode::OpenRead:
				return IOErrorsType::kOpenRead;
			case IO::OpenMode::OpenWrite:
				return IOErrorsType::kOpenWrite;
			case IO::OpenMode::Create:
				return IOErrorsType::kCreate;
			default:
				return IOErrorsType::kUnknown;
			}
		}

		static inline std::string getDiskOrFileError(const IOErrorsType error, const std::string & sourceName)
		{
			std::string resString;
			switch (error)
			{
			case IOErrorsType::kOpenRead:
				return "Error opening the " + sourceName + " for reading.";
			case IOErrorsType::kOpenWrite:
				return "Error opening " + sourceName + " for writing.";
			case IOErrorsType::kCreate:
				return "Error creating " + sourceName;
			case IOErrorsType::kOpenPhysicalDrive:
				return "Error open physical " + sourceName;
			case IOErrorsType::kReadData:
				return "Error reading from " + sourceName;
			case IOErrorsType::kWriteData:
				return "Error writing to " + sourceName;
			case IOErrorsType::kGetFileSize:
				return "Error to get file size from " + sourceName;
			case IOErrorsType::kSetFileSize:
				return "Erro to set file size to " + sourceName;
			default:
				return Unknown_str;
			}
		}

		static std::string LastErrorMessage(uint32_t lastError)
		{
			std::string errMsg;
			DWORD dwFlg = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

			LPSTR lpMsgBuf = 0;
			if (FormatMessageA(dwFlg, 0, lastError, 0, (LPSTR)& lpMsgBuf, 0, NULL))
				errMsg = lpMsgBuf;

			//				UnicodeConverter::toUTF8(lpMsgBuf, errMsg);
			//#else
			//			LPTSTR lpMsgBuf = 0;
			//			if (FormatMessageA(dwFlg, 0, errorCode, 0, (LPTSTR)& lpMsgBuf, 0, NULL))
			//				errMsg = lpMsgBuf;
			//#endif
			LocalFree(lpMsgBuf);
			return errMsg;
		}


		class IOStatus
		{
		private:
			IOErrorsType error_code_ = IOErrorsType::OK;
			uint32_t last_error_  = 0;
			std::string error_message_;


		public:
			IOStatus(IOErrorsType error_code, const std::string& error_message, const uint32_t last_error);

			IOStatus(IOErrorsType error_code, uint32_t last_error);
			IOStatus();
			static IOStatus OK()
			{
				return IOStatus();
			}
			bool isOK();
			bool isFailed();
			IOErrorsType code() const;
			void setLastError(uint32_t last_error);
			uint32_t lastError() const;
			std::string error_message() const;
		};

		class IOErrorException
			: public std::exception
		{
		private :
			IOStatus error_status_;
			mutable std::string tmp_str;
		public:
			IOErrorException(IOStatus error_status);
			//IOErrorException()
			//{

			//}
			IOStatus getStatus() const;
			const char* what() const override;
		};



	}
	template <class S>
	class SingletonHolder
		/// This is a helper template class for managing
		/// singleton objects allocated on the heap.
		/// The class ensures proper deletion (including
		/// calling of the destructor) of singleton objects
		/// when the application that created them terminates.
	{
	public:
		SingletonHolder() :
			_pS(0)
			/// Creates the SingletonHolder.
		{
		}

		~SingletonHolder()
			/// Destroys the SingletonHolder and the singleton
			/// object that it holds.
		{
			delete _pS;
		}

		S* get()
			/// Returns a pointer to the singleton object
			/// hold by the SingletonHolder. The first call
			/// to get will create the singleton.
		{
			//FastMutex::ScopedLock lock(_m);
			if (!_pS) _pS = new S;
			return _pS;
		}

	private:
		S* _pS;
		//FastMutex _m;
	};

	enum class ErrorLevel
	{
		none, // (turns off logging)
 	    fatal,
	    critical,
	    error,
	    warning,
	    notice,
	    information,
	    debug,
	    trace
	};

	constexpr ErrorLevel defaulErrorLevel()
	{
		return ErrorLevel::error;
	}



	class ErrorHandler
	{
	private: 
		static ErrorHandler * error_handler_;
	protected:
		static ErrorHandler* defaultHandler();
	public:
		static ErrorHandler * get()
		{
			return error_handler_;
		}
		void showMessage(const std::wstring & messageText)
		{
			wprintf(messageText.c_str());
			wprintf(L"\n");
		}
		void showMessage(const std::string & messageText)
		{
			printf(messageText.c_str());
			printf("\n");
		}
		std::wstring getMessage(uint32_t errorCode)
		{
			std::wstring errMsg;
			DWORD dwFlg = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

			LPWSTR lpMsgBuf = 0;
			if (FormatMessageW(dwFlg, 0, errorCode, 0, (LPWSTR)& lpMsgBuf, 0, NULL))
				errMsg = lpMsgBuf;

			//				UnicodeConverter::toUTF8(lpMsgBuf, errMsg);
			//#else
			//			LPTSTR lpMsgBuf = 0;
			//			if (FormatMessageA(dwFlg, 0, errorCode, 0, (LPTSTR)& lpMsgBuf, 0, NULL))
			//				errMsg = lpMsgBuf;
			//#endif
			LocalFree(lpMsgBuf);
			return errMsg;
		}


	};

	static void LOG_MESSAGE(const std::wstring & messageText)
	{
		ErrorHandler::get()->showMessage(messageText);
	}
	static void LOG_MESSAGE(const std::string & messageText)
	{
		ErrorHandler::get()->showMessage(messageText);
	}


	//static constexpr auto ErrorLevelStr = std::string_view::make_literal_array(
	//	"none",
	//	"fatal",
	//	"critical",
	//	"error",
	//	"warning",
	//	"notice",
	//	"information",
	//	"debug",
	//	"trace"

	//);


}