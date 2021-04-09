#pragma once
#include "io/iodevice.h"
#include "io/utility.h"
#include "io/finder.h"

  class FileOffsetWriter
	{
	private:
		IO::IODevicePtr targetDevice_;
		uint32_t sectorSize_ = {default_sector_size};
	public:
		FileOffsetWriter(IO::IODevicePtr targetDevice)
			:targetDevice_(targetDevice)
		{

		}

		void writeImageFiles(const IO::path_string source_folder)
		{
			if (!targetDevice_->isOpen())
				if (!targetDevice_->Open(IO::OpenMode::Create))
				{
					wprintf(L"Error open target disk");
					return;
				}

			IO::Finder finder;
			finder.add_extension(L".bin");
			finder.FindFiles(source_folder);

			for (const auto & sourceFilePath :  finder.getFiles())
			{
				auto write_offset = IO::fileNameToOffset(sourceFilePath);
				write_offset *= default_sector_size;
				wprintf(L"Write size %I64u(bytes) for %s \r\n", write_offset, sourceFilePath.c_str());
				writeToImage(sourceFilePath , write_offset);
				
			}
		}

		bool writeToImage(IO::path_string sourceFilename, const uint64_t target_offset )
		{

			IO::File srcFile(sourceFilename);
			if (!srcFile.Open(IO::OpenMode::OpenRead))
			{
				wprintf(L"Error open %s \n", sourceFilename.c_str());
				return false;
			}

			uint64_t data_pos = 0;
			auto write_size = srcFile.Size();
			uint32_t bytes_to_read = 0;
			auto transfer_size = default_block_size;

			auto data_array = IO::makeDataArray(transfer_size);

			uint32_t bytes_read = 0;
			uint32_t bytes_written = 0;

			uint64_t src_position = 0;
			uint64_t dst_position = target_offset;

			while (data_pos < write_size)
			{
				bytes_to_read = IO::calcBlockSize(data_pos, write_size, transfer_size);
				srcFile.setPosition(src_position);
				bytes_read =  srcFile.ReadData(data_array->data(), bytes_to_read);

				targetDevice_->setPosition(dst_position);
				bytes_written = targetDevice_->WriteData(data_array->data(), bytes_read);

				data_pos += bytes_read;
				src_position += bytes_read;
				dst_position += bytes_read;

			}
			wprintf(L"Successfully write from %s.\n", sourceFilename.c_str());
			return true;
		}
	};