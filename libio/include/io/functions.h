#pragma once

#include "io/finder.h"


namespace IO
{
	uint64_t appendDataToFile(IODevicePtr source_file , const uint64_t source_offset, File& write_file, const uint64_t write_size , uint32_t block_size = default_block_size)
	{
		if (source_offset >= source_file->Size())
		{
			printf("Error append to file. Source offset is more than size.\r\n");
			return 0;
		}

		uint64_t to_write = write_size;
		if (source_offset + write_size > source_file->Size())	// ?????
			to_write = source_file->Size() - write_size;

		if (to_write == 0)
		{
			printf("Error append to file. Write size is 0.\r\n");
		}

		auto target_offset = write_file.Size();
		uint32_t bytes_read = 0;
		uint32_t bytes_written = 0;
		uint64_t cur_pos = 0;
		uint64_t read_pos = source_offset;
		uint32_t bytes_to_write = 0;

		auto buffer = makeDataArray(block_size);
		while (cur_pos < to_write)
		{
			bytes_to_write = calcBlockSize(cur_pos, write_size, block_size);

			source_file->setPosition(read_pos);
			bytes_read = source_file->ReadData(buffer->data(), bytes_to_write);

			read_pos += bytes_read;

			write_file.setPosition(target_offset);
			bytes_written = write_file.WriteData(buffer->data(), bytes_read);

			target_offset += bytes_written;
			cur_pos += bytes_written;
		}

		return cur_pos;
	}

	inline void calcNullsForFile(const path_string& file_name, uint32_t block_size)
	{
		IO::File src_file(file_name);
		src_file.OpenRead();

		IO::path_string entropy_name = file_name + L".nulls";


		IO::File entrory_file(entropy_name);
		entrory_file.OpenCreate();

		IO::DataArray data_array(block_size);

		uint32_t bytesRead = 0;
		DWORD byteWritten = 0;
		uint32_t number_nulls = 0;
		bool bResult = false;
		DWORD cluster_number = 0;



		while (true)
		{
			try {
				bResult = true;
				bytesRead = src_file.ReadData(data_array);
			}
			catch (Error::IOErrorException ex)
			{
				std::cout << "Cougth exception" << ex.what() << std::endl;
				bResult = false;
			}
			if (!bResult)
				break;

			number_nulls = 0;
			for (uint32_t i = 0; i < bytesRead; ++i)
			{
				if (data_array.data()[i] == 0)
					++number_nulls;
			}
			std::string write_string(boost::lexical_cast<std::string>(number_nulls));
			write_string.append("\r\n");

			entrory_file.WriteData((IO::ByteArray)write_string.data(), (DWORD)write_string.size());
			++cluster_number;

		}
	}
	inline void calcNullsForFolder(const path_string& folder, uint32_t block_size)
	{
		Finder finder;
		finder.FindFiles(folder);
		auto files = finder.getFiles();
		for (auto theFile : files)
		{
			calcNullsForFile(theFile, block_size);
		}
	}
}
