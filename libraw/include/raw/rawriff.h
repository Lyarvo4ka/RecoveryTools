#pragma once

#include "AbstractRaw.h"
#include "Factories.h"

namespace RAW
{
	#pragma pack (push) 
	#pragma pack (1)
	struct riff_header_struct
	{
		uint8_t riff_name[4];
		uint32_t size;

	};
	#pragma pack (pop)

	const uint32_t riff_header_struct_size = sizeof(riff_header_struct);

	const uint32_t ONE_MB = 1 * 1024 * 1024;
	class RawRIFF
		: public DefaultRaw
	{
	private:
		uint32_t minFileSize_ = ONE_MB;
	public:
		RawRIFF(IODevicePtr device)
			: DefaultRaw(device)
		{
		}
		~RawRIFF()
		{}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (!target_file.isOpen())
			{
				wprintf(L"Target file wasn't opened.\n");
				return 0;
			}

			riff_header_struct riff_struct = { 0 };
			readRiffStruct(riff_struct, start_offset);
			wprintf(L"Riff write size: %d\n", riff_struct.size);
			if (riff_struct.size > 0)
			{
				riff_struct.size += riff_header_struct_size*2;

				return appendToFile(target_file, start_offset, riff_struct.size);
			}
			else
				wprintf(L"Size RIFF is 0.\n");
			return 0;
		}

		bool Specify(const uint64_t start_offset) override
		{
			// Read header_sector if size is more than 1Mb return true
			riff_header_struct riff_struct = { 0 };
			readRiffStruct(riff_struct, start_offset);

			if (riff_struct.size == 0)
				return false;
			if (riff_struct.size >= 0xFFFFFFFF - riff_header_struct_size)
				return false;

			//if (riff_struct.size >= getMinFileSize())
				return true;


			return false;
		}
		void setMinFileSize(const uint32_t minFileSize)
		{
			minFileSize_ = minFileSize;
		}
		uint32_t getMinFileSize() const
		{
			return minFileSize_;
		}
		void readRiffStruct(riff_header_struct & riff_header, const uint64_t start_offset)
		{
			setPosition(start_offset);
			ReadData((ByteArray)&riff_header, riff_header_struct_size);
		}
	};

	class RawFIFFFactory
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new RawRIFF(device);
		}
	};


	class ZOOMHandyRecorderRaw
		: public RawRIFF
	{
	private:
		uint32_t cluster_size_ = 32768;
	public:
		ZOOMHandyRecorderRaw(IODevicePtr device)
			: RawRIFF(device)
		{

		}
		uint64_t SaveRawFile(File & target_file, const uint64_t start_offset) override
		{
			if (!target_file.isOpen())
			{
				wprintf(L"Target file wasn't opened.\n");
				return 0;
			}

			uint64_t stereo_offset = start_offset;
			uint64_t mono_offset = start_offset + cluster_size_;

			// read stereo
			riff_header_struct stereo_struct = riff_header_struct();
			readRiffStruct(stereo_struct, stereo_offset);
			wprintf(L"STEREO write size: %d\n", stereo_struct.size);

			// read mono
			riff_header_struct mono_struct = riff_header_struct();
			readRiffStruct(mono_struct, mono_offset);
			wprintf(L"MONO write size: %d\n", mono_struct.size);


			auto stereoFileName = target_file.getFileName();
			fs::path mono_path(target_file.getFileName());
			auto folder = mono_path.parent_path();
			auto file = mono_path.stem();
			auto monoFileName = addBackSlash(folder.wstring()) + file.wstring() + L"_Tr1" + L".wav";
			auto monoFile = makeFilePtr(monoFileName);
			if (!monoFile->Open(OpenMode::Create))
			{
				wprintf(L"Error to create MONO file: %s\n", monoFileName.c_str());
				return 0;
			}
			uint32_t bytes_written = 0;

			bytes_written = appendToFile(target_file, stereo_offset, cluster_size_);
			uint32_t stereo_size = bytes_written;

			bytes_written = appendToFile(*monoFile, mono_offset, cluster_size_);
			uint32_t mono_size = bytes_written;
			
			stereo_offset = mono_offset + cluster_size_;

			while (stereo_size < stereo_struct.size)
			{
				if (stereo_offset >= this->getSize())
				{
					wprintf(L"Error end of source.");
					return 0;
				}

				stereo_size += appendToFile(target_file, stereo_offset, cluster_size_* 2);
				mono_offset = stereo_offset + cluster_size_ * 2;

				mono_size += appendToFile(*monoFile, mono_offset, cluster_size_);
				stereo_offset = mono_offset +cluster_size_;
			}

			
			target_file.setSize(stereo_struct.size + riff_header_struct_size);
			if (mono_struct.size < stereo_struct.size)
				monoFile->setSize(mono_struct.size + riff_header_struct_size);
			monoFile->Close();
			return stereo_size;

		}
		bool Specify(const uint64_t start_offset) override
		{
			return true;
		}

	};

	class RawZOOMHandyRecorder
		: public RawFactory
	{
	public:
		RawAlgorithm * createRawAlgorithm(IODevicePtr device) override
		{
			return new ZOOMHandyRecorderRaw(device);
		}
	};
}

