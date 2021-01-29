#pragma once

#include "file.h"
#include "diskdevice.h"
#include <filesystem>
namespace fs = std::filesystem;

#include <sstream>

namespace IO
{
	inline bool hexStringToValue(const path_string& hex_string, uint64_t& resValue)
	{
		// test is hex values;
		//spath_string tmp = path_string(L"0x") + hex_string;
		std::wstringstream ss;
		ss << std::hex << hex_string;
		if (ss.fail())
			return false;

		ss >> resValue;
		return true;
		//try
		//{
		//	resValue = lexical_cast<uint64_t>(tmp.c_str());
		//	return true;
		//}
		//catch (const boost::bad_lexical_cast& ex)
		//{
		//	
		//}
		//return false;
	}

	class Sparsebundle
	{
	private:
		IODevicePtr diskDevice_;
		uint64_t blockSize_ = 0;
		File logFile_;
		uint64_t written_size_ = 0;
	public:
		Sparsebundle(IODevicePtr target_device)
			:diskDevice_(target_device)
			, logFile_(L"log.txt")
		{
			logFile_.OpenCreate();
		}
		void setBlockSize(uint64_t block_size)
		{
			blockSize_ = block_size;
		}
		uint64_t writtenSize() const
		{
			return written_size_;
		}
		bool saveFileToDisk(const IO::path_string& filePath)
		{
			if (!diskDevice_->isOpen())
			{
				if (!diskDevice_->Open(OpenMode::OpenWrite))
				{
					wprintf(L"Error open target disk");
					return false;
				}
			}
			uint64_t write_offset = 0;

			fs::path full_name(filePath);
			auto file_name = full_name.filename();

			if (!hexStringToValue(file_name.generic_wstring(), write_offset))
			{
				logFile_.WriteText(file_name.generic_string() + "\r\n");
				return false;
			}
			write_offset *= blockSize_;
			wprintf(L"Write size %I64u(bytes) for %s \r\n", write_offset, filePath.c_str());

			if (write_offset > diskDevice_->Size())
				return false;

			File srcFile(filePath);
			if (!srcFile.Open(IO::OpenMode::OpenRead))
			{
				wprintf(L"Error open %s \n", filePath.c_str());
				return false;
			}


			auto bResult = writeToDisk(srcFile, write_offset);
			if (bResult)
				written_size_ += blockSize_;
			return bResult;


		}

		bool writeToDisk(File& theFile, const uint64_t target_offset)
		{
			uint64_t data_pos = 0;
			auto write_size = theFile.Size();
			uint32_t bytes_to_read = 0;
			auto transfer_size = default_block_size;

			auto data_array = IO::makeDataArray(transfer_size);

			uint32_t bytes_read = 0;
			uint32_t bytes_written = 0;

			uint64_t src_position = 0;
			uint64_t dst_position = target_offset;

			while (data_pos < write_size)
			{
				// read from file
				bytes_to_read = calcBlockSize(data_pos, write_size, transfer_size);
				theFile.setPosition(src_position);
				bytes_read = theFile.ReadData(data_array->data(), bytes_to_read);
				if (bytes_read == 0)
				{
					wprintf(L"Error read from file.\n");
					return false;
				}

				// write to disk
				diskDevice_->setPosition(dst_position);
				bytes_written = diskDevice_->WriteData(data_array->data(), bytes_read);
				if (bytes_written == 0)
				{
					wprintf(L"Error write to disk.\n");
					return false;
				}

				data_pos += bytes_read;
				src_position += bytes_read;
				dst_position += bytes_read;

			}
			wprintf(L"Successfully write from %s.\n", theFile.getFileName().c_str());
			return true;
		}
	};
}

