#ifndef DBF_H
#define DBF_H

#include <windows.h>
#include "io/constants.h"
#include "io/iodevice.h"

//#include "StandartRaw.h"


namespace IO
{
#pragma pack( push, 1)
	struct dbf_header
	{
		uint8_t valid_base;
		uint8_t yymmdd[3];
		uint32_t numRecords;
		uint16_t header_size;
		uint16_t record_size;

	};
#pragma pack(pop)
	const std::size_t dbf_header_size = sizeof(dbf_header);

	inline bool hasNulls(IO::ByteArray data, const uint32_t size)
	{
		for (uint32_t i = 0; i < size; ++i)
			if (data[i] == 0)
				return true;

		return false;
	}

	inline void fixDBF(const IO::path_string& file_path)
	{
		auto dbf_file = IO::makeFilePtr(file_path);
		dbf_file->OpenRead();
		auto file_size = dbf_file->Size();

		dbf_header dbf_data = { 0 };
		auto bytes_read = dbf_file->ReadData((IO::ByteArray) & dbf_data, dbf_header_size);

		if (dbf_data.header_size == 0)
		{
			wprintf(L"Header size is 0.");
			return;
		}
		if (dbf_data.record_size == 0)
		{
			wprintf(L"Record size is 0.");
			return;
		}

		auto data_buffer = IO::makeDataArray(dbf_data.record_size);
		uint32_t offset = dbf_data.header_size;
		auto tmp_path = file_path + L".tmp";
		auto tmp_file = IO::makeFilePtr(tmp_path);
		if (!tmp_file->Open(IO::OpenMode::Create))
		{
			wprintf(L"Error open file %s.", file_path.c_str());
			return;
		}

		uint32_t write_offset = 0;
		auto header_data = IO::makeDataArray(dbf_data.header_size);
		dbf_file->setPosition(0);
		bytes_read = dbf_file->ReadData(header_data->data(), header_data->size());
		if (bytes_read != dbf_data.header_size)
		{
			wprintf(L"Error Read header.");
			return;
		}
		tmp_file->WriteData(header_data->data(), header_data->size());
		write_offset += dbf_data.header_size;

		auto dbf_file_size = dbf_file->Size();
		auto src_numRecords = dbf_file_size - dbf_data.header_size - 1 ;
		src_numRecords /= dbf_data.record_size ;

		for (uint32_t i = 0; i < src_numRecords; ++i)
		{
			if ((offset + data_buffer->size()) >= dbf_file_size)
				break;
			dbf_file->setPosition(offset);
			bytes_read = dbf_file->ReadData(data_buffer->data(), data_buffer->size());
			if (bytes_read != dbf_data.record_size)
			{
				wprintf(L"Error Read record.");
				return;
			}


			if (!hasNulls(data_buffer->data(), data_buffer->size()))
			{
				tmp_file->setPosition(write_offset);
				tmp_file->WriteData(data_buffer->data(), data_buffer->size());
				write_offset += data_buffer->size();
			}
			else
			{
				wprintf(L"Has nulls.");
			}
			offset += data_buffer->size();
		}

		uint32_t tmp_size = (uint32_t)tmp_file->Size();
		uint32_t numRecords = 0;
		numRecords = tmp_size - dbf_data.header_size - 1;
		numRecords /= dbf_data.record_size;
		++numRecords;


		auto new_size = dbf_data.header_size + numRecords * dbf_data.record_size + 1;
		tmp_file->setSize(new_size);

		tmp_file->setPosition(tmp_file->Size() - 1);
		uint8_t lastByte = 0;
		tmp_file->ReadData((IO::ByteArray) & lastByte, 1);
		if (lastByte != 0x1a)
		{
			lastByte = 0x1a;
			tmp_file->setPosition(tmp_file->Size() - 1);
			tmp_file->WriteData((IO::ByteArray) & lastByte, 1);
		}

		dbf_data.numRecords = numRecords;
		tmp_file->setPosition(0);
		tmp_file->WriteData((IO::ByteArray) & dbf_data, dbf_header_size);




	}
}
//
#endif
