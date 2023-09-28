#include "raw/ext4_raw.h"

namespace RAW
{
	uint32_t ext4_raw::getBlockSize() const
	{
		return block_size_;
	}
	void ext4_raw::setVolumeOffset(uint64_t new_offset)
	{
		volume_offset_ = new_offset;
	}
	void ext4_raw::setDepth(uint16_t depth)
	{
		depth_ = depth;
	}
	uint64_t ext4_raw::Execute(const uint64_t inode_offset, const path_string target_folder)
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


		saveToFile(inode_block, target_file);
		//auto inode = read_inode(inode_offset);
		//ext4_inode * pInode = (ext4_inode*)inode.data();
		//if (pInode->extent_block.header.magic == EXTENT_HEADER_MAGIC)
		//{
		//}
		return 0;
	}
	DataArray ext4_raw::readInode(const uint64_t inode_offset)
	{
		DataArray inode(INODE_SIZE);
		device_->setPosition(inode_offset);
		device_->ReadData(inode.data(), inode.size());
		return inode;
	}
	void ext4_raw::readExtent(const uint64_t block_num, DataArray& buffer)
	{

		uint64_t extent_offset = volume_offset_ + block_num * block_size_;
		device_->setPosition(extent_offset);
		device_->ReadData(buffer.data(), buffer.size());
	}
	void ext4_raw::setOffsetsFileName(const path_string& offsetsFileName)
	{
		txtFile_.Close();
		offsetsFileName_ = offsetsFileName;
		txtFile_.setFileName(offsetsFileName_);
		txtFile_.OpenCreate();
	}

	// if (first_offset == size_to_cmp)

	bool ext4_raw::firstExtentOffsetEqualTo(ByteArray data, uint32_t size)
	{
		EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)(data);
		if (isValidExtentWithDepth(*extent_block))
		{
			uint64_t first_offset = (uint64_t)extent_block->extent[0].block * block_size_;
			if (first_offset == value_to_cmp_)
				return true;
		}
		return false;
	}
	void ext4_raw::readExtentsListFromFile(const path_string& fileName)
	{
		auto listAllffset = readOffsetsFromFile(fileName);
		listExtents_ = readListExtents(listAllffset);
	}
	ListExtents ext4_raw::readListExtents(std::list<uint64_t>& listOffsets)
	{
		ListExtents listExtents;
		DataArray block(block_size_);
		for (auto offset : listOffsets)
		{
			auto block_num = offset / block_size_;
			readExtent(block_num, block);
			auto extent_size = CalculateExtentSize(block);
			EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)(block.data());
			uint64_t first_offset = 0;
			if (extent_block->header.entries > 0)
				first_offset = ((uint64_t)extent_block->extent[0].block) * block_size_;

			listExtents.add(offset, extent_size, extent_block->header.entries, first_offset);
		}
		return listExtents;
	}
	void ext4_raw::searchExtends(uint64_t block_start)
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

	void ext4_raw::searchExtents(uint64_t start_offset, const IO::path_string& target_folder, uint16_t max_depth)
	{
		IO::DataFinder data_finder(device_);
		data_finder.setSearchSize(block_size_);

		ExtentSearchValues extSearchValues(block_size_);
		extSearchValues.setMaxDepth(max_depth);

		data_finder.compareFunctionPtr_ = std::bind(&ExtentSearchValues::cmpExtentIfLessMaxDepth, &extSearchValues, std::placeholders::_1, std::placeholders::_2);

		ExtentsDepthByLevel extDepth(target_folder);

		uint64_t offset = start_offset;

		while (true)
		{
			if (!data_finder.findFromCurrentToEnd(offset))
				break;
			offset = data_finder.getFoundPosition();
			std::cout << offset << " depth = " << extSearchValues.getDepth() << std::endl;
			if (extSearchValues.getDepth() > 0)
			{
				int k = 1;
				k = 2;
			}
			extDepth.addOffset(offset, extSearchValues.getDepth());



			offset += block_size_;
		}

	}
	

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
	std::list<uint64_t> ext4_raw::readOffsetsFromFile(const path_string& fileName)
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
	uint64_t ext4_raw::getOffsetFromPhysicalBlock(const uint64_t physical_block)
	{
		return (volume_offset_ + physical_block * block_size_);
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

	bool ext4_raw::isValidExtent(const EXTENT_BLOCK& extent_block)
	{
		if ((extent_block.header.magic != EXTENT_HEADER_MAGIC) ||
			(extent_block.header.max != max_extents_in_block_) ||
			(extent_block.header.entries > max_extents_in_block_)) {
			return false;
		}
		return true;
	}

	bool ext4_raw::isValidExtentWithDepth(const EXTENT_BLOCK& extent_block, uint16_t depth)
	{
		if (isValidExtent(extent_block))
			if (extent_block.header.depth == depth)
				return true;
		return false;
	}

	bool ext4_raw::compareIsValidExtentWithDepth(ByteArray data, uint32_t size)
	{
		EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)(data);
		return isValidExtentWithDepth(*extent_block, depth_);
	}

	void ext4_raw::toResize(DataArray& data_array, uint32_t new_size)
	{
		if (data_array.size() < new_size) {
			data_array.resize(new_size);
		}

	}

	uint32_t ext4_raw::determineSize(const uint16_t len)
	{
		return (len <= 0x8000)
			? (len * block_size_)
			: ((len - 0x8000) * block_size_);
	}

	bool ext4_raw::readFirstSectorFromExtent(const uint64_t extent_offset)
	{
		DataArray buffer(block_size_);
		EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)buffer.data();

		device_->setPosition(extent_offset);
		device_->ReadData(buffer.data(), buffer.size());

		DataArray data_array(default_block_size);
		uint64_t offset = 0;
		uint32_t size = 0;
		if (extent_block->header.depth == 0)
			if (extent_block->header.entries > 0)
			{
				offset = volume_offset_ + extent_block->extent[0].PysicalBlock() * block_size_;
				DataArray sector(default_sector_size);
				device_->setPosition(offset);
				device_->ReadData(sector.data(), sector.size());

				const uint32_t ZipHeaderSize = 4;
				const uint8_t ZipHeader[ZipHeaderSize] = { 0x50, 0x4B, 0x03, 0x04 };

				if (memcmp(sector.data(), ZipHeader, ZipHeaderSize) == 0)
					return true;

			}
		return false;

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
				offset = 0 + extent_block->extent[i].PysicalBlock() * block_size_;



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


				if (memcmp(data_array.data(), Signatures::bad_sector_marker, Signatures::bad_sector_marker_size) == 0)
					return 0;

				//extent_block->extent[i].block += 0x10000000;

				uint64_t target_offset = (uint64_t)extent_block->extent[i].block * block_size_;
				//if (i == 0)
				//	if (target_offset > 0)
				//	{
				//		int k = 1;
				//		k = 1;
				//		return 0;
				//	}
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


   
	ExtentSearchValues::ExtentSearchValues(uint32_t block_size)
		: block_size_(block_size)
	{
		max_extents_in_block_ = calcMaxExtentsInBlock(block_size_);
	}

	void ExtentSearchValues::setMaxDepth(uint16_t maxDepth)
	{
		maxDepth_ = maxDepth;
	}

	uint16_t ExtentSearchValues::getDepth() const
	{
		return depth_;
	}

	bool ExtentSearchValues::cmpExtentIfLessMaxDepth(ByteArray data, uint32_t size)
	{
		EXTENT_BLOCK* extent_block = (EXTENT_BLOCK*)(data);
		//const EXTENT_BLOCK& extent_block = (const EXTENT_BLOCK&)data;
		if (extent_block->header.depth <= maxDepth_)
			if (isValidExtent(*extent_block, max_extents_in_block_))
			{
				if (depth_ > 0)
				{
					int k = 1;
					k = 2;
				}
				depth_ = extent_block->header.depth;
				return true;
			}

		return false;
	}

	ExtentsDepthByLevel::ExtentsDepthByLevel(IO::path_string targetFolder)
		:targetFolder_(targetFolder)
	{

	}

	void ExtentsDepthByLevel::createFile(const uint16_t depth)
	{
		IO::path_string filepath = IO::addBackSlash(targetFolder_) + std::to_wstring(depth) + L".depth";
		arrayFiles_.emplace_back(IO::File(filepath));
	}

	void ExtentsDepthByLevel::createFiles(const uint16_t depth)
	{
		std::size_t new_size = depth + 1;
		if (arrayFiles_.size() < new_size)
		{
			for (std::size_t i = arrayFiles_.size(); i < new_size; ++i)
			{
				createFile(i);
			}
		}
	}

	void ExtentsDepthByLevel::addOffset(uint64_t offset, uint16_t depth)
	{
		createFiles(depth);
		auto str_text = StringConverter::toString(offset) + "\n";
		if (!arrayFiles_.at(depth).isOpen())
			arrayFiles_.at(depth).OpenCreate();

		arrayFiles_.at(depth).WriteText(str_text);
		int k = 1; 
		k = 2;

		// if file 
	}

}