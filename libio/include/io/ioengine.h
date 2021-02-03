#pragma once

#include <functional>

#include "IO/dataarray.h"
#include "IO/error.h"



namespace IO
{
	using namespace Error;

	using read_or_write_func = std::function<Error::IOErrorsType(ByteArray, const uint32_t, uint32_t &)>;

	inline uint32_t calcBlockSize(uint64_t current, uint64_t size, uint32_t block_size)
	{
		if (current > size)
			return 0;
		uint32_t bytes = 0;
		if (current + (uint64_t)block_size <= size)
			bytes = block_size;
		else
			bytes = (uint32_t)(size - current);
		return bytes;
	}

	path_string addPrefix(const path_string& path);

	enum class enReadWrite { kREAD, kWRITE };

	class IOEngine
	{
		HANDLE hDevice_ = INVALID_HANDLE_VALUE;
		uint32_t transfer_size_ = default_block_size;
		uint64_t position_ = 0;
		bool bOpen_ = false;

	public:
		virtual ~IOEngine();

		virtual IOErrorsType OpenPhysicalDrive(const path_string& path);
		virtual IOErrorsType OpenRead(const path_string & path);
		virtual IOErrorsType OpenWrite(const path_string & path);
		virtual IOErrorsType Create(const path_string & path);
		virtual void Close();
		virtual bool isOpen() const;
		virtual void setPosition(uint64_t position);
		virtual uint64_t getPosition() const;
		virtual IOErrorsType Read(ByteArray data, const uint32_t read_size, uint32_t & bytes_read);
		virtual IOErrorsType Write(ByteArray data, const uint32_t write_size, uint32_t & bytes_written);
		virtual IOErrorsType SetFileSize(uint64_t new_size);

		virtual IOErrorsType readFileSize(uint64_t & file_size);
		void setTranserSize(const uint32_t transfer_size);
		uint32_t getTranferSize() const;
		bool isParamsValid(ByteArray data, uint32_t size);
	private:

		
		virtual IOErrorsType read_data(ByteArray data, uint32_t read_size, uint32_t & bytes_read);
		virtual IOErrorsType write_data(ByteArray data, uint32_t write_size, uint32_t & bytes_written);

		virtual BOOL read_device(HANDLE & hDevice, ByteArray data, const uint32_t bytes_to_read, uint32_t & bytes_read);
		virtual BOOL write_device(HANDLE & hDevice, ByteArray data, const uint32_t bytes_to_write, uint32_t & bytes_written);
	protected:
		IOErrorsType ReadOrWriteData(ByteArray data, const uint32_t read_size, uint32_t& bytes_read, enReadWrite read_write);

	

	};

	using IOEnginePTR = std::shared_ptr<IOEngine>;
	IOEnginePTR makeIOEngine();
}