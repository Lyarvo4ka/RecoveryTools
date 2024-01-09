#pragma once

#include "raw/gopro.h"
#include "io/file.h"
#include "raw/ext4_raw.h"
 
namespace RAW
{
    void appendOffsetToFile(IO::File & targetile, uint64_t offset)
    {
        auto textOffset = StringConverter::toHexString(offset);
        targetile.WriteText(textOffset + "\n");
    }

    void SavePos_ftyp_mdat(const IO::path_string & filename)
    {
        IO::File sourceFile(filename);
        sourceFile.OpenRead();

        uint64_t offset = 0;
        IO::DataArray buff(default_block_size);

        IO::File ftyp_txt(filename + L".ftyp");
        ftyp_txt.OpenCreate();

        IO::File mdat_txt(filename + L".mdat");
        mdat_txt.OpenCreate();

        while (offset < sourceFile.Size())
        {
            sourceFile.setPosition(offset);
            sourceFile.ReadData(buff);

            for (uint32_t i = 0; i < buff.size(); i += default_sector_size)
            {
                qt_block_t * pQtBlock = (qt_block_t *)(buff.data() + i);
                uint64_t marker_offset = offset + i;
                if (isQuickTimeKeyword(*pQtBlock , s_ftyp) )
                    appendOffsetToFile(ftyp_txt,marker_offset);
                else if (isQuickTimeKeyword(*pQtBlock , s_mdat))
                {
                    appendOffsetToFile(mdat_txt, marker_offset);
                }
                else if (isQuickTimeKeyword(*pQtBlock, s_free))
                {
                    appendOffsetToFile(mdat_txt, marker_offset);
                }

            }


            offset += default_block_size;
        }
        
    }

    uint64_t find_co64_position(const IO::DataArray& moovData)
    {
        for (uint32_t i = 0; i < moovData.size() - co64_table_name.length() ; i++)
        {
            if (memcmp(moovData.data() + i, co64_table_name.data(), co64_table_name.length()) == 0)
                return i;
        }
        return 0;
    }


    uint64_t findSTCOposition(const IO::DataArray & moovData )
    {
        for (uint32_t i = 0; i < moovData.size() - stco_table_name.length(); i++)
        {
            if (memcmp(moovData.data() + i , stco_table_name.data(), stco_table_name.length()) == 0 )
                return i;
        }       
        return 0;
    }

    const uint64_t MAX_SIZE_256 = 249309429760;

#pragma pack(1)
    struct co64_t
    {
        char name[4];
        uint64_t co64_size;
    };
#pragma pack

    bool isContinue(uint64_t * markerArray,  
                    uint32_t numCMP, 
                    const uint64_t mdat_offset , 
                    const uint64_t firstPartSize,
                     IO::FilePtr  srcFile   )
    {
        const uint16_t MARKER = 0x0000;
        const uint16_t MARKER2 = 0x0100;
        const uint16_t MARKER3 = 0x0200;
        const uint8_t PANASONIC_MARKER[] = { 0x00 , 0x00 , 0x00 , 0x02 , 0x09 , 0x10 };
        const uint32_t PANASONIC_MARKER_size = SIZEOF_ARRAY(PANASONIC_MARKER);
        IO::DataArray buff(PANASONIC_MARKER_size);
        uint64_t offset = 0;
        uint64_t markerOffset = 0;
        auto tableSize = numCMP * sizeof(uint64_t);
        uint32_t counter = 0;
        for (uint32_t i = 0; i < numCMP; ++i)
        {
            auto markerPos = markerArray[i];
            toBE64(markerPos);
            if (markerPos > firstPartSize)
            {
                markerOffset = mdat_offset + markerPos - firstPartSize;

                if (markerOffset >= MAX_SIZE_256)
                    return false;
                srcFile->setPosition(markerOffset);
                srcFile->ReadData(buff);
                if (memcmp(buff.data() , PANASONIC_MARKER , PANASONIC_MARKER_size) == 0)
                //uint16_t* pMarker = (uint16_t*)buff.data();
                //toBE16(*pMarker);
                //if (*pMarker <= 9)
                {
                    ++counter;
                    if (counter >= numCMP)
                    {
                        int k = 1;
                        k = 2;
                        return true;
                    }
                }



            }


        }

        return false;
    }

    void saveQtFragment(const IO::path_string & filename,
                        const IO::path_string & ftyp_txt,
                        const IO::path_string & mdat_txt ,
                        const IO::path_string & target_folder)
    {
        const uint32_t NUM_CMP = 5;
        ext4_raw ext4(nullptr);
        auto ftypList = ext4.readOffsetsFromFile(ftyp_txt);
        auto mdatList = ext4.readOffsetsFromFile(mdat_txt);

        auto sourcePtr = IO::makeFilePtr(filename);
        sourcePtr->OpenRead();

        for (auto ftypOffset : ftypList)
        {
           // ftypOffset = 0xbb6600000;
            // ftypOffset = 0x2740000;
            QuickTimeRaw qtRaw(sourcePtr);
            auto ftypHandle = qtRaw.readQtAtom(ftypOffset);
            if (!ftypHandle.isValid())
                continue;
            auto moovHandle = qtRaw.readQtAtom(ftypOffset + ftypHandle.size() );
            if (!moovHandle.isValid())
                continue;
            auto freeHandleoffset = ftypOffset + ftypHandle.size() + moovHandle.size();
            uint64_t freeSize = 0;
            auto freeHandle = qtRaw.readQtAtom(freeHandleoffset);
            if (freeHandle.isValid())
                freeSize = freeHandle.size();
            auto firstPartSize = ftypHandle.size() + moovHandle.size()  + freeSize;

            auto moovData = qtRaw.readQtHandleData(moovHandle); 
            if (moovData.size() == 0)
            {
                continue;
            }
            auto co64Pos = find_co64_position(moovData); // 
            co64_t * p_co64 = (co64_t*)moovData[co64Pos];
            for (auto mdatOffset : mdatList)
            {
                //auto tableBlock = pSTCO->stco_block; 
                auto size_co64t = sizeof(co64_t);
                uint64_t markerStart = ftypOffset + size_co64t + co64Pos + 8 + ftypHandle.size();
                auto tableSize = NUM_CMP * sizeof(uint64_t);
                IO::DataArray markerTable(tableSize);
                sourcePtr->setPosition(markerStart);
                sourcePtr->ReadData(markerTable);
                uint64_t* pTable = (uint64_t*)markerTable.data();

                if (isContinue(pTable, NUM_CMP, mdatOffset, firstPartSize, sourcePtr))
                {
                    auto fileName = target_folder + toHexString(ftypOffset) + L"_" + toHexString(mdatOffset) + L".mov";
                    IO::File targetFile(fileName);
                    targetFile.OpenCreate();
                    qtRaw.appendToFile(targetFile, ftypOffset, firstPartSize);
                    auto mdatHadle = qtRaw.readQtAtom(mdatOffset);
                    //if (mdatHadle.compareKeyword("free"))
                    //{ 
                    //    auto free_size = qtRaw.readQtAtomSize(mdatHadle.size(), mdatOffset);
                    //    qtRaw.appendToFile(targetFile, mdatOffset, free_size);
                    //    mdatOffset += free_size;
                    //    mdatHadle = qtRaw.readQtAtom(mdatOffset);
                    //}
                    auto mdatSize = mdatHadle.size();//qtRaw.readQtAtomSize(mdatHadle.size(), mdatOffset);
                    qtRaw.appendToFile(targetFile, mdatOffset, mdatSize);
                    targetFile.Close();
                    int k = 1;
                    k = 2;
                }
                //pSTCO->
                // read markers
            }



        }
    }
    // ext4_raw::readOffsetsFromFile
}