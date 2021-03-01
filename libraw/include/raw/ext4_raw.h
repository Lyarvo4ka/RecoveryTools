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
	public:
		ext4_raw(IODevicePtr device)
			: device_(device)
		{
			max_extents_in_block_ = (block_size_ - sizeof(ext4_extent_header)) / sizeof(ext4_extent);
		}
		void setVolumeOffset(uint64_t new_offset)
		{
			volume_offset_ = new_offset;
		}
		uint64_t Execute(const uint64_t inode_offset, const path_string target_folder) override
		{
			if (!device_->isOpen())
				return 0;

			//DataArray inode(256);
			//device_->setPosition(inode_offset);
			//device_->ReadData(inode.data(), inode.size());
			//EXTENT_BLOCK *pExtBlock = (EXTENT_BLOCK*)(inode.data());

			//auto ext_offset = pExtBlock->extent_index->PysicalBlock();
			
			//DataArray next_inode(4096);
			//device_->setPosition(inode_offset);
			//device_->ReadData(next_inode.data(), next_inode.size());
			//EXTENT_BLOCK *pNext = (EXTENT_BLOCK*)(next_inode.data());

			//int k = 1;
			//k = 2;
			auto ext_offset = inode_offset / 4096;
			uint64_t inode_block = ext_offset/* / 4096*/;
			File target_file(target_folder);
			target_file.OpenWrite();


			saveToFile(inode_block , target_file);
			//auto inode = read_inode(inode_offset);
			//ext4_inode * pInode = (ext4_inode*)inode.data();
			//if (pInode->extent_block.header.magic == EXTENT_HEADER_MAGIC)
			//{
			//}
			return 0;
		}

		DataArray readInode(const uint64_t inode_offset)
		{
			DataArray inode(INODE_SIZE);
			device_->setPosition(inode_offset);
			device_->ReadData(inode.data(), inode.size());
			return inode;
		}
		void setOffsetsFileName(const path_string& offsetsFileName)
		{
			offsetsFileName_ = offsetsFileName;
		}

		// if (first_offset == size_to_cmp)
		bool firstExtentOffsetEqualTo(ByteArray data, uint32_t size)
		{
			EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)(data);
			if (isValidExtentWithNullDepth(*extent_block))
			{
				uint64_t first_offset = (uint64_t)extent_block->extent[0].block * block_size_;
				if (first_offset == value_to_cmp_)
					return true;
			}
			return false;
		}


		void readExtentsListFromFile(const path_string & fileName)
		{
			auto listAllffset = readOffsetsFromFile(fileName);
			listExtents_ = readListExtents(listAllffset);
		}
		ListExtents readListExtents(std::list<uint64_t>& listOffsets)
		{
			ListExtents listExtents;
			DataArray block(block_size_);
			for (auto offset : listOffsets)
			{
				auto block_num = offset / block_size_;
				readExtent(block_num , block);
				auto extent_size = CalculateExtentSize(block);
				EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)(block.data());
				uint64_t first_offset = 0;
				if (extent_block->header.entries > 0)
					first_offset =((uint64_t) extent_block->extent[0].block )* block_size_;

				listExtents.add(offset, extent_size , extent_block->header.entries , first_offset);
			}
			return listExtents;
		}

		void searchExtends(uint64_t block_start)
		{
			uint64_t current_offset = block_start * block_size_;
			uint64_t current_block = block_start;
			ExtentHandle extent_handle = listExtents_.findByOffset(current_offset);
			auto current_size = extent_handle.size;

			uint64_t target_offset = 0;

			while (true)
			{
				std::cout << "dst offset " << target_offset << " size " << current_size << " src_block " << current_block << std::endl;

				listExtents_.remove(current_offset);
				target_offset += current_size;
				if (current_size == 0)
					break;

				if (target_offset == 0x10000000000)
				{
					int k = 1;
					k = 2;
				}

				auto listWithSameSize = listExtents_.findByFirstOffset(target_offset);
				if (listWithSameSize.empty())
					break;

				extent_handle = findWithMaxEntries(listWithSameSize);
				if (extent_handle.number_entries == 0)
					break;

				current_offset = extent_handle.offset;
				current_block = current_offset / block_size_;
				
				extent_handle = listExtents_.findByOffset(current_offset);
				current_size = extent_handle.size;
			}

		}
		void findExtentsWithDepth(uint16_t depth, const path_string & fileName )
		{
			File extentsOffset_txt(fileName);
			extentsOffset_txt.OpenCreate();

			uint64_t offset = 0;

			IO::DataFinder data_finder(device_);
			data_finder.setSearchSize(block_size_);
			data_finder.compareFunctionPtr_ = std::bind(&ext4_raw::compareIsValidExtentWithNullDepth, this, std::placeholders::_1, std::placeholders::_2);
			

			while (true)
			{
				if (!data_finder.findFromCurrentToEnd(offset))
					break;
				offset = data_finder.getFoundPosition();
				auto str_text = StringConverter::toString(offset) + "\n";
				//str_text += std::endl;

				extentsOffset_txt.WriteText(str_text);

				offset += block_size_;
			}
		}
		std::list<uint64_t> readOffsetsFromFile(const path_string & fileName)
		{
			std::list<uint64_t> offsetList;
			File extentsOffset_txt(fileName);
			extentsOffset_txt.OpenRead();

			if (extentsOffset_txt.Size() == 0)
				return offsetList;

			IO::DataArray buff((uint32_t)extentsOffset_txt.Size());
			extentsOffset_txt.ReadData(buff);

			const uint8_t NEW_LINE = 0x0A;

			uint64_t iPrev = 0;
			uint64_t iCur = 0;

			while (iCur < buff.size())
			{
				if (buff[iCur] == NEW_LINE)
				{
					uint64_t iSize = iCur - iPrev;
					//IO::DataArray tmp(iSize + 1);
					//ZeroMemory(tmp.data(), tmp.size());
					//memcpy(tmp.data(), buff.data() + iPrev, iSize);
					std::string tmp_str((char*)(buff.data() + iPrev), iSize);
					//

					uint64_t offset = 0;
					std::stringstream ss;
					ss << std::hex << tmp_str;
					ss >> offset;

					offsetList.push_back(offset);


					iPrev = iCur + 1;
				}

				++iCur;
			} 


			return offsetList;

		}
		void readExtent(const uint64_t block_num, DataArray& buffer)
		{

			uint64_t extent_offset = volume_offset_ + block_num * block_size_;
			device_->setPosition(extent_offset);
			device_->ReadData(buffer.data(), buffer.size());
		}

		uint64_t getOffsetFromPhysicalBlock(const uint64_t physical_block)
		{
			return (volume_offset_ + physical_block * block_size_);
		}
		
		uint64_t CalculateExtentSize(const DataArray & block)
		{
			EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)block.data();

			if (!isValidExtentWithNullDepth(*extent_block))
				return 0;

			uint64_t extent_size = 0;
			if (extent_block->header.entries == 1)
				extent_size = determineSize(extent_block->extent[0].len);
			if (extent_block->header.entries > MIN_REQUIRE_ENTRIES)
			{
				auto first_offset = getOffsetFromPhysicalBlock(extent_block->extent[0].block);

				auto last_extent = extent_block->extent[extent_block->header.entries - 1];

				auto last_offset = getOffsetFromPhysicalBlock(last_extent.block);
				auto last_size = last_extent.len;

				extent_size = last_offset + determineSize(last_size) - first_offset;

			}
			return extent_size;
		}

		uint64_t CalculateExtentSize(const uint64_t block_num)
		{
			DataArray extent(block_size_);
			readExtent(block_num, extent);
			return CalculateExtentSize(extent);

		}
		
		bool isValidExtent(EXTENT_BLOCK& extent_block)
		{
			if ((extent_block.header.magic != EXTENT_HEADER_MAGIC) ||
				(extent_block.header.max != max_extents_in_block_) ||
				(extent_block.header.entries > max_extents_in_block_)) {
				return false;
			}
			return true;
		}
		bool isValidExtentWithNullDepth(EXTENT_BLOCK& extent_block)
		{
			if (isValidExtent(extent_block))
				if (extent_block.header.depth == 0)
					return true;
			return false;
		}
		bool compareIsValidExtentWithNullDepth(ByteArray data, uint32_t size)
		{
			EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)(data);
			return isValidExtentWithNullDepth(*extent_block);
		}
		void toResize(DataArray & data_array , uint32_t new_size)
		{
			if (data_array.size() < new_size) {
				data_array.resize(new_size);
			}

		}

		uint32_t determineSize(const uint16_t len)
		{
			return (len <= 0x8000)
				? (len * block_size_)
				: ((len - 0x8000) * block_size_);
		}

		uint64_t saveToFile(const uint64_t block_num, File &target_file)
		{
			if (!target_file.isOpen())
				return 0;

			DataArray buffer(block_size_);
			EXTENT_BLOCK *extent_block = (EXTENT_BLOCK *)buffer.data();

			uint64_t extent_offset = volume_offset_ + block_num * block_size_;
			device_->setPosition(extent_offset);
			device_->ReadData(buffer.data(),buffer.size());
			
			if (!isValidExtent(*extent_block))
				return 0;

			//std::vector<BYTE> data_buff;
			DataArray data_array(default_block_size);
			uint64_t offset = 0;
			uint32_t size = 0;
			if (extent_block->header.depth == 0) {
				for (int i = 0; i < extent_block->header.entries; i++) {
					offset = volume_offset_ + extent_block->extent[i].PysicalBlock() * block_size_;

					size = determineSize(extent_block->extent[i].len);

					toResize(data_array, size);

					if (extent_block->extent[i].len <= 0x8000) 
					{
						device_->setPosition(offset);
						device_->ReadData(data_array.data(), size);
					}
					else {
						memset(data_array.data(), 0x00, size);
					}

					//extent_block->extent[i].block += 0x10000000;
					uint64_t target_offset = (uint64_t)extent_block->extent[i].block * block_size_;
					target_file.setPosition(target_offset);
					target_file.WriteData(data_array.data(), size);
				}
			}
			else {
				//File text_file(LR"(e:\48264\offsets.txt)");
				//text_file.OpenCreate();

				for (int i = 0; i < extent_block->header.entries; i++) {
					saveToFile(extent_block->extent_index[i].PysicalBlock(), target_file);
					//auto text = std::to_string(extent_block->extent_index[i].PysicalBlock());
					//text_file.WriteText(text + "\n");
					//int x = 0;
				}
				//text_file.Close();
				int k = 1;
				k = 2;

			}
			return 0;
		}

	//	uint64_t SaveRawFile(uint64_t iNode_offset)
	};

#pragma pack(pop)
}