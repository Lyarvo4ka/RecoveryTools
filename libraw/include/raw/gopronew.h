#pragma once

#include "raw/gopro.h"
#include "raw/ext4_raw.h"

namespace RAW
{
	const uint8_t GOPRO_MARKER[] = {0x00 , 0x00 , 0x00 , 0x03 , 0x46 };
	const uint32_t GOPRO_MARKER_size = SIZEOF_ARRAY(GOPRO_MARKER);
	const uint8_t GOPRO_MP4_MARKER[] = {0x21 , 0x40 , 0x01 , 0x0C , 0x01 };
	const uint32_t GOPRO_MP4_MARKER_size = SIZEOF_ARRAY(GOPRO_MP4_MARKER);
	const uint32_t MARKER_OFFSET = 855;

	using ListMarkers = std::list<uint32_t>;
	using ArrayListMarkers = std::vector<ListMarkers>;

	class GoProRawNew
	{
		IO::path_string sourcepath_;
		IO::File sourceFile_;
		static constexpr uint32_t kb256 = (uint32_t)256 * 1024;
		uint32_t clusterSize_ = kb256;
		ArrayListMarkers tableListMarkers_;
		MoovData moovData_;
	public:
		GoProRawNew(const IO::path_string& sourcepath)
			:sourcepath_(sourcepath)
			, sourceFile_(sourcepath)
		{

		}
		ArrayListMarkers toArrayListMarkers(const std::vector<uint32_t>& listOffest)
		{
			ListMarkers listMarkers;
			uint32_t prev = 0;
			ArrayListMarkers tableListMarkers;
			auto maxVal = listOffest.back();
			auto nCount = maxVal / clusterSize_;
			tableListMarkers.resize(nCount);

			for (auto i : listOffest)
			{
				auto numCluster = i / clusterSize_;
				if (numCluster != prev)
				{
					
					tableListMarkers.at(prev) = listMarkers;
					listMarkers.clear();
				}
				listMarkers.push_back(i);

				prev = numCluster;
			}
			tableListMarkers.push_back(listMarkers);
			return tableListMarkers;
		}
		bool cmpWithFirstCluster(ListMarkers listMarkers, const uint64_t headerOffset , uint32_t clusterLevel)
		{
			if (listMarkers.empty())
				return false;
			auto fileOffset = listMarkers.back();
			auto clusterOffset = fileOffset -  clusterSize_ * clusterLevel;
			auto max_val = clusterOffset + GOPRO_MARKER_size;
			IO::DataArray buff(max_val);
			if (!sourceFile_.isOpen())
				sourceFile_.OpenRead();

			sourceFile_.setPosition(headerOffset);
			sourceFile_.ReadData(buff);

			for (auto i : listMarkers)
			{
				if (memcmp(buff.data() + i - clusterLevel * clusterSize_, GOPRO_MARKER, GOPRO_MARKER_size) != 0)
					return false;
			}
			return true;
		}
		bool findMoovAddToListMarkers(const uint64_t headerPos)
		{
				auto srcDevice = IO::makeFilePtr(sourcepath_);
				srcDevice->OpenRead();
				GP_Analyzer gpAnalyzer(srcDevice);
				auto headerOffset = headerPos;
				while(true)
				{
					moovData_ = gpAnalyzer.findMOOVData(headerOffset);
					auto stco_table = moovData_.getTable();

					tableListMarkers_ = toArrayListMarkers(stco_table);
					if (cmpWithFirstCluster(tableListMarkers_.at(0), headerPos , 0))
						return true;

					headerOffset = moovData_.getHandle().offset();
					headerOffset /= default_sector_size;
					++headerOffset;
					headerOffset *= default_sector_size;

				} 
				return false;
		}
		void SaveByMarkers(const IO::path_string& ftypfilename, const IO::path_string& target_folder)
		{
			sourceFile_.OpenRead();
			ext4_raw ext4(nullptr);
			auto ftypList = ext4.readOffsetsFromFile(ftypfilename);
			for (const auto headerPos : ftypList)
			//auto headerPos = 41683517440;
			{
				IO::DataArray buff(clusterSize_);
				sourceFile_.setPosition(headerPos);
				sourceFile_.ReadData(buff);
				if ( memcmp(buff.data() + MARKER_OFFSET , GOPRO_MP4_MARKER , GOPRO_MP4_MARKER_size) == 0)
				if (findMoovAddToListMarkers(headerPos))
				{
					auto fileName = target_folder + toHexString(headerPos) + L".mov";
					IO::File targetFile(fileName);
					targetFile.OpenCreate();

					auto offset = headerPos;

					for (uint32_t i = 0; i < tableListMarkers_.size(); ++i)
					{
						while (true)
						{
							auto listPos = tableListMarkers_.at(i);
							if (listPos.empty())
							{
								sourceFile_.setPosition(offset);
								sourceFile_.ReadData(buff);
								targetFile.setPosition((uint64_t)i * clusterSize_);
								targetFile.WriteData(buff.data(), buff.size());

								offset += clusterSize_;

								break;

							}
							if (cmpWithFirstCluster(listPos, offset, i))
							{
								sourceFile_.setPosition(offset);
								sourceFile_.ReadData(buff);
								targetFile.setPosition((uint64_t)i * clusterSize_);
								targetFile.WriteData(buff.data(), buff.size());

								offset += clusterSize_;

								break;
							}
							offset += clusterSize_;
						}
					}
					sourceFile_.setPosition(offset);
					sourceFile_.ReadData(buff);
					targetFile.WriteData(buff.data(), buff.size());


					auto sourcePtr = IO::makeFilePtr(sourcepath_);
					sourcePtr->OpenRead();
					QuickTimeRaw qtRaw(sourcePtr);
					auto ftypHandle = qtRaw.readQtAtom(headerPos);
					if (ftypHandle.isValid())
					{
						uint64_t new_offset = headerPos + ftypHandle.size();
						auto freeHandle = qtRaw.readQtAtom(new_offset);
						if (freeHandle.isValid())
						{
							new_offset = new_offset + freeHandle.size();
							auto mdatHandle = qtRaw.readQtAtom(new_offset);
							if (mdatHandle.isValid())
							{
								uint64_t moov_target_offset = ftypHandle.size() + freeHandle.size() + mdatHandle.size();
								IO::DataArray moov_buff(moovData_.getHandle().size());
								sourceFile_.setPosition(moovData_.getHandle().offset());
								sourceFile_.ReadData(moov_buff);

								targetFile.setPosition(moov_target_offset);
								targetFile.WriteData(moov_buff.data(), moov_buff.size());
							}
						}
					


						
					}
					//headerPos



				}


			}
		}
	};
};