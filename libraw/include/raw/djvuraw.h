#pragma once

#include "raw/AbstractRaw.h"
#include "raw/factories.h"


namespace RAW
{
#pragma pack( push, 1)
	struct DjvuHeader
	{
		uint8_t header[8];
		uint32_t size;
	};
#pragma pack(pop)

	uint32_t djvu_header_size = sizeof(DjvuHeader);

	class DjvuRAW
		: public DefaultRaw
	{
		uint32_t file_size_ = 0;
	public:
		DjvuRAW(IO::IODevicePtr device)
			: DefaultRaw(device)
		{

		}

		uint64_t SaveRawFile(File& target_file, const uint64_t start_offset)   override
		{
			if (file_size_ > 0)
				return appendToFile(target_file, start_offset, file_size_);
			return 0;
		}
		bool Specify(const uint64_t start_offset)
		{
			DjvuHeader djvu_header;
			this->setPosition(start_offset);
			this->ReadData((IO::ByteArray) & djvu_header, djvu_header_size);

			toBE32(djvu_header.size);
			file_size_ = djvu_header.size + djvu_header_size;

			return true;
		}



	};

	class DjvuRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm* createRawAlgorithm(IO::IODevicePtr device) override
		{
			return new DjvuRAW(device);
		}
	};

	

}