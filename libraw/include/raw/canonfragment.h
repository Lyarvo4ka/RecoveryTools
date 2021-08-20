#pragma once

#include "quicktime.h"

namespace RAW
{
	class QTFragmentRaw
		: public QuickTimeRaw
	{
	public:
		QTFragmentRaw(IODevicePtr device)
			: QuickTimeRaw(device)
		{
		}

		uint64_t SaveRawFile(File& target_file, const uint64_t start_offset) override
		{
			setBlockSize(131072);
			qt_block_t mdat_block = qt_block_t();
			// read mdat size
			auto mdat_atom = readQtAtom(start_offset);
			if (!mdat_atom.isValid())
				return 0;

			uint64_t find_pos = start_offset + mdat_atom.size();
			find_pos = alingToSector(find_pos);
			find_pos += this->getSectorSize();

			auto ftyp_atom = findQtKeyword(find_pos, s_ftyp);
			if (!ftyp_atom.isValid())
				return 0;

			uint64_t moov_offset = ftyp_atom.offset() + ftyp_atom.size();
			auto moov_atom = readQtAtom(moov_offset);
			if (moov_atom.isValid())
				if (moov_atom.compareKeyword(s_moov))
				{
					uint64_t writeSize = ftyp_atom.size() + moov_atom.size();

					uint64_t free_offset = moov_atom.offset() + moov_atom.size();
					auto free_atom = readQtAtom(free_offset);
					if (free_atom.isValid())
						if (free_atom.compareKeyword(s_free))
							writeSize += free_atom.size();

					uint64_t target_size = appendToFile(target_file, ftyp_atom.offset(), writeSize);
					target_size += appendToFile(target_file, start_offset, mdat_atom.size());
					return target_size;
				}
			return 0;
		}



		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}
	};


	class QTFragmentRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm* createRawAlgorithm(IODevicePtr device) override
		{
			return new QTFragmentRaw(device);
		}
	};

}
