#pragma once
#include "raw\StandartRaw.h"

namespace RAW
{
	class PLNRaw :
		public StandartRaw
	{
	public:

		PLNRaw(IODevicePtr device)
			: StandartRaw(device)
		{
		}

		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			return StandartRaw::SaveRawFile(target_file,start_offset);
		}

		bool Specify(const uint64_t start_offset) override
		{
			DataArray sector_data(getSectorSize());
			this->setPosition(start_offset);
			auto bytes_read = this->ReadData(sector_data.data(), sector_data.size());
			if (bytes_read != getSectorSize())
				return 0;
			const uint32_t numBytes = 5;
			auto fiveBytes = makeDataArray(numBytes);
			memcpy(fiveBytes->data(), sector_data.data(), fiveBytes->size());
			SignatureArray fotersArray;
			fotersArray.emplace_back(std::move(fiveBytes));
			this->setFooters(fotersArray);
			return true;
		}
		bool Verify(const IO::path_string & file_path) override
		{
			return StandartRaw::Verify(file_path);
		}
	};

	class PLNRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new PLNRaw(device);
		}
	};

	const uint32_t PLN_BL_header_size = 32;

#pragma pack( push, 1)
	struct PLN_BL_header
	{
		uint8_t data[PLN_BL_header_size];
	};

	const char BL_text[] = "BL";
	uint32_t BL_cmp_size = 2;

	struct BL_struct
	{
		uint8_t name[2];
		uint32_t size;
	};
	constexpr uint32_t BL_struct_size = sizeof(BL_struct);
#pragma pack(pop)

	class PLN_BL
		: public DefaultRaw
	{
	public:
		PLN_BL(IODevicePtr device)
			: DefaultRaw(device)
		{
		}

		uint64_t SaveRawFile(File& target_file, const uint64_t start_offset) override
		{
			PLN_BL_header header = { 0 };
			auto offset = start_offset;
			this->setPosition(offset);
			this->ReadData(header.data, PLN_BL_header_size);
			offset += PLN_BL_header_size;

			uint64_t size = PLN_BL_header_size;

			uint32_t bl_size_chunk = 0;

			BL_struct bl = { 0 };
			//uint64_t prev_offset = 0;
			while (true)
			{
				this->setPosition(offset);
				this->ReadData((uint8_t*)&bl, BL_struct_size);

				if (memcmp(bl.name, BL_text, BL_cmp_size) != 0)
					break;

				bl_size_chunk = bl.size ;
				bl_size_chunk *= 16;
				offset += bl_size_chunk;

				size += bl_size_chunk;

			}

			return appendDataToFile(this->getDevice(), start_offset, target_file, size);
		}

		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}
		bool Verify(const IO::path_string& file_path) override
		{
			return true;
		}

	};

	class PLN_BLRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm* createRawAlgorithm(IODevicePtr device) override
		{
			return new PLN_BL(device);
		}
	};
}
