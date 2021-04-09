#pragma once

#include "iodevice.h"

namespace IO
{
	class File
		: public IODevice
	{
	private:
		IOEnginePTR io_engine_ ;
		uint64_t size_;
		path_string file_path_;
		DeviceInfo deviceInfo_;

	public:
		File(const path_string& file_path);
		~File();

		bool OpenRead();
		bool OpenWrite();
		bool OpenCreate();

		bool Open(OpenMode openMode) override;


		void Close() override;
		bool isOpen() override;

		void setPosition(uint64_t offset) override;
		uint64_t getPosition() const override;

		uint32_t ReadData(ByteArray data, uint32_t read_size) override;
		uint32_t ReadData(DataArray& data_array);

		uint32_t WriteText(const std::string_view text_data);
		uint32_t WriteData(ByteArray data, uint32_t write_size) override;;

		uint64_t Size() const override;
		void setSize(uint64_t new_size);

		void setFileName(const path_string new_filename);
		path_string getFileName() const;
		void readFileSize(uint64_t& file_size);
		DeviceInfo getDeviceInfo() const override;

		void setIOEngine(IOEnginePTR new_ioengine);
		IOEnginePTR getIOEngine();
	};

	using FilePtr = std::shared_ptr<File>;
	FilePtr makeFilePtr(const path_string& file_path);

}
