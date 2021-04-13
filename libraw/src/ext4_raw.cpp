#include "raw/ext4_raw.h"

namespace RAW
{
 		void ext4_raw::findExtentsWithDepth(uint16_t depth, const path_string & fileName , uint64_t start_offset )
		{
			File extentsOffset_txt(fileName);
			extentsOffset_txt.OpenCreate();

			uint64_t offset = start_offset;

			this->setDepth(depth);

			IO::DataFinder data_finder(device_);
			data_finder.setSearchSize(block_size_);
			data_finder.compareFunctionPtr_ = std::bind(&ext4_raw::compareIsValidExtentWithDepth, this, std::placeholders::_1, std::placeholders::_2);
			

			while (true)
			{
				if (!data_finder.findFromCurrentToEnd(offset))
					break;
				offset = data_finder.getFoundPosition();
				std::cout << offset << std::endl;
				auto str_text = StringConverter::toString(offset) + "\n";
				//str_text += std::endl;

				extentsOffset_txt.WriteText(str_text);

				offset += block_size_;
			}
		}
		uint64_t ext4_raw::CalculateExtentSizeWith_0Depth(const DataArray& block)
		{
			EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)block.data();

			if (!isValidExtentWithDepth(*extent_block, 0))
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
		uint64_t ext4_raw::CalculateExtentSize(const DataArray& block)
		{
			uint64_t size = 0;
			EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)block.data();

			if (!isValidExtent(*extent_block))
				return 0;

			if (extent_block->header.depth == 1)
			{
				IO::DataArray nextExtent(block.size());
				for (int i = 0; i < extent_block->header.entries; i++) 
				{
					auto newBlockNumber = extent_block->extent_index[i].PysicalBlock();
					readExtent(newBlockNumber , nextExtent);
					size += CalculateExtentSizeWith_0Depth(nextExtent);

				}

			}
			else
				size = CalculateExtentSizeWith_0Depth(block);
			return size;
		}
		uint64_t ext4_raw::CalculateExtentSize(const uint64_t block_num)
		{
			DataArray extent(block_size_);
			readExtent(block_num, extent);
			return CalculateExtentSize(extent);

		}

		uint64_t ext4_raw::saveToFile(const uint64_t block_num, File& target_file)
		{
			if (!target_file.isOpen())
				return 0;

			DataArray buffer(block_size_);
			EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)buffer.data();

			uint64_t extent_offset = volume_offset_ + block_num * block_size_;
			device_->setPosition(extent_offset);
			device_->ReadData(buffer.data(), buffer.size());

			if (!isValidExtent(*extent_block))
				return 0;

			//auto text = StringConverter::toString(block_num * block_size_);
			//txtFile_.WriteText(text + "\n");

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


					if (memcmp (data_array.data() , Signatures::bad_sector_marker , Signatures::bad_sector_marker_size) == 0)
						return 0;

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
					//txtFile_.WriteText(text + "\n");
					//int x = 0;
				}
				//text_file.Close();
				int k = 1;
				k = 2;

			}
			return 0;
		}


   
}