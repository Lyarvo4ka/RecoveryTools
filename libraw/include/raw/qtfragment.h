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
                qt_block_t * pQtBlock = (qt_block_t *)buff[i];
                uint64_t marker_offset = offset + i;
                if (isQuickTimeKeyword(*pQtBlock , s_ftyp) )
                    appendOffsetToFile(ftyp_txt,marker_offset);
                else if (isQuickTimeKeyword(*pQtBlock , s_mdat))
                {
                    appendOffsetToFile(mdat_txt, marker_offset);
                }
                
            }


            offset += default_block_size;
        }
        
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

    bool isContinue(uint32_t * markerArray,  
                    uint32_t numCMP, 
                    const uint64_t mdat_offset , 
                    const uint64_t firstPartSize,
                     IO::FilePtr  srcFile   )
    {
        const uint32_t MARKER = 0x000000;
        IO::DataArray buff(4);
        uint64_t offset = 0;
        auto tableSize = numCMP * sizeof(uint32_t);
        for (uint32_t i = 0; i < numCMP; ++i)
        {
            auto markerPos = markerArray[i];
            toBE32(markerPos);
            if (markerPos > firstPartSize)
            {
                uint64_t markerOffset = mdat_offset + markerPos - firstPartSize;
                srcFile->ReadData(buff);
                if (memcmp(buff.data(), (void*)MARKER, 4) != 0)
                    return false;
            }

        }
        return true;
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
            QuickTimeRaw qtRaw(sourcePtr);
            auto ftypHandle = qtRaw.readQtAtom(ftypOffset);
            if (!ftypHandle.isValid())
                continue;
            auto moovHandle = qtRaw.readQtAtom(ftypOffset + ftypHandle.size() );
            if (!moovHandle.isValid())
                continue;
            auto freeHandleoffset = ftypOffset + ftypHandle.size() + moovHandle.size();
            auto freeHandle = qtRaw.readQtAtom(freeHandleoffset);
            if (!freeHandle.isValid())
                continue;
            auto firstPartSize = ftypHandle.size() + moovHandle.size()  + freeHandle.size();

            auto moovData = qtRaw.readQtHandleData(moovHandle);
            auto STCOPos = findSTCOposition(moovData);
            STCO_Table * pSTCO = (STCO_Table *)moovData[STCOPos];
            for (auto mdatOffset : mdatList)
            {
                auto tableBlock = pSTCO->stco_block; 
                uint64_t markerStart = sizeof(STCO_Table) + STCOPos;
                auto tableSize = NUM_CMP * sizeof(uint32_t);
                IO::DataArray markerTable(tableSize);
                sourcePtr->ReadData(markerTable);
                uint32_t* pTable = (uint32_t*)markerTable.data();

                if (isContinue(pTable, NUM_CMP, mdatOffset, firstPartSize, sourcePtr))
                {
                    auto fileName = target_folder + toHexString(ftypOffset) + L".mov";
                    IO::File targetFile(fileName);
                    targetFile.OpenCreate();
                    qtRaw.appendToFile(targetFile, ftypOffset, firstPartSize);
                    auto mdatHadle = qtRaw.readQtAtom(mdatOffset);
                    auto mdatSize = qtRaw.readQtAtomSize(mdatHadle.size(), mdatOffset);
                    qtRaw.appendToFile(targetFile, mdatOffset, mdatSize);
                }
                //pSTCO->
                // read markers
            }



        }
    }
    // ext4_raw::readOffsetsFromFile
}