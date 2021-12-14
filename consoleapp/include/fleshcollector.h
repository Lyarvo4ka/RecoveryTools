#pragma once

#include "io/file.h"
#include "io/finder.h"

#include <map>

#pragma pack( push, 1)
struct MarkerData
{
    uint8_t marker1;
    uint8_t marker2;
    uint8_t marker3;
    uint8_t marker4;
};
#pragma pack(pop)

class FleshCollector
{
    uint64_t maxImageSize_ = (uint64_t)128 * 1024 * 1024 * 1024;
    IO::path_string dumpsFolder_;
    uint32_t pageSize_ = 18592;
    uint32_t markerOffset_ = 1024;

    const uint32_t sectorSize = 1024;
    const uint32_t numSectors = 16;
    const uint32_t imagePageSize = 1024 * numSectors;

    std::map<uint32_t, bool> imageMap_;
public:
    FleshCollector(const IO::path_string& dumpFolder)
        :dumpsFolder_(dumpFolder)
    {

    }

    uint32_t getMarker(const IO::DataArray& page)
    {
        MarkerData* pMarkers = (MarkerData*)(page.data() + markerOffset_);
        if (pMarkers->marker1 == 0xFF)
            if (pMarkers->marker2 == 0xFF)
                if (pMarkers->marker3 == 0xFF)
                    if (pMarkers->marker4 == 0xFF)
                        return 0xFFFFFFFF;

        uint8_t marker3 = pMarkers->marker3 & 0b0111'1111;
        uint32_t marker1 = pMarkers->marker1;
        marker1 <<= 16;

        uint32_t Marker32 = pMarkers->marker1 << 15 | pMarkers->marker2 << 7 | marker3;
        return Marker32;
    }
    void dumpToImage(const IO::DataArray& dumpPage, const IO::DataArray& imagePage)
    {
        for (uint32_t i = 0; i < numSectors; ++i)
        {
            memcpy(imagePage.data() + sectorSize * i, dumpPage.data() + (sectorSize + 128) * i, sectorSize);
        }
    }

    uint64_t SaveImage(const IO::path_string& imageFileName)
    {
        IO::File imageFile(imageFileName);
        imageFile.OpenWrite();

        uint64_t dumpOffset = 0x0;
        IO::DataArray dumpPage(pageSize_);
        IO::DataArray imagePage(imagePageSize);

        IO::Finder finder;
        finder.add_extension(L".dump");
        finder.FindFiles(dumpsFolder_);

        auto listFiles = finder.getFiles();


        for (const auto & dumpFilename : listFiles)
        {
            IO::File dumpFile(dumpFilename);
            dumpFile.OpenRead();
            dumpOffset = 0x0;

            while (dumpOffset < dumpFile.Size())
            {

                dumpFile.setPosition(dumpOffset);
                dumpFile.ReadData(dumpPage);

                auto marker = getMarker(dumpPage);



                uint64_t imageOffset = (uint64_t)marker * imagePageSize;
                if (imageOffset < maxImageSize_)
                {
                    uint8_t marker4 = dumpPage[markerOffset_ + 3];

                    dumpToImage(dumpPage, imagePage);

                    imageFile.setPosition(imageOffset);

                    auto findIter = imageMap_.find(marker);
                    if (findIter == imageMap_.end())
                    {
                        if (marker4 == 0x10)
                            imageMap_.insert(std::make_pair(marker, true));

                        imageFile.WriteData(imagePage.data(), imagePage.size());
                    }
                    else
                        if (marker4 == 0x10)
                        {
                            imageFile.WriteData(imagePage.data(), imagePage.size());
                        }
                



                }
                dumpOffset += pageSize_;
            }
        }







        return 0;
    }
};