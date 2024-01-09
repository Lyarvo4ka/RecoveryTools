#ifndef ABSTRACT_RAW_H
#define ABSTRACT_RAW_H


//#include "iofunctions.h"
#include "io/iodevice.h"
#include <memory>

#include "io/utility.h"
#include "io/dataarray.h"
#include "filestruct.h"

namespace RAW
{
	using namespace IO;





	class HeaderBase
	{
	public:
		using Ptr = std::shared_ptr<HeaderBase>;
	private:
		std::list<FileStruct::Ptr> listHeaders_;
		std::list<FileStruct::Ptr>::const_iterator iter_;
	public:
		void addFileFormat(FileStruct::Ptr new_file_format)
		{
			listHeaders_.emplace_back(std::move(new_file_format));
		}
		FileStruct * find(const ByteArray data , uint32_t size)
		{
			for (auto & theFileStruct : listHeaders_)
			{
				if (theFileStruct->compareWithAllHeaders(data, size))
					return theFileStruct.get();
			}
			return nullptr;
		}
		FileStruct* findByAlgorithmName(const std::string algorithmName)
		{
			for (auto& file_struct : listHeaders_)
			{
				if (file_struct->getAlgorithmName().compare(algorithmName) == 0)
					return file_struct.get();
			}
			return nullptr;
		}
	};

	class RawAlgorithm;

	class SignatureFinder
	{
		IODevicePtr device_;
		uint32_t block_size_ = default_block_size;
		uint32_t sector_size_ = default_sector_size;
		HeaderBase::Ptr header_base_;
	public:
		SignatureFinder(IODevicePtr device , HeaderBase::Ptr header_base)
			: device_(device)
			, header_base_(header_base)
		{

		}
		~SignatureFinder()
		{

		}
		void setBlockSize(const uint32_t block_size)
		{
			this->block_size_ = block_size;
		}
		FileStruct * findHeader(const uint64_t start_offset, uint64_t & header_pos)
		{
			header_pos = 0;
			if ( !device_->isOpen())
			if (!device_->Open(OpenMode::OpenRead))
			{
				wprintf_s(L"Error open device\n");
				return nullptr;
			}

			uint64_t file_size = device_->Size();
			uint64_t offset = start_offset;
			auto buffer = makeDataArray(block_size_);
			uint32_t bytes_read = 0;
			uint32_t result_offset = 0;
			uint32_t bytesToRead = 0;


			while (offset < file_size)
			{
				device_->setPosition(offset);
				bytesToRead = IO::calcBlockSize(offset, file_size, block_size_);
				bytes_read = device_->ReadData(buffer->data(), bytesToRead);
				if (bytes_read == 0)
				{
					printf("Error read drive\r\n");
					break;
				}
				if (auto header_ptr = cmpHeader(buffer, bytes_read, result_offset))
				{
					header_pos = offset;
					header_pos += result_offset;
					return header_ptr;
				}

				offset += bytes_read;
			}
			return nullptr;
		}

		FileStruct * cmpHeader(const DataArray::Ptr & buffer, const uint32_t size, uint32_t & header_pos)
		{
			for (header_pos = 0; header_pos < size; header_pos += sector_size_)
			{
				if (auto header = header_base_->find(buffer->data() + header_pos, buffer->size()))
					return header;
			}
			return nullptr;
		}


		RawAlgorithm * createRawAlgorithm(FileStruct::Ptr headerPtr)
		{
			
			return nullptr;
		}

	};

	class RawAlgorithm
	{
	public:
		virtual uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) = 0;
		virtual bool Specify(const uint64_t start_offset) = 0;
		virtual bool Verify(const IO::path_string & file_path) 
		{
			return true;
		}
	};

	class FileAnalyzer
	{
	public:
		virtual void Analyze(const IO::path_string & file_name) = 0;
	};

	class BlockDevice
	{
		IODevicePtr device_;
		uint32_t block_size_ = default_block_size;
		uint32_t sector_size_ = default_sector_size;
	public:
		IODevice * getDevice()
		{
			return device_.get();
		}

	};

	class DefaultRaw : public RawAlgorithm
	{
	private:
		IODevicePtr device_;
		uint32_t block_size_ = default_block_size;
		uint32_t sector_size_ = default_sector_size;
	public:
		DefaultRaw(IODevicePtr device)
			: device_(device)
		{

		}
		IODevicePtr getDevice()
		{
			return device_;
		}
		void setBlockSize(const uint32_t block_size)
		{
			this->block_size_ = block_size;
		}
		uint32_t getBlockSize() const
		{
			return block_size_;
		}
		void setSectorSize(const uint32_t sector_size)
		{
			this->sector_size_ = sector_size;
		}
		uint32_t getSectorSize() const
		{
			return sector_size_;
		}
		uint32_t ReadData(ByteArray data, uint32_t size)
		{
			return device_->ReadData(data, size);
		}
		uint32_t ReadData(DataArray & data_array)
		{
			return device_->ReadData(data_array.data(), data_array.size());
		}
		void setPosition(uint64_t offset)
		{
			device_->setPosition(offset);
		}
		uint64_t getSize() const
		{
			return device_->Size();
		}
		uint64_t appendToFile(File & write_file, const uint64_t source_offset, const uint64_t write_size)
		{
			if (source_offset >= getSize())
			{
				printf("Error append to file. Source offset is more than size.\r\n");
				return 0;
			}

			uint64_t to_write = write_size;
			if (source_offset + write_size > getSize())	// ?????
				to_write = getSize() - write_size;

			if (to_write == 0)
			{
				printf("Error append to file. Write size is 0.\r\n");
			}

			auto target_offset = write_file.Size();
			uint32_t bytes_read = 0;
			uint32_t bytes_written = 0;
			uint64_t cur_pos = 0;
			uint64_t read_pos = source_offset;
			uint32_t bytes_to_write = 0;

			auto buffer = makeDataArray(getBlockSize());
			while (cur_pos < to_write)
			{
				bytes_to_write = calcBlockSize(cur_pos, write_size, getBlockSize());

				setPosition(read_pos);
				bytes_read = ReadData(buffer->data(), bytes_to_write);
				if (bytes_read == 0)
					return 0;

				read_pos += bytes_read;

				write_file.setPosition(target_offset);
				bytes_written = write_file.WriteData(buffer->data(), bytes_read);
				if (bytes_written == 0)
					return 0;

				target_offset += bytes_written;
				cur_pos += bytes_written;
			}

			return cur_pos;
		}


	};

	class SpecialAlgorithm
	{
	public:
		virtual uint64_t Execute(const uint64_t start_offset, const path_string target_folder) = 0;
	};


};

//
//
//class AbstractRaw
//{
//public:
//	AbstractRaw( const std::string & file_name  )
//		: hSource_( INVALID_HANDLE_VALUE )
//		, bReady_( false )
//	{
//		bReady_ = IO::open_read( hSource_ , file_name );
//	}
//	AbstractRaw( const DWORD drive_number  )
//		: hSource_( INVALID_HANDLE_VALUE )
//		, bReady_( false )
//	{
//		std::string drive_path( drivePathFromNumber( drive_number ) );
//		bReady_ = IO::open_read( hSource_ , drive_path );
//	}
//	virtual ~AbstractRaw()
//	{
//		close();
//	}
//	bool isReady() const
//	{
//		return bReady_;
//	}
//	void close()
//	{
//		if ( hSource_ != INVALID_HANDLE_VALUE )
//		{
//			CloseHandle( hSource_ );
//			hSource_ = INVALID_HANDLE_VALUE;
//			bReady_ = false;
//		}
//	}
//	HANDLE * getHandle() 
//	{
//		return &hSource_;
//	}
//	virtual void execute() = 0;
//private:
//	HANDLE hSource_;
//	bool bReady_;
//};

#endif
