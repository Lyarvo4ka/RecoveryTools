#pragma once


 
#include "io/constants.h"
//#include "iofs.h"
#include "raw/StandartRaw.h"
//#include "Entropy.h"

#include <iostream>

//#include <boost\filesystem.hpp>



namespace RAW
{

	const char s_ftyp[] = "ftyp";
	const char s_moov[] = "moov";
	const char s_mdat[] = "mdat";
	const char s_wide[] = "wide";
	const char s_skip[] = "skip";
	const char s_free[] = "free";
	const char s_pnot[] = "pnot";
	const char s_prfl[] = "prfl";
	const char s_mvhd[] = "mvhd";
	const char s_clip[] = "clip";
	const char s_trak[] = "trak";
	const char s_udta[] = "udta";
	const char s_ctab[] = "ctab";
	const char s_cmov[] = "cmov";
	const char s_rmra[] = "rmra";
	const char s_uuid[] = "uuid";
	const char s_meta[] = "meta";
	const char s_PICT[] = "PICT";

	const std::string_view stco_table_name = "stco";


	const int qt_keyword_size = 4;

	using array_keywords = std::vector<const char*>;


	const array_keywords qt_array = { s_ftyp, s_moov, s_mdat, s_wide , s_free, s_skip, s_pnot, s_prfl,
									  s_mvhd, s_clip, s_trak, s_udta, s_ctab, s_cmov, s_rmra , s_uuid, s_meta , s_PICT };



#pragma pack(1)
	struct qt_block_t
	{
		uint32_t block_size;
		char block_type[qt_keyword_size];
	};
#pragma pack()

	//using ListQtBlock = std::list<qt_block_t>;
	inline ByteArray toByteArray(qt_block_t& qtBlock)
	{
		return reinterpret_cast<ByteArray>(&qtBlock);
	}
	inline ByteArray toByteArray(uint64_t& value64bit)
	{
		return reinterpret_cast<ByteArray>(&value64bit);
	}
	const uint32_t qt_block_struct_size = sizeof(qt_block_t);

	inline bool isQuickTimeKeyword(const qt_block_t& pQtBlock, const char* keyword_name)
	{
		return (memcmp(pQtBlock.block_type, keyword_name, qt_keyword_size) == 0);
	}

	inline bool verify_region(const uint64_t start, const uint64_t size)
	{
		return start < size;
	}
	inline bool isQuickTime(const qt_block_t& pQtBlock)
	{
		for (auto keyword_name : qt_array)
			if (isQuickTimeKeyword(pQtBlock, keyword_name))
				return true;

		return false;
	}

	inline bool isPresentInArrayKeywords(const array_keywords& keywords, const char* key_val)
	{
		for (auto theKeyword : keywords)
		{
			if (memcmp(theKeyword, key_val, qt_keyword_size) == 0)
				return true;
		}
		return false;
	}
	inline bool cmp_keyword(const qt_block_t& qt_block, const char* keyword_name)
	{
		return (memcmp(qt_block.block_type, keyword_name, qt_keyword_size) == 0);
	}



	const uint8_t moov_signToFind[] = { 0x6D, 0x6F, 0x6F, 0x76, 0x00, 0x00, 0x00 };
	const int moov_signToFind_size = SIZEOF_ARRAY(moov_signToFind);

	inline bool findMOOV_signature(const DataArray& data_array, uint32_t& position)
	{
		//return findTextTnBlock(data_array, s_moov, position);
		//uint32_t temp_pos = 0;
		for (uint32_t pos = 0; pos < data_array.size() - moov_signToFind_size; ++pos)
		{

			if (memcmp(data_array.data() + pos, moov_signToFind, moov_signToFind_size) == 0)
			{
				position = pos;
				return true;
			}
		}
		return false;


	}

	class QtHandle
	{
		qt_block_t qtBlock_ = qt_block_t();
		uint64_t offset_ = 0;
		uint64_t size_ = 0;
		bool valid_ = false;
	public:
		QtHandle()
		{}
		QtHandle(const uint64_t offset, const uint64_t size)
			: offset_(offset)
			, size_(size)
		{
			if (size != 0)
				setValid();
		}
		//QtHandle operator=(const QtHandle & new_handle)
		//{
		//	memcpy(&qtBlock_, &new_handle, sizeof(qt_block_t));
		//}
		//QtHandle operator=(const QtHandle && new_handle)
		//{
		//	memcpy(&qtBlock_, &new_handle, sizeof(qt_block_t));
		//}
		qt_block_t* getBlock()
		{
			return &qtBlock_;
		}
		const char* block_type() const
		{
			return qtBlock_.block_type;
		}
		void setBlock(const qt_block_t& qt_block)
		{
			memcpy(&qtBlock_, &qtBlock_, qt_block_struct_size);
		}
		void setOffset(const uint64_t offset)
		{
			offset_ = offset;
		}
		bool compareKeyword(const std::string& keyword_name)
		{
			if (keyword_name.length() != qt_keyword_size)
			{
				LOG_MESSAGE("Error keyword length is not equal to 4 bytes.");
				return false;
			}
			return (memcmp(keyword_name.c_str(), qtBlock_.block_type, qt_keyword_size) == 0);

		}
		uint64_t offset() const
		{
			return offset_;
		}
		void setSize(const uint64_t size)
		{
			size_ = size;
			if (size_ != 0)
				setValid();
		}
		uint64_t size() const
		{
			return size_;
		}
		void setValid()
		{
			valid_ = true;
		}
		void setInvalid()
		{
			valid_ = false;
		}
		bool isValid() const
		{
			return valid_;
		}
	};
	using QuickTimeList = std::list<QtHandle>;



	class QuickTimeRaw
		: public StandartRaw
	{
	private:
		QuickTimeList keywordsList_;
		uint64_t sizeToWrite_ = 0;
	public:
		explicit QuickTimeRaw(IODevicePtr device)
			: StandartRaw(device)
		{
		}
		explicit QuickTimeRaw(const path_string& fileName)
			: StandartRaw(makeFilePtr(fileName))
		{
		}

		virtual ~QuickTimeRaw()
		{
		}

		DataArray readFtypData(const QtHandle& ftyp_handle)
		{
			if (ftyp_handle.isValid())
				if (ftyp_handle.size() > qt_block_struct_size)
				{
					auto ftypdata_size = ftyp_handle.size() - qt_block_struct_size;
					DataArray ftyp_data(ftypdata_size);
					this->setPosition(ftyp_handle.offset() + qt_block_struct_size);
					this->ReadData(ftyp_data);
					return ftyp_data;
				}
			return DataArray(0);
		}


		uint64_t readQtAtom(const uint64_t start_offset, qt_block_t& qt_block)
		{
			if ((start_offset + qt_block_struct_size) > this->getSize())
				return 0;
			this->setPosition(start_offset);
			auto bytes_read = this->ReadData(toByteArray(qt_block), qt_block_struct_size);
			if (bytes_read != qt_block_struct_size)
				return 0;

			if (!isQuickTime(qt_block))
				return 0;

			if (qt_block.block_size == 0)
				return 0;

			toBE32(qt_block.block_size);

			return readQtAtomSize(qt_block.block_size, start_offset);
		}
		QtHandle readQtAtom(const uint64_t start_offset)
		{
			QtHandle atom_handle;
			atom_handle.setOffset(start_offset);
			auto atom_size = readQtAtom(start_offset, *atom_handle.getBlock());
			if (atom_size == 0)
				return QtHandle();

			atom_handle.setSize(atom_size);
			return atom_handle;
		}
		QtHandle readQtAtomAndCompareTo(const uint64_t start_offset , const std::string & keyword_name)
		{
			auto atom_handle = readQtAtom(start_offset);
			if (!atom_handle.compareKeyword(keyword_name))
				atom_handle.setInvalid();
			return atom_handle;
		}
		uint64_t readAllQtAtoms(const uint64_t start_offset, QuickTimeList& keywordsList)
		{
			uint64_t keyword_offset = start_offset;
			uint64_t write_size = 0;
			uint64_t full_size = 0;

			while (true)
			{
				auto qt_handle = readQtAtom(keyword_offset);
				if (!qt_handle.isValid())
					break;

				full_size += qt_handle.size();
				keywordsList.push_back(qt_handle);
				keyword_offset += qt_handle.size();
			}

			return full_size;
		}
		uint64_t readQtAtomSize(const qt_block_t& qt_block, uint64_t keyword_offset)
		{
			return readQtAtomSize(qt_block.block_size, keyword_offset);
		}
		uint64_t readQtAtomSize(const uint32_t block_size, uint64_t keyword_offset)
		{
			uint64_t write_size = block_size;

			if (write_size == 1)
			{
				uint64_t ext_size = 0;
				uint64_t ext_size_offset = keyword_offset + qt_block_struct_size;
				if (ext_size_offset + sizeof(uint64_t) >= this->getSize())
					return 0;

				this->setPosition(ext_size_offset);
				this->ReadData(toByteArray(ext_size), sizeof(uint64_t));

				toBE64(ext_size);
				write_size = ext_size;
			}
			return write_size;
		}

		QtHandle findQtKeyword(const uint64_t start_offset, const std::string& keyword_name)
		{
			if (keyword_name.length() != qt_keyword_size)
			{
				LOG_MESSAGE("Error keyword length is not equal to 4 bytes.");
				return QtHandle();
			}

			uint64_t keyword_pos = start_offset;
			DataArray data_array(this->getBlockSize());
			uint32_t bytesRead = 0;
			uint32_t bytesToRead = 0;

			while (keyword_pos < this->getSize())
			{
				bytesToRead = calcBlockSize(keyword_pos, this->getSize(), data_array.size());
				this->setPosition(keyword_pos);
				bytesRead = this->ReadData(data_array.data(), bytesToRead);

				for (uint32_t iSector = 0; iSector < bytesRead; ++iSector /*+= default_sector_size*/)
				{
					qt_block_t* pQt_block = reinterpret_cast<qt_block_t*>(data_array.data() + iSector);
					if (cmp_keyword(*pQt_block, keyword_name.c_str()))
					{
						auto keyword_block = readQtAtom(keyword_pos + iSector);
						if (keyword_block.isValid())
							return keyword_block;
					}
				}
				keyword_pos += bytesRead;
			}


			return QtHandle();
		}

		uint64_t SaveRawFile(File& target_file, const uint64_t start_offset) override
		{
			if (!target_file.isOpen())
			{
				wprintf(L"Target file wasn't opened.\n");
				return 0;
			}

			if (sizeToWrite_ > 0)
				return appendToFile(target_file, start_offset, sizeToWrite_);

			return 0;
		}
		bool Specify(const uint64_t start_offset) override
		{
			keywordsList_.clear();

			auto sizeKeywords = readAllQtAtoms(start_offset, keywordsList_);
			if (isPresentMainKeywords(keywordsList_))
			{
				sizeToWrite_ = sizeKeywords;
				return true;
			}
			return false;
		}

		bool isPresentMDAT(const QuickTimeList& keywordsList_)
		{
			for (auto& refQtHandle : keywordsList_)
			{
				if (memcmp(refQtHandle.block_type(), s_mdat, qt_keyword_size) == 0)
					return true;
			}
			return false;
		}
		bool isPresentMainKeywords(const QuickTimeList& keywordsList_)
		{
			bool bmdat = false;
			bool bmoov = false;

			for (auto& refQtHandle : keywordsList_)
			{
				if (memcmp(refQtHandle.block_type(), s_mdat, qt_keyword_size) == 0)
					bmdat = true;
				else if (memcmp(refQtHandle.block_type(), s_moov, qt_keyword_size) == 0)
					bmoov = true;

				if (bmdat && bmoov)
					return true;
			}

			return false;
		}

	};




	class QuickTimeRawFactory
		: public RawFactory
	{
	public:
		RawAlgorithm* createRawAlgorithm(IODevicePtr device) override
		{
			return new QuickTimeRaw(device);
		}
	};


	/*
	1. goto offset 2235 (full file size)
	2. read ftyp and moov (calculate size 1 part)
	3. full file size - size 1 part
	4. Find mdat with expexted size
	*/

	const uint32_t OffsetToFileSize = 2234;
	const uint32_t Offset_29CA = 0x29CA;
	const uint32_t offset_4764 = 4764;
	const uint32_t offset_1398 = 1398;

	class CanonStartFragment
		: public StandartRaw
	{
	public:
		explicit CanonStartFragment(IODevicePtr device)
			: StandartRaw(device)
		{

		}
		uint64_t SaveRawFile(File& target_file, const uint64_t start_offset) override
		{
			auto file_size_offset = start_offset + Offset_29CA;
			uint32_t fullFileSize = 0;
			this->setPosition(file_size_offset);
			this->ReadData((ByteArray)&fullFileSize, sizeof(uint32_t));
			//toBE32(fullFileSize);

			auto offset = start_offset;
			QuickTimeRaw qt_raw(this->getDevice());
			auto ftyp_handle = qt_raw.readQtAtom(offset);
			if (!ftyp_handle.isValid())
				return 0;

			auto moov_offset = ftyp_handle.offset() + ftyp_handle.size();
			auto moov_handle = qt_raw.readQtAtom(moov_offset);
			if (!moov_handle.isValid())
				return 0;

			auto firstPartSize = ftyp_handle.size() + moov_handle.size();

			if (fullFileSize <= firstPartSize)
				return 0;
			
			auto sectondPartSize = fullFileSize - firstPartSize;

			qt_block_t expected_block = qt_block_t();
			expected_block.block_size = 1;
			toBE32(expected_block.block_size);
			memcpy(expected_block.block_type, s_mdat, qt_keyword_size);

			auto mdatHandle = findSecondPart(expected_block, sectondPartSize);
			if (mdatHandle.isValid())
			{
				auto written_size = appendToFile(target_file, start_offset, firstPartSize);
				written_size += appendToFile(target_file, mdatHandle.offset(), mdatHandle.size());
				return written_size;
			}

			return 0;
		}

		QtHandle findSecondPart(const qt_block_t &expected_block , const uint64_t sectondPartSize)
		{
			uint64_t offset = 0;
			DataArray buffer(default_block_size);

			QuickTimeRaw qt_raw(this->getDevice());

			while (offset < this->getSize())
			{
				setPosition(offset);
				ReadData(buffer);

				for (uint32_t iSector = 0; iSector < buffer.size(); iSector += default_sector_size)
				{
					qt_block_t* pQtBlock = (qt_block_t*)(buffer.data() + iSector);
					if (memcmp(&expected_block, pQtBlock, qt_block_struct_size) == 0)
					{
						auto mdat_offset = offset + iSector;
						auto mdat_handle = qt_raw.readQtAtom(mdat_offset);
						if (mdat_handle.size() == sectondPartSize)
						{
							return mdat_handle;
						}
					}
				}

				offset += buffer.size();
			}



			return QtHandle();

		}
		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}	
	};

	class CanonStartFragmentFactory
		: public RawFactory
	{
	public:
		RawAlgorithm* createRawAlgorithm(IODevicePtr device) override
		{
			return new CanonStartFragment(device);
		}
	};


}