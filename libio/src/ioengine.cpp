#include "io/ioengine.h"

#include <cassert>

namespace IO
{

	IOEngine::~IOEngine()
	{
		Close();
	}
	path_string addPrefix(const path_string& path)
	{
		path_string new_string = path;
		if (new_string.size() >= MAX_PATH)
			new_string = LR"(\\?\)" + path;
		//new_string = path;
		return new_string;
	}

	IOEnginePTR makeIOEngine()
	{
		return std::make_shared<IOEngine>();
	}

	IOErrorsType IOEngine::OpenPhysicalDrive(const path_string& path)
	{
		hDevice_ = ::CreateFile(path.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (hDevice_ == INVALID_HANDLE_VALUE)
		{
			auto last_error = ::GetLastError();
			return IOErrorsType::kOpenPhysicalDrive;
		}
		bOpen_ = true;
		return IOErrorsType::OK;
	}

	IOErrorsType IOEngine::OpenRead(const path_string & path)
	{
		auto new_string = addPrefix(path);
		hDevice_ = ::CreateFile(new_string.c_str(),
			GENERIC_READ ,
			FILE_SHARE_READ ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (hDevice_ == INVALID_HANDLE_VALUE)
		{
			auto last_error = ::GetLastError();
			return IOErrorsType::kOpenRead;
		}


		bOpen_ = true;
		return IOErrorsType::OK;
	}
	IOErrorsType IOEngine::OpenWrite(const path_string & path)
	{
		auto new_string = addPrefix(path);

		hDevice_ = ::CreateFile(new_string.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (hDevice_ == INVALID_HANDLE_VALUE)
			return IOErrorsType::kOpenWrite;

		bOpen_ = true;
		return IOErrorsType::OK;
	}
	IOErrorsType IOEngine::Create(const path_string & path)
	{
		auto new_string = addPrefix(path);
		hDevice_ = ::CreateFile(new_string.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL);
		if (hDevice_ == INVALID_HANDLE_VALUE)
			return IOErrorsType::kCreate;

		bOpen_ = true;
		return IOErrorsType::OK;

	}
	void IOEngine::Close()
	{
		if (hDevice_ != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hDevice_);
			hDevice_ = INVALID_HANDLE_VALUE;
		}
		bOpen_ = false;
	}
	bool IOEngine::isOpen() const
	{
		return bOpen_;
	}
	void IOEngine::setPosition(uint64_t position)
	{
		//if (position_ != position)	??????
		{
			position_ = position;
			LARGE_INTEGER liPos = { 0 };
			liPos.QuadPart = position_;
			::SetFilePointerEx(hDevice_, liPos, NULL, FILE_BEGIN);
		}
	}
	uint64_t IOEngine::getPosition() const
	{
		return position_;
	}
	IOErrorsType IOEngine::Read(ByteArray data, const uint32_t read_size, uint32_t & bytes_read)
	{
		return ReadOrWriteData(data, read_size, bytes_read, enReadWrite::kREAD);
	}
	IOErrorsType IOEngine::Write(ByteArray data, const uint32_t write_size, uint32_t & bytes_written)
	{
		return ReadOrWriteData(data, write_size, bytes_written, enReadWrite::kWRITE);
	}
	IOErrorsType IOEngine::SetFileSize(uint64_t new_size)
	{
		LARGE_INTEGER li = LARGE_INTEGER();
		li.QuadPart = new_size;
		::SetFilePointerEx(hDevice_, li, NULL, FILE_BEGIN);
		if (auto bResult = ::SetEndOfFile(hDevice_); !bResult)
			return IOErrorsType::kSetFileSize;

		return IOErrorsType::OK;
	}
	IOErrorsType IOEngine::readFileSize(uint64_t & file_size)
	{
		LARGE_INTEGER liSize = { 0 };
		auto bResult = ::GetFileSizeEx(hDevice_, &liSize);
		if (!bResult)
			return IOErrorsType::kGetFileSize;

		file_size = liSize.QuadPart;
		return IOErrorsType::OK;
	}
	void IOEngine::setTranserSize(const uint32_t transfer_size)
	{
		transfer_size_ = transfer_size;
	}
	uint32_t IOEngine::getTranferSize() const
	{
		return transfer_size_;
	}
	bool IOEngine::isParamsValid(ByteArray data, uint32_t size)
	{
		return ((data != nullptr) && (size > 0));
	}
	IOErrorsType IOEngine::ReadOrWriteData(ByteArray data, const uint32_t read_size, uint32_t & bytes_read, enReadWrite read_write)
	{
		if (!isParamsValid(data , read_size))
			return IOErrorsType::kWrongParam;
		if (getTranferSize() == 0)
			return IOErrorsType::kWrongParam;

		uint32_t data_pos = 0;
		uint32_t bytes_to_read = 0;
		IOErrorsType result = IOErrorsType::kUnknown;
		while (data_pos < read_size)
		{
			bytes_to_read = calcBlockSize(data_pos, read_size, getTranferSize());
			setPosition(position_);
			ByteArray pData = data + data_pos;
			
			if (read_write == enReadWrite::kREAD)
				result = read_data(pData, bytes_to_read, bytes_read);
			else
				if (read_write == enReadWrite::kWRITE)
					result = write_data(pData, bytes_to_read, bytes_read);

			if( result != IOErrorsType::OK)
				return result;
			data_pos += bytes_read;
			position_ += bytes_read;
		}
		bytes_read = data_pos;
		return IOErrorsType::OK;

	}


	IOErrorsType IOEngine::read_data(ByteArray data, uint32_t read_size, uint32_t & bytes_read)
	{
		auto bResult = read_device(hDevice_, data, read_size, bytes_read);
		if (!bResult || (bytes_read == 0))
			return IOErrorsType::kReadData;

		return IOErrorsType::OK;
	}
	Error::IOErrorsType IOEngine::write_data(ByteArray data, uint32_t write_size, uint32_t & bytes_written)
	{
		auto bResult = write_device(hDevice_, data, write_size, bytes_written);
		if (!bResult || (bytes_written == 0))
			return IOErrorsType::kWriteData;

		return IOErrorsType::OK;
	}
	BOOL IOEngine::read_device(HANDLE & hDevice, ByteArray data, const uint32_t bytes_to_read, uint32_t & bytes_read)
	{
		return::ReadFile(hDevice, data, bytes_to_read, reinterpret_cast<LPDWORD>(&bytes_read), NULL);
	}
	BOOL IOEngine::write_device(HANDLE & hDevice, ByteArray data, const uint32_t bytes_to_write, uint32_t & bytes_written)
	{
		return::WriteFile(hDevice, data, bytes_to_write, reinterpret_cast<LPDWORD>(&bytes_written), NULL);
	}



}