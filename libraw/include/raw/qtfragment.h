#pragma once

#include "raw/gopro.h"
#include "io/file.h"
#include "raw/ext4_raw.h"
 
namespace RAW
{
    void appendOffsetToFile(IO::File & targetile, uint64_t offset)
    {
        auto textOffset = StringConverter::toHexString(offset);
        targetile.WriteString(textOffset + "\n");
    }

    void SavePos_ftyp_mdat(const IO::path_string & filename)
    {
        IO::File sourceFile(filename);
        sourceFile.OpenRead();

        uint64_t offset = 0;
        IO::DataArray buff(default_block_size);

        IO::File ftyp_txt(filename + L".ftyp");
        ftyp_txt.OpenCreate();

        IO::File mdat_txt(name + L".mdat"));   
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
                ekse if (isQuickTimeKeyword(*pQtBlock , s_mdat))
                {
                   appendOffsetToFile(mdat_txt,marker_offset)
                }
                
            }


            offset += default_block_size;
        }
        
    }

    uint64_t findSTCOposition(const IO::DataArray & moovData )
    {
        for (uint32_t i = 0; i < moovData - stco_table_name.length(); i++)
        {
            if (memcmp(moovData.data() + i , stco_table_name.data(), stco_table_name.length()) == 0 )
                return i;
        }       
        return 0;
    }

    void saveQtFragment(const IO::path_string & filename,
                        const IO::path_string & ftyp_txt,
                        const IO::path_string & mdat_txt ,
                        const IO::path_string & target_folder)
    {
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
            auto freeHandle = ftypOffset + ftypHandle.size() + moovHandle.size();
            if (!freeHandle.isValid())
                continue;
            uint32_t firstPartSize = ftypHandle.size() + moovHandle.size()  + freeHandle.size();

            auto moovData = qtRaw.readQtHandleData(moovHandle);
            auto STCOPos = findSTCOposition(moovData);
            STCO_Table * pSTCO = (STCO_Table *)moovData.data[STCOPos];
            for (auto mdatOffset : mdatList)
            {
                // read markers
            }



        }
    }
    // ext4_raw::readOffsetsFromFile
}