#pragma once

#include "IO\IODevice.h"

#include <boost/algorithm/string.hpp>



namespace RAW
{
	constexpr std::string_view ObjectHeaderStr = { 0x31 , 0x43 , 0x44 , 0x42 , 0x4F , 0x42 , 0x56 , 0x38 };//"1CDBOBV8";
	// const char[] ObjectHeader = {0x31 , 0x43 , 0x44 , 0x42 , 0x4F , 0x42 , 0x56 , 0x38};
	// const uin32_t ObjectHeaderSize = SIZEOF_ARRAY(ObjectHeader);

	//namespace 
	struct Header_1CD
	{
		char sig[8]; // ��������� �1CDBMSV8� 
		char ver1; 
		char ver2; 
		char ver3; 
		char ver4;
		unsigned int length; 
		int unknown; 
	};

	const uint32_t max_object_blocks = 1018;
	struct Object{

	char sig[8]; // ��������� �1CDBOBV8�

	int length; // ����� ����������� �������

	int version1;

	int version2;

	unsigned int version;

	unsigned int blocks[max_object_blocks];

	};

	struct IndexObject
	{
		int numblocks;

		unsigned int datablocks[1023];
	};

#pragma pack( 1 )
	struct RootObject
	{
		char lang[32];

		int numblocks;

	};
#pragma pack()

	class File1C
	{
	private:
		uint32_t block_size_;
		IO::File file_;
	public:
		File1C(const IO::path_string fileName , const uint32_t block_size)
			: block_size_(block_size)
			, file_(fileName)
		{

		}

		IO::DataArray ReadBlock(const uint32_t block_number)
		{
			IO::DataArray data_array(block_size_);
			file_.setPosition(block_number * block_size_);
			auto bytes_read = file_.ReadData(data_array);
			if (bytes_read == 0)
				return IO::DataArray(0);
			return data_array;

		}

		IO::DataArray ReadBlocks(const IndexObject & index_object)
		{
			IO::DataArray block_data(index_object.numblocks * block_size_);
			for (auto index = 0; index < index_object.numblocks; ++index)
			{
				// Root Object
				auto pos = index_object.datablocks[index] * block_size_;
				file_.setPosition(pos);
				auto bytes_read = file_.ReadData(block_data.data() + block_size_ * index, block_size_);

			}
			return block_data;

		}

		void Read()
		{
			
			if (!file_.Open(IO::OpenMode::OpenRead))
				return;

			const uint32_t header_page_number = 0;
			const uint32_t free_page_number = 1;
			const uint32_t root_page_number = 2;

			uint32_t page_number = 0;
			// Read header;
			auto header_data = ReadBlock(header_page_number);
			auto free_data = ReadBlock(free_page_number);

		
			auto root_data1 = ReadBlock(root_page_number);
			Object * pObject = (Object*)root_data1.data();
			for (uint32_t i = 0; i < max_object_blocks; ++i)
			{
				auto block_number = pObject->blocks[i];
				if (block_number == 0)
					break;

				// IndexObject
				auto index_data = ReadBlock(block_number);
				IndexObject * pIndexBlock = (IndexObject *)index_data.data();

				auto root_data = ReadBlocks(*pIndexBlock);
				RootObject * pRootObject = (RootObject *)root_data.data();
				int * pTableIndexes = (int*) (root_data.data() + sizeof(RootObject));
				for (auto iTable = 0; iTable < pRootObject->numblocks; ++iTable)
				{
					printf("%d\r\n",pTableIndexes[iTable]);
				}
					int k = 1;
					k = 2;
				

			}



		}

	};

	enum FILE_TABLE_NAME { DESCR  =0 ,INDEX , BLOB };





}
