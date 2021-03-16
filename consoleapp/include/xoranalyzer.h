#pragma once

#include <vector>

#include "io/utility.h"
#include "io/IODevice.h"

#pragma warning(disable:4251)
namespace IO
{
	const int BYTE_SIZE = 256;

	uint64_t NumBytesForBlock(DWORD block_size)
	{
		return (uint64_t)(sizeof(WORD) * BYTE_SIZE) * block_size;
	}

	int chunksPrerBlock(uint64_t block_size)
	{
		int chunks = (int)(block_size / BS::GB);
		if (chunks == 0)
			++chunks;
		return chunks;
	}
	class ByteCount
	{
		WORD bytes_[BYTE_SIZE];
	public:
		ByteCount(void)
			//:bytes_(BYTE_SIZE,0x00)
		{
			//bytes_ = new WORD[BYTE_SIZE];
			memset(bytes_, 0x00, BYTE_SIZE*sizeof(WORD));
			int k = 1;
			k = 1;
		}
		~ByteCount(void)
		{
			//delete bytes_;
		}
		void add(BYTE _byte)
		{
			bytes_[_byte]++;
		}
		BYTE getMax()
		{
			BYTE popularByte = 0;
			WORD dwMax = bytes_[0];
			for (auto i = 1; i < BYTE_SIZE; ++i)
			{
				auto val_byte = bytes_[i];
				if (val_byte > dwMax)
				{
					popularByte = i;
					dwMax = bytes_[i];
				}
			}
			return popularByte;
		}

	};

	//class NumberOfBytesArray
	//{
	//	std::vector<ByteCount> byteCountArray_;
	//	public:
	//		ByteCounts(uint32_t size)
	//		{
	//			byteCountArray_.resize(size);
	//		}
	//		void add(uint32_t offset , uint8_t  byte_value)
	//		{
	//			byteCountArray_.at(offset).add(byte_value);
	//		}

	//};

	//class XorAnalyzer
	//{
	//	uint64_t xorSize_;
	//	File dumpFile_;
	//	File xorFile_;
	//	std::unique_ptr<NumberOfBytesArray> numberOfBytes = nullptr;
	//	//File::Ptr target_file_;

	//public:
	//	XorAnalyzer(const path_string& dump_file)
	//		:dumpFile_(dump_file)
	//	{
	//	}
	//	XorAnalyzer(const File & dumpFile , const File & xorFile , uint64_t xorSize)
	//		: dumpFile_(dumpFile)
	//		, xorFile_(xorFile)
	//		, xorSize_(xorSize)
	//	{}
	//	~XorAnalyzer(void)
	//	{
	//		dumpFile_.Close();
	//		xorFile_.Close();
	//	}
	//	uint64_t getXorSize() const
	//	{
	//		return xorSize_;
	//	}
	//	DataArray generateBlock(uint32_t block_size)
	//	{
	//		DataArray block(block_size);
	//		uint32_t fill_value = 0x11111111;
	//		const uint32_t value_size = sizeof(uint32_t);
	//		const uint32_t page_size = 16 * 1024;

	//		for (auto i = 0; i < block.size(); i += value_size)
	//		{
	//			auto val_ptr = (uint32_t*)(block.data() + i);
	//			*val_ptr = fill_value;
	//			if (i > 0)
	//				if (i % page_size)
	//					++fill_value;
	//		}



	//		return block;
	//	}
	//	void analizeXor(const path_string& xor_filename , uint32_t block_size , uint32_t xor_size)
	//	{
	//		File xor_file(xor_filename);
	//		xor_file.OpenRead();

	//		File fixed_xor_file(xor_filename + L".fixed");
	//		fixed_xor_file.OpenCreate();

	//		DataArray block(block_size);
	//		uint64_t offset = 0;

	//		while (offset < xor_file.Size())
	//		{
	//			xor_file.setPosition(offset);
	//			xor_file.ReadData(block);

	//			auto fixed_xor = getPopulaValueBlock(block, xor_size);

	//			for (auto i = 0; i < block.size(); i += xor_size)
	//				memcpy(block.data() + i, fixed_xor.data(), fixed_xor.size());

	//			fixed_xor_file.WriteData(block.data(), block.size());
	//			offset += block_size;
	//		}
	//	}

	//	DataArray getPopulaValueBlock(const DataArray &block, uint32_t xor_size)
	//	{
	//		DataArray resBlock(xor_size);
	//		ByteCount* pByteCounts = new ByteCount[xor_size];

	//		//uint32_t num_sectors = block.size() / xor_size;

	//		for (uint32_t i = 0; i < block.size(); ++i)
	//		{
	//			auto iPos = i % xor_size;
	//			auto pData = block[i];
	//			pByteCounts[iPos].add(pData);
	//		}

	//		for (uint32_t i = 0; i < xor_size; ++i)
	//			resBlock[i] = pByteCounts[i].getMax();


	//		delete[]pByteCounts;

	//		return resBlock;
	//	}

	//	void Analize()
	//	{
	//		dumpFile_.OpenRead();

	//		if (xor_size <= 0)
	//		{
	//			printf("Error block size must be more than 0.\r\n");
	//			return;
	//		}




	//		ULONGLONG needMemory = NumBytesForBlock(xor_size);

	//		int chunks = chunksPrerBlock(needMemory);
	//		DWORD chunk_size = xor_size / chunks;

	//		DWORD buffer_size = 0;
	//		DWORD xor_offset = 0;
	//		//DWORD bytesRead = 0;
	//		DWORD bytesWritten = 0;
	//		uint64_t read_offset = 0;

	//		BYTE* xor_data = new BYTE[xor_size];

	//		for (auto nChunk = 0; nChunk < chunks; ++nChunk)
	//		{
	//			printf("Start analyzing #%d chunk of %dn\r\n", nChunk, chunks);
	//			xor_offset = chunk_size * nChunk;
	//			if (nChunk == chunks - 1)
	//				buffer_size = xor_size - xor_offset;
	//			else
	//				buffer_size = getChunckBufferSize(chunk_size, nChunk, xor_size);
	//			
	//			//auto buff_size = IO::calcBlockSize(xor_offset,xor_size, chunk_size);

	//			read_offset = xor_offset;

	//			//numberOfBytes = std::make_unique<NumberOfBytesArray>(buffer_size);

	//			ByteCount* pByteCounts = new ByteCount[buffer_size];
	//			DataArray buffer(buffer_size);


	//			ReadAllBytesAndAdd(read_offset, bufferSize, pByteCounts);

	//			for (DWORD nByte = 0; nByte < buffer_size; ++nByte)
	//				xor_data[(DWORD)nByte + xor_offset] = pByteCounts[nByte].getMax();

	//			ByteArray pWriteData = xor_data + xor_offset;

	//			target_file.WriteData(pWriteData, buffer_size);

	//			delete[] pByteCounts;
	//		}

	//		delete[] xor_data;

	//		dumpFile_.Close();
	//	}

	//	void ReadAllBytesAndAdd(uint64_t read_offset ,const uint32_t bufferSize , ByteCount*pByteCounts)
	//	{
	//		DataArray buffer(bufferSize);
	//			while (read_offset < File_.Size())
	//			{
	//				dumpFile_.setPosition(read_offset);
	//				dumpFile_.ReadData(buffer);


	//				if (IO::isNot00orFF(buffer.data(), buffer.size()))
	//				{
	//					for (DWORD nByte = 0; nByte < buffer.size(); ++nByte)
	//						pByteCounts[nByte].add(buffer[nByte]);
	//				}

	//				read_offset += xorSize_;
	//			}

	//	}

	//	//void getAllMaxValues()

	//	uint32_t getChunckBufferSize(uint32_t chunck_size, int nChunck, uint32_t xor_size)
	//	{
	//		uint32_t buffer_size = 0;
	//		uint32_t xor_offset = chunck_size * nChunck;
	//		if ((xor_size - xor_offset) < chunck_size)
	//			buffer_size = xor_size - xor_offset;
	//		else
	//			buffer_size = chunck_size;
	//		return buffer_size;
	//	}


	//};



}