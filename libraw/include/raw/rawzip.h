#pragma once

#include "StandartRaw.h"

namespace RAW
{
	class ZipRAW
		: public StandartRaw
	{
	public:
		ZipRAW(IO::IODevicePtr device)
			: StandartRaw(device)
		{

		}

		uint64_t SaveRawFile(File& target_file, const uint64_t start_offset)   override
		{
			return StandartRaw::SaveRawFile(target_file, start_offset);
		}
		bool Specify(const uint64_t start_offset)
		{
			return true;
		}



	};
}

