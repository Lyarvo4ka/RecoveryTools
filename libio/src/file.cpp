#include "io/file.h"
#include <iostream>

namespace IO
{
	const std::string_view file_txt = "file";

	File::File(const path_string& file_path)
		: size_(0)
		, file_path_(file_path)
		, io_engine_(makeIOEngine())
	{
		deviceInfo_.deviceTypeName = file_txt;
		//std::cout << "File created " << std::endl;
	}

	File::~File()
	{
		Close();
		//std::cout << "File closed " << std::endl;
	}

	bool File::OpenRead()
	{
		return Open(OpenMode::OpenRead);
	}

	bool File::OpenWrite()
	{
		return Open(OpenMode::OpenWrite);
	}

	bool File::OpenCreate()
	{
		return Open(OpenMode::Create);
	}

	bool File::Open(OpenMode openMode)
	{
		Error::IOErrorsType result = Error::IOErrorsType::OK;
		switch (openMode)
		{
		case OpenMode::OpenRead:
			result = io_engine_->OpenRead(file_path_);
			break;
		case OpenMode::OpenWrite:
			result = io_engine_->OpenWrite(file_path_);
			break;
		case OpenMode::Create:
			result = io_engine_->Create(file_path_);
			break;
		}

		if (result != Error::IOErrorsType::OK)
		{
			Error::IOErrorException error_exception(makeErrorStatus(*this, result));
			throw error_exception;
		}

		readFileSize(size_);

		return io_engine_->isOpen();
	}

	void File::Close()
	{
		io_engine_->Close();
	}

	bool File::isOpen()
	{
		return io_engine_->isOpen();
	}

	void File::setPosition(uint64_t offset)
	{
		io_engine_->setPosition(offset);
	}

	uint64_t File::getPosition() const
	{
		return io_engine_->getPosition();
	}

	uint32_t File::ReadData(ByteArray data, uint32_t read_size)
	{
		uint32_t bytes_read = 0;

		auto result = io_engine_->Read(data, read_size, bytes_read);
		if (result == Error::IOErrorsType::OK)
			return bytes_read;

		throw Error::IOErrorException(makeErrorStatus(*this, result));
	}

	uint32_t File::ReadData(DataArray& data_array)
	{
		return ReadData(data_array.data(), data_array.size());
	}

	uint32_t File::WriteText(const std::string_view text_data)
	{
		return WriteData((ByteArray)text_data.data(), static_cast<uint32_t>(text_data.length()));
	}

	uint32_t File::WriteData(ByteArray data, uint32_t write_size)
	{
		uint32_t bytes_written = 0;
		auto result = io_engine_->Write(data, write_size, bytes_written);
		if (result == Error::IOErrorsType::OK)
		{
			size_ += bytes_written;
			return bytes_written;
		}

		//	//ERROR_DISK_FULL
		throw Error::IOErrorException(makeErrorStatus(*this, result));
	}

	uint64_t File::Size() const
	{
		return size_;
	}

	void File::setSize(uint64_t new_size)
	{
		if (auto status = makeErrorStatus(*this, io_engine_->SetFileSize(new_size)); status.isFailed())
			throw Error::IOErrorException(status);
		size_ = new_size;
	}

	void File::setFileName(const path_string new_filename)
	{
		Close();
		file_path_ = new_filename;
	}

	path_string File::getFileName() const
	{
		return file_path_;
	}

	void File::readFileSize(uint64_t& file_size)
	{
		if ( auto status = makeErrorStatus(*this, io_engine_->readFileSize(file_size)); status.isFailed() )
			throw Error::IOErrorException(status);
	}

	inline DeviceInfo File::getDeviceInfo() const
	{
		return deviceInfo_;
	}

	inline void File::setIOEngine(IOEnginePTR new_ioengine)
	{
		io_engine_ = new_ioengine;
	}

	inline IOEnginePTR File::getIOEngine()
	{
		return io_engine_;
	}

	FilePtr makeFilePtr(const path_string& file_path)
	{
		return std::make_shared<File>(file_path);
	}

}