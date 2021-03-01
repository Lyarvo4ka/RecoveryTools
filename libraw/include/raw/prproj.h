#pragma once

#include "AbstractRaw.h"
#include "Factories.h"

namespace RAW
{
	class RawAdobePremire
		: public DefaultRaw
	{
	private:
		uint64_t file_size_ = 0;
	public:
		explicit RawAdobePremire(IODevicePtr device)
			: DefaultRaw(device)
		{
		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (file_size_ > 0)
				return appendToFile(target_file, start_offset, file_size_);
			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			uint64_t offset = start_offset + default_sector_size;
			DataArray buffer(default_block_size);

			const uint8_t four_nulls[] = { 0x00, 0x00, 0x00, 0x00 };
			const uint32_t four_nulls_size = SIZEOF_ARRAY(four_nulls);

			while(offset < this->getSize())
			{
				this->setPosition(offset);
				this->ReadData(buffer);

				for (uint32_t i = 0; i < buffer.size(); ++i)
				{
					if (memcmp(buffer.data() + i, four_nulls, four_nulls_size) == 0)
					{
						file_size_ = offset + i + 1 - start_offset;
						return true;
					}
				}
				offset += default_block_size;
			}
			return false;
		}

	};


	class RawAdobePremireFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new RawAdobePremire(device);
		}
	};
}