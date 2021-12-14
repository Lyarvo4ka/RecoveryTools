#pragma once

#include "AbstractRaw.h"
#include "Factories.h"


namespace RAW
{
#pragma pack( push, 1)
	struct WebmHeader
	{
		uint8_t header[0x30];
		uint64_t size;
	};
#pragma pack(pop)

	constexpr auto WebmHeaderSize = sizeof(WebmHeader);

	
	class RawWebm
		: public DefaultRaw
	{
	private:
		uint64_t file_size_ = 0;
	public:
		explicit RawWebm(IODevicePtr device)
			: DefaultRaw(device)
		{
		}

		uint64_t SaveRawFile(File& target_file, const uint64_t start_offset) override
		{
			if (file_size_ > 0)
				return appendToFile(target_file, start_offset, file_size_);
			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			IO::DataArray header(WebmHeaderSize);
			this->setPosition(start_offset);
			this->ReadData(header);
			WebmHeader* pWebmHeader = (WebmHeader *)header.data();
			uint64_t tmp_size = pWebmHeader->size;
			toBE64(tmp_size);

			file_size_ = tmp_size >> 8;
			if (file_size_ == 0)
				return false;
			file_size_ += 0x37;
			return true;
		}
	};
	class RawWebmFactory
		: public RawFactory
	{
	public:
		RawAlgorithm* createRawAlgorithm(IODevicePtr device) override
		{
			return new RawWebm(device);
		}
	};

}

