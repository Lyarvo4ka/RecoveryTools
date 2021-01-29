#pragma once

#include "file.h"

#include <boost/algorithm/string.hpp>



namespace IO
{
	//constexpr std::string_view ObjectHeaderStr = { 0x31 , 0x43 , 0x44 , 0x42 , 0x4F , 0x42 , 0x56 , 0x38 };//"1CDBOBV8";
	// const char[] ObjectHeader = {0x31 , 0x43 , 0x44 , 0x42 , 0x4F , 0x42 , 0x56 , 0x38};
	// const uin32_t ObjectHeaderSize = SIZEOF_ARRAY(ObjectHeader);

#pragma pack(push,1)

	struct ObjectHeader {
		char signature[8]; // "1CDBOBV8"
		uint32_t object_size;
		uint32_t flag1;
		uint32_t flag2;
		uint32_t flag3;
		uint32_t alloc_pages[1];
	};

	struct AllocationPage {
		uint32_t pages_count;
		uint32_t pages[1];
	};

	struct BlobHeader {
		uint32_t next_block;
		uint16_t data_size;
		uint8_t data[250];
	};

#pragma pack(pop)



	bool IsValidDbObject(File & db_file,  ObjectHeader& obj_header, uint32_t page_size, uint32_t max_block_num)
	{
		size_t total_blocks = 0;
		size_t blk_idx = 0;
		uint32_t first_page = 0;
		DataArray buff(page_size);
		if (obj_header.object_size && obj_header.alloc_pages[0]) {
			while (obj_header.alloc_pages[blk_idx]) {

				db_file.setPosition(page_size * obj_header.alloc_pages[blk_idx]);
				db_file.ReadData(buff);

				AllocationPage* table = (AllocationPage*)buff.data();

				if ((table->pages_count < 1) || (table->pages_count > sizeof(AllocationPage::pages))) {
					return false;
				}

				int curr_blocks = 0;
				for (int i = 0; i < sizeof(AllocationPage::pages); i++) {
					if ((table->pages[i] == 0) || (table->pages[i] > max_block_num)) {
						break;
					}
					if (blk_idx == 0 && i == 0) {
						first_page = table->pages[i];
					}
					curr_blocks++;
					total_blocks++;
				}

				if (curr_blocks != table->pages_count) {
					return false;
				}

				blk_idx++;
			}

			if (((obj_header.object_size + (page_size - 1)) / page_size) != total_blocks) {
				return false;
			}

			db_file.setPosition(first_page * page_size);
			db_file.ReadData(buff);


			uint32_t* obj_sign = (uint32_t*)buff.data();
			if (*obj_sign == 0x0022007B) {
				return true;
			}
		}
		return false;
	}

	void RestoreRootObject()
	{
		constexpr uint32_t PAGE_SIZE = 4 * 1024;

		File db(LR"(y:\48945\1Cv8.1CD )");
		db.OpenRead();

		File root_object(LR"(y:\48945\root_object.dat)");
		root_object.OpenCreate();

			const size_t page_size = PAGE_SIZE;
			const uint32_t max_page = db.Size() / page_size;

			uint32_t page = 0;
			size_t valid_objects = 0;
			DataArray buffer(page_size);

			page = 3;
			uint64_t offset = page_size * page;
			db.setPosition(offset);

			while (offset < db.Size())
			{
				db.ReadData(buffer);
				if (std::memcmp(buffer.data(), "1CDBOBV8", 8) == 0) {

					ObjectHeader* obj_header = (ObjectHeader*)buffer.data();
					if (IsValidDbObject(db, *obj_header, page_size, max_page)) {
						root_object.WriteData((ByteArray)&page, sizeof(page));
						valid_objects++;
					}
				}
				page++;
				auto position = page_size * page;

				db.setPosition(position);
				offset = position;
				int k = 1;
				k = 2;
			}
			int k = 1;
			k = 2;


	}

	//class File1C
	//{
	//private:
	//	uint32_t block_size_;
	//	File file_;
	//public:
	//	File1C(const path_string fileName , const uint32_t block_size)
	//		: block_size_(block_size)
	//		, file_(fileName)
	//	{

	//	}

	//	DataArray ReadBlock(const uint32_t block_number)
	//	{
	//		DataArray data_array(block_size_);
	//		file_.setPosition(block_number * block_size_);
	//		auto bytes_read = file_.ReadData(data_array);
	//		if (bytes_read == 0)
	//			return DataArray(0);
	//		return data_array;

	//	}

	//	DataArray ReadBlocks(const IndexObject & index_object)
	//	{
	//		DataArray block_data(index_object.numblocks * block_size_);
	//		for (auto index = 0; index < index_object.numblocks; ++index)
	//		{
	//			// Root Object
	//			auto pos = index_object.datablocks[index] * block_size_;
	//			file_.setPosition(pos);
	//			auto bytes_read = file_.ReadData(block_data.data() + block_size_ * index, block_size_);

	//		}
	//		return block_data;

	//	}

	//	void Read()
	//	{
	//		
	//		if (!file_.Open(OpenMode::OpenRead))
	//			return;

	//		const uint32_t header_page_number = 0;
	//		const uint32_t free_page_number = 1;
	//		const uint32_t root_page_number = 2;

	//		uint32_t page_number = 0;
	//		// Read header;
	//		auto header_data = ReadBlock(header_page_number);
	//		auto free_data = ReadBlock(free_page_number);

	//	
	//		auto root_data1 = ReadBlock(root_page_number);
	//		Object * pObject = (Object*)root_data1.data();
	//		for (uint32_t i = 0; i < max_object_blocks; ++i)
	//		{
	//			auto block_number = pObject->blocks[i];
	//			if (block_number == 0)
	//				break;

	//			// IndexObject
	//			auto index_data = ReadBlock(block_number);
	//			IndexObject * pIndexBlock = (IndexObject *)index_data.data();

	//			auto root_data = ReadBlocks(*pIndexBlock);
	//			RootObject * pRootObject = (RootObject *)root_data.data();
	//			int * pTableIndexes = (int*) (root_data.data() + sizeof(RootObject));
	//			for (auto iTable = 0; iTable < pRootObject->numblocks; ++iTable)
	//			{
	//				printf("%d\r\n",pTableIndexes[iTable]);
	//			}
	//				int k = 1;
	//				k = 2;
	//			

	//		}



	//	}

	//};

	//enum FILE_TABLE_NAME { DESCR  =0 ,INDEX , BLOB };
	//struct Table1COffsets
	//{
	//	Table1COffsets()
	//	{
	//		threeTables.resize(3, 0);
	//	}
	//	std::vector<uint64_t> threeTables;
	//	bool isValid() const
	//	{
	//		uint32_t counter = 0;
	//		for (const auto & table : threeTables)
	//		{
	//			if (table == 0)
	//				++counter;
	//		}

	//		return (counter != 3);
	//	
	//	}
	//};

	//class Table1CHandle
	//{
	//private:
	//	path_string filePath_;
	//	path_string tableName_;
	//	std::list<uint64_t> tablesOffset_;
	//	uint64_t page_size_ = 4096;
	//public:
	//	Table1CHandle(const path_string & pathToFile)
	//		: filePath_(pathToFile)
	//	{

	//	}
	//	std::vector<path_string> splitStrings(path_string str, const path_string delimiters)
	//	{
	//		const path_string endLineDelimeter = L"\n";
	//		std::vector<path_string> strs;
	//		boost::split(strs, str, boost::is_any_of(endLineDelimeter)/*,boost::algorithm::token_compress_on*/);
	//		std::vector<path_string> no_str_bracked;
	//		std::vector<IO::path_string> new_vec_string;
	//		for (const auto & tmp_str : strs)
	//		{
	//			if (!tmp_str.empty())
	//				if (tmp_str.compare(L"\"") != 0)
	//				{
	//					new_vec_string.push_back(tmp_str);
	//				}
	//		}
	//		return new_vec_string;
	//	}
	//	Table1COffsets getFilesOffset(std::vector<path_string> & strings_1C)
	//	{
	//		Table1COffsets offsetsTables;
	//		// First Entry must 1C table name
	//		if (strings_1C.empty())
	//			return Table1COffsets();

	//		tableName_ = strings_1C.at(0);

	//		const path_string FILES_NAME = L"\"Files\"";
	//		auto iter = std::find(begin(strings_1C), end(strings_1C), FILES_NAME);
	//		if (iter == strings_1C.end())
	//		{
	//			LOG_MESSAGE(L"Not found keyword (\"Files\")");
	//			return Table1COffsets();
	//		}

	//		for (int i = 0; i < 3; ++i)
	//		{
	//			++iter;
	//			if (iter == strings_1C.end())
	//				break;

	//			auto tmp_str = *iter;
	//			try
	//			{
	//				auto tmp_val = boost::lexical_cast<uint64_t>(tmp_str);
	//				offsetsTables.threeTables.at(i) = tmp_val;
	//			}
	//			catch (const boost::bad_lexical_cast& ex)
	//			{
	//				LOG_MESSAGE("Could not convert string to uint64_t");
	//				break;
	//			}
	//			
	//		}
	//		return offsetsTables;
	//	}
	//	bool readTable(Table1COffsets offsets)
	//	{

	//	}
	//};



}
