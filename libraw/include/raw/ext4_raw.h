#pragma once

#include "io/iodevice.h"
#include "AbstractRaw.h"
#include "io/finder.h"

namespace RAW
{


	const uint64_t KB = 1024;
	const uint64_t MB = KB * KB;
	const uint64_t GB = MB * KB;
	constexpr uint64_t TB = GB * KB;

	const uint16_t EXTENT_HEADER_MAGIC = (uint16_t)0xF30A;
	
	const uint16_t INODE_SIZE = 256;

	const uint16_t MIN_REQUIRE_ENTRIES = 2;

#pragma pack(push)
#pragma pack(1)

	typedef struct _ext4_extent {
		uint32_t block;		//	First file block number that this extent covers.
		uint16_t len;
		uint16_t start_hi;	//	Upper 16-bits of the block number to which this extent points.
		uint32_t start_lo;	//	Lower 32-bits of the block number to which this extent points.
		uint64_t PysicalBlock(void) { return ((uint64_t)start_lo) | (((uint64_t)start_hi) << 32); }
	} ext4_extent;

	typedef struct _ext4_extent_idx {
		uint32_t block;		//	This index node covers file blocks from 'block' onward.
		uint32_t leaf_lo;		//	Lower 32-bits of the block number of the extent node that is the next level lower in the tree. 
							//	The tree node pointed to can be either another internal node or a leaf node, described below.
		uint16_t leaf_hi;		//	Upper 16-bits of the previous field.
		uint16_t unused;
		uint64_t PysicalBlock(void) { return ((uint64_t)leaf_lo) | (((uint64_t)leaf_hi) << 32); }
	} ext4_extent_idx;


	typedef struct _ext4_extent_header {
		uint16_t magic;
		uint16_t entries;
		uint16_t max;
		uint16_t depth;
		uint32_t generation;
	} ext4_extent_header;

	typedef struct _EXTENT_BLOCK {
		ext4_extent_header header;
		union {
			ext4_extent extent[1];
			ext4_extent_idx extent_index[1];
		};
	} EXTENT_BLOCK;


	struct ext4_inode
	{
		uint8_t skip;
		EXTENT_BLOCK extent_block;
	};

	class ExtentStruct
	{
		DataArray block_;
		bool bValid_ = false;
		uint64_t block_num_ = 0;
	public:
		ExtentStruct(const uint32_t block_size)
			: block_(block_size)
		{

		}
		void copyData(ByteArray source_data)
		{
			memcpy(block_.data(), source_data, block_.size());
			bValid_ = true;
		}
		ByteArray data()
		{
			return block_.data();
		}
		uint32_t size()
		{
			return block_.size();
		}
		bool isValid()
		{
			return bValid_;
		}
		void setValid(bool bValue = true)
		{
			bValid_ = bValue;
		}
		EXTENT_BLOCK* getExtentBlock() const
		{
			return (EXTENT_BLOCK*)block_.data();
		}
		void setBlockNumber(uint64_t block_num)
		{
			block_num_ = block_num;
		}
		uint64_t getBlockNumber() const
		{
			return block_num_;
		}
	};


	class StringConverter
	{
	public:
		static std::string toHexString(uint64_t value_to_convert, const uint32_t num_nulls_before = 17)
		{
			std::string string_txt;
			if (num_nulls_before == 0)
				return string_txt;

			std::vector<char> buff(num_nulls_before, 0);

			sprintf_s(buff.data(), num_nulls_before, "%.15I64x", value_to_convert);
			string_txt = buff.data();
			return string_txt;
		}
		static std::string toString(uint64_t value_to_convert)
		{
			const uint32_t max_values = 20;
			std::vector<char> buff(max_values, 0);

			sprintf_s(buff.data(), buff.size(), "%I64x", value_to_convert);

			std::string str(buff.data());
			return str;

		}
	};

	struct ExtentHandle
	{
		uint64_t offset = 0;
		uint64_t size = 0;
		uint16_t number_entries = 0;
		uint64_t first_offset = 0;
	};

	class ListExtents
	{
		std::list<ExtentHandle> listExtents_;
		//std::list<ExtentHandle>::iterator listIterator;
	public:
		void add(uint64_t offset , uint64_t size , uint16_t num_entries , uint64_t first_offset)
		{
			ExtentHandle extHandle{ offset , size , num_entries ,first_offset };
			listExtents_.emplace_back(extHandle);
		}
		void remove(uint64_t offset)
		{
			listExtents_.remove_if([&](const ExtentHandle & val) {return val.offset == offset; });
		}
		bool isEmpty() const
		{
			return listExtents_.empty();
		}
		ExtentHandle findByOffset(uint64_t offset)
		{
			auto findIter = std::find_if(begin(listExtents_), end(listExtents_), [&](const ExtentHandle& extent_handle) {return extent_handle.offset == offset; });
			if (findIter != listExtents_.end())
				return *findIter;

			return ExtentHandle{ 0,0 };
		}
		std::list<ExtentHandle> findBySize(uint64_t size)
		{
			std::list<ExtentHandle> found_extents;
			for (auto extent_handle : listExtents_)
			{
				if (extent_handle.size == size)
					found_extents.emplace_back(extent_handle);
			}
			return found_extents;
		}
		std::list<ExtentHandle> findByFirstOffset(uint64_t size)
		{
			std::list<ExtentHandle> found_extents;
			for (auto extent_handle : listExtents_)
			{
				if (extent_handle.first_offset == size)
					found_extents.emplace_back(extent_handle);
			}
			return found_extents;
		}




	};

	inline ExtentHandle findWithMaxEntries(const std::list<ExtentHandle>& list_handles)
	{
		auto find_iter = std::max_element(begin(list_handles), end(list_handles),
			[](const ExtentHandle& left, const ExtentHandle& right)
			{
				return left.number_entries < right.number_entries;
			});
		if (find_iter != list_handles.end())
			return *find_iter;

		return ExtentHandle{ 0, 0, 0 };
	}



	class ext4_raw
		: public SpecialAlgorithm
	{
		IODevicePtr device_;
		uint64_t volume_offset_ = 0;
		uint32_t block_size_ = 4096;
		uint32_t blocksPerGroup_ = 32768 * 4096;
		uint32_t max_extents_in_block_ = 0;
		uint64_t value_to_cmp_ = 0;
		IO::path_string offsetsFileName_ = L"extents_offsets.txt";
		ListExtents listExtents_;
		uint16_t depth_ = 0;
		IO::File txtFile_;
	public:
		ext4_raw(IODevicePtr device)
			: device_(device)
			, txtFile_(L"")
		{
			max_extents_in_block_ = (block_size_ - sizeof(ext4_extent_header)) / sizeof(ext4_extent);
		}
		uint32_t getBlockSize() const;
		void setVolumeOffset(uint64_t new_offset);
		void setDepth(uint16_t depth);
		uint64_t Execute(const uint64_t inode_offset, const path_string target_folder) override;

		DataArray readInode(const uint64_t inode_offset);
		void readExtent(const uint64_t block_num, DataArray& buffer);
		void setOffsetsFileName(const path_string& offsetsFileName);

		// if (first_offset == size_to_cmp)
		bool firstExtentOffsetEqualTo(ByteArray data, uint32_t size);


		void readExtentsListFromFile(const path_string& fileName);
		ListExtents readListExtents(std::list<uint64_t>& listOffsets);

		void searchExtends(uint64_t block_start);
		void findExtentsWithDepth(uint16_t depth, const path_string & fileName , uint64_t start_offset = 0 );
		
		std::list<uint64_t> readOffsetsFromFile(const path_string& fileName);


		uint64_t getOffsetFromPhysicalBlock(const uint64_t physical_block);
		
		uint64_t CalculateExtentSizeWith_0Depth(const DataArray& block);

		uint64_t CalculateExtentSize(const DataArray& block);
		uint64_t CalculateExtentSize(const uint64_t block_num);

		
		bool isValidExtent(const EXTENT_BLOCK& extent_block);
		bool isValidExtentWithDepth(const EXTENT_BLOCK& extent_block, uint16_t depth = 0);
		bool compareIsValidExtentWithDepth(ByteArray data, uint32_t size);

		void toResize(DataArray& data_array, uint32_t new_size);

		uint32_t determineSize(const uint16_t len);

		bool readFirstSectorFromExtent(const uint64_t extent_offset);
		uint64_t saveToFile(const uint64_t block_num, File& target_file);

	//	uint64_t SaveRawFile(uint64_t iNode_offset)
	};

#pragma pack(pop)
}