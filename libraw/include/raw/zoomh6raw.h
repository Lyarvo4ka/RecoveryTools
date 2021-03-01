#pragma once
#include "AbstractRaw.h"
#include "RawRIFF.h"

//#include <boost\filesystem.hpp>
#include <filesystem>

namespace RAW
{

/*
	"1. Читаем клестер. Файл проекта *.hprj".
	"2. Дальше должны бить заголовки wav файлов, оперделяем сколько заголовков сколько и у нас файлов.".
	"3. Сохраняем файлы первый файл имеет 2 кластера, остальные 1 кластер."
*/
	const char zoom_h6[] = { 0x5A , 0x4F , 0x4F , 0x4D , 0x20 , 0x48 , 0x36 , 0x20 };
	const uint32_t zoom_h6_size = SIZEOF_ARRAY(zoom_h6);

	const char riff_header[] = { 0x52 , 0x49 , 0x46 , 0x46 };
	const uint32_t riff_header_size = SIZEOF_ARRAY(riff_header);

	struct ZoomH6Header
	{
		char header_text[22];
		char folder_name[11];
	};

	namespace fs = std::filesystem;//boost::filesystem;
	class RiffFile
	{
		FilePtr file_;
		bool bReady_ = false;
		uint32_t size_;
		uint32_t currentSize_ = 0;
	public:
		using Ptr = std::shared_ptr<RiffFile>;
		RiffFile(FilePtr file, const uint32_t file_size)
			:file_(file)
			, size_(file_size + riff_header_struct_size)
		{

		}
		bool isReady() const
		{
			return bReady_;
		}

		uint32_t WriteBlock(const DataArray & data_array)
		{
			uint32_t bytesToWrite = data_array.size();
			if (currentSize_ + data_array.size() > size_)
			{
				bytesToWrite = size_ - currentSize_;
			}
			auto bytesWritten = file_->WriteData(data_array.data(), bytesToWrite);
			if (bytesWritten == 0)
				return 0;
			currentSize_ += bytesWritten;
			if (currentSize_ == size_)
				bReady_ = true;
			return bytesWritten;
		}
	};
	RiffFile::Ptr makeRiffFile(FilePtr filePtr, const uint32_t file_size)
	{
		return std::make_shared<RiffFile>(filePtr, file_size);
	}

	const uint32_t MAXZOOMFILES = 4;
	class ZoomFiles
	{
		std::vector<RiffFile::Ptr> files_;
		path_string folder_;
		path_string extension_ = L".wav";
		uint32_t maxZoomFiles_ = MAXZOOMFILES;
	public:
		ZoomFiles(const path_string & target_folder)
			: folder_(target_folder)
		{
		}
		void add_file(FilePtr file, const uint32_t file_size)
		{
			files_.emplace_back(makeRiffFile(file, file_size));
		}

		FilePtr createFile(const DataArray & data_array, const path_string fileName, const uint32_t fileNumber)
		{
			uint32_t bytesWritten = 0;

			riff_header_struct * pRiffHeader = (riff_header_struct *)data_array.data();
			if (memcmp(pRiffHeader->riff_name, riff_header, riff_header_size) != 0)
				return nullptr;

			auto number_name = std::to_wstring(fileNumber);
			fs::path filePath(folder_);
			filePath += fileName + L"_" + number_name + extension_;
			auto file_ptr = makeFilePtr(filePath.generic_wstring());
			if (!file_ptr->Open(OpenMode::Create))
				return nullptr;


			this->add_file(file_ptr, pRiffHeader->size);
			return file_ptr;
		}
		uint32_t appendFile(const DataArray & data_array, const uint32_t fileNumber)
		{
			if (fileNumber < files_.size())
				return files_.at(fileNumber)->WriteBlock(data_array);
			return 0;
		}
		bool isAllReady()  
		{
			uint32_t nCount = 0;
			for (auto & theFile : files_)
				if (theFile->isReady())
					++nCount;

			if (nCount == files_.size())
				return true;
			else return false;
		}
		std::size_t count() const
		{
			return files_.size();
		}
	};



	class ZoomH6Raw
		: public SpecialAlgorithm
	{
	private:
		IODevicePtr device_;
		uint32_t block_size_ = default_block_size;
	public:
		ZoomH6Raw(IODevicePtr device)
			: device_(device)
		{

		}
		void setBlockSize(const uint32_t block_size)
		{
			block_size_ = block_size;
		}
		uint32_t block_size() const
		{
			return block_size_;
		}
		uint32_t ReadBlock(DataArray & data_array , const uint64_t offset)
		{
			device_->setPosition(offset);
			return device_->ReadData(data_array.data(), data_array.size());
		}
		std::string readZoomHeaderAndFolderName(const uint64_t header_offset)
		{
			IO::DataArray data_array(this->block_size());

			auto bytesRead = ReadBlock(data_array, header_offset);
			if (bytesRead != data_array.size())
				return std::string();

			// must *.hprj
			if (memcmp(data_array.data(), zoom_h6, zoom_h6_size) != 0)
			{
				LOG_MESSAGE("It's not zoom h6 header.");
				return std::string();;
			}
			ZoomH6Header *pZoomHeader = (ZoomH6Header*)data_array.data();
			std::string folder_name(pZoomHeader->folder_name);

			return folder_name;
		}
		path_string createFolder(const path_string & folder_name, const path_string & new_folder)
		{
			fs::path folder_path(folder_name);
			folder_path += new_folder + L"\\";
			std::error_code ec;
			if (!fs::exists(folder_path))
			if (!fs::create_directory(folder_path, ec))
			{
				LOG_MESSAGE(L"Error to create directory " + folder_path.generic_wstring());
				return path_string();
			}

			return folder_path;

		}
		ZoomFiles createZoomFiles(const uint64_t start_offset, const path_string & zoom_folder, const path_string & zoomFileName)
		{
			ZoomFiles zoomFiles(zoom_folder);
			uint32_t bytesRead = 0;
			uint32_t bytesWritten = 0;
			DataArray data_array(this->block_size());
			uint64_t position = start_offset;

			for (uint32_t fileNumber = 0; fileNumber < MAXZOOMFILES; ++fileNumber)
			{
				bytesRead = ReadBlock(data_array, position);
				if (bytesRead == 0)
					break;

				auto file = zoomFiles.createFile(data_array, zoomFileName, fileNumber);
				if (!file)
					break;

				bytesWritten += zoomFiles.appendFile(data_array, fileNumber);

				position += this->block_size();
			}
			return zoomFiles;
		}
		uint64_t saveZoomFiles(const uint64_t start_offset, ZoomFiles & zoomFiles)
		{
			DataArray data_array(this->block_size());
			uint32_t bytesRead = 0;
			uint32_t bytesWritten = 0;
			uint32_t fileCounter = 0;
			uint64_t position = start_offset;
			while (true)
			{
				if (zoomFiles.isAllReady())
					break;
				// 2 clusters in first file consecutive 
				if (fileCounter == 0)
				{
					//for (auto i = 0; i < 2; ++i)
					{
						bytesRead = ReadBlock(data_array, position);
						if (bytesRead == 0)
							break;
						bytesWritten += zoomFiles.appendFile(data_array, fileCounter);
						position += this->block_size();
					}
				}
				else
				{
					bytesRead = ReadBlock(data_array, position);
					if (bytesRead == 0)
						break;
					bytesWritten += zoomFiles.appendFile(data_array, fileCounter);
					position += this->block_size();
				}
				++fileCounter;
				if (fileCounter >= zoomFiles.count())
					fileCounter = 0;

			}
			return bytesWritten;
		}
		uint64_t Execute(const uint64_t start_offset, const path_string target_folder) override
		{

			setBlockSize(32768);
			if (!device_->isOpen())
			{
				LOG_MESSAGE("Error device isn't opened.");
				return 0;
			}
			uint64_t position = start_offset;

			// must *.hprj

			std::string zoomSubFolder = readZoomHeaderAndFolderName(position);
			if (zoomSubFolder.empty())
				return 0;

			path_string zoomFileName = toWString(zoomSubFolder);
			path_string zoomFolderPath = createFolder(target_folder, zoomFileName);
			if (zoomFolderPath.empty())
				return 0;

			position += this->block_size();
			uint32_t fileCount = 0;

			auto zoomFiles = createZoomFiles(position, zoomFolderPath, zoomFileName);
			if (zoomFiles.count() == 0)
				return 0;

			uint64_t bytesWritten = zoomFiles.count() * this->block_size();
			position += bytesWritten;
			bytesWritten += saveZoomFiles(position, zoomFiles);
			return bytesWritten;

		 }
	};

}