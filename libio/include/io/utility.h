#pragma once

#include "io/constants.h"
#include "io/iodevice.h"
#include "io/file.h"
//#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <locale>
#include <codecvt>
#include <iostream>
//#include <boost\variant\variant.hpp>


#include <filesystem>

namespace fs = std::filesystem;

namespace IO
{
	inline bool isBlockContainsValue(ByteArray data, uint32_t size , uint8_t value_to_compare)
	{
		for (uint32_t i = 0; i < size; i++)
		{
			if (data[i] != value_to_compare)
				return false;
		}
		return true;
	}


	// not working
	inline bool isNot00orFF(ByteArray data, uint32_t size)
	{
		if (isBlockContainsValue(data, size, 0x00))
			return false;
		if (isBlockContainsValue(data, size, 0xFF))
			return false;

		return true;
	}

	inline path_string toWString(const std::string& oneByteString)
	{
		return path_string(oneByteString.begin(), oneByteString.end());
		//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		//return converter.from_bytes(oneByteString);

	}
	inline path_string addBackSlash(const path_string& path_str)
	{
		path_string new_string(path_str);
		if (*path_str.rbegin() != back_slash)
			new_string.push_back(back_slash);

		return new_string;
	}

	inline std::string intToString(const uint32_t int_val)
	{
		try
		{
			return boost::lexical_cast<std::string>(int_val);
		}
		catch (boost::bad_lexical_cast& ex)
		{
			std::cout << "cought error " << ex.what();
		}
		return "";
	}

	inline bool createFoldersFromPath(const path_string& path)
	{

		return false;
	}
	inline bool isDirectoryAttribute(const WIN32_FIND_DATA& attributes)
	{
		return (attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	inline bool isOneDotOrTwoDots(const path_string& name_string)
	{
		if (name_string.compare(OneDot) == 0)
			return true;
		if (name_string.compare(TwoDot) == 0)
			return true;
		return false;
	}


	inline bool isPresentInList(const path_string& text_value, const path_list& listToFind)
	{
		auto str = text_value;
		boost::algorithm::to_lower(str);
		//auto findIter = std::find(listToFind.begin(), listToFind.end(), str);
		for (auto& theExt : listToFind)
		{
			if (theExt.compare(str) == 0)
				return true;
		}
		//return (findIter != listToFind.end()) ? true : false;
		return false;
	}

	inline path_string getExtension(const path_string& file_name)
	{
		fs::path file_path(file_name);	// can crash ????
		return file_path.extension().generic_wstring();
	}

	inline bool isOffice2003(const path_string& file_extension)
	{
		return isPresentInList(file_extension, listExtensions2003);
	}

	inline bool isOffice2007(const path_string& file_extension)
	{
		return isPresentInList(file_extension, listExtensions2007);
	}


	inline path_string toNumberString(const uint32_t number)
	{
		const int numValues = 11;
		wchar_t buff[numValues] = { 0 };
		ZeroMemory(buff, sizeof(wchar_t) * numValues);

		swprintf_s(buff, numValues, L"%.10u", number);
		path_string number_str(buff);
		return number_str;
	}

	inline path_string toHexString(const uint64_t big_value)
	{
		const int numValues = 17;
		wchar_t buff[numValues] = { 0 };
		ZeroMemory(buff, sizeof(wchar_t) * numValues);

		swprintf_s(buff, numValues, L"%.15I64x", big_value);
		path_string number_str(buff);
		return number_str;
	}

	inline path_string toNumberExtension(const uint32_t number, const path_string& extension)
	{
		return toNumberString(number) + extension;
	}
	inline path_string toFullPath(const path_string& folder, const uint32_t number, const path_string& extension)
	{
		return addBackSlash(folder) + toNumberString(number) + extension;
	}
	inline path_string offsetToPath(const path_string& folder, const uint64_t byte_offset, const path_string& extension, uint32_t sector_size = 512)
	{
		return addBackSlash(folder) + toHexString(byte_offset/* / sector_size*/) + extension;
	}

	inline bool createDirectory(const path_string& folder, const path_string& new_folder, path_string& result_folder)
	{
		result_folder = addBackSlash(folder) + new_folder;
		if (!fs::exists(result_folder))
			return fs::create_directory(result_folder);
	}

	inline std::string getDateFromSystemtime(const SYSTEMTIME& system_time)
	{
		char tmp_buffer[255];
		::GetDateFormatA(LOCALE_USER_DEFAULT, 0, &system_time, "yyyy-MM-dd", tmp_buffer, 255);
		std::string tmp_str = tmp_buffer;

		return tmp_str;
	}

	inline std::string getTimeFromSystemtime(const SYSTEMTIME& system_time)
	{
		char tmp_buffer[255];
		::GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &system_time, "HH-mm-ss", tmp_buffer, 255);
		std::string tmp_str = tmp_buffer;

		return tmp_str;
	}

	inline std::string getDateAndTimeFromSystemtime(const SYSTEMTIME& system_time)
	{
		return getDateFromSystemtime(system_time) + '-' + getTimeFromSystemtime(system_time);
	}

	inline std::string parse_string_date(const std::string& original_date)
	{
		// 2017-06-02T13:25:56+03:00
		SYSTEMTIME sys_time = { 0 };
		std::string tmp_str;
		sys_time.wYear = std::stoi(original_date.substr(0, 4));
		sys_time.wMonth = std::stoi(original_date.substr(5, 2));
		sys_time.wDay = std::stoi(original_date.substr(8, 2));
		sys_time.wHour = std::stoi(original_date.substr(11, 2));
		sys_time.wMinute = std::stoi(original_date.substr(14, 2));
		sys_time.wSecond = std::stoi(original_date.substr(17, 2));

		return getDateAndTimeFromSystemtime(sys_time);
	}

	//inline std::string getQuickTimeDate(const IO::path_string & filePath)
	//{

	//}
	struct CanonDate
	{
		//2017:08:08 11:10:02
		char year[4];
		char colon1;
		char month[2];
		char colon2;
		char day[2];
		char white_space;
		char hour[2];
		char colon3;
		char min[2];
		char colon4;
		char sec[2];

	};

	inline uint32_t calc_nulls(const ByteArray data, const uint32_t size)
	{
		uint32_t counter = 0;
		for (uint32_t iByte = 0; iByte < size; ++iByte)
		{
			if (data[iByte] == 0)
				++counter;
		}
		return counter;
	}
	inline uint32_t calc_nulls(const DataArray& data_array)
	{
		return calc_nulls(data_array.data(), data_array.size());
	}


	inline void RenameMP4_date(const IO::path_string& filePath)
	{
		//auto test_file = IO::makeFilePtr(filePath);
		//const uint32_t date_offset = 0x126;
		//const uint32_t date_size = 19;
		//const uint32_t check_size = 0x13A;
		//char buff[date_size + 1];
		//ZeroMemory(buff, date_size + 1);

		//if (test_file->Open(IO::OpenMode::OpenRead))
		//{
		//	if (check_size > test_file->Size())
		//		return;

		//	test_file->setPosition(date_offset);
		//	test_file->ReadData((ByteArray)buff, date_size);


		//	if (buff[0] == '2' && buff[1] == '0')
		//	{
		//		// Canon date
		//		std::string date_string(buff);
		//		std::replace(date_string.begin(), date_string.end(), ' ', '-');
		//		std::replace(date_string.begin(), date_string.end(), '.', '-');
		//		std::replace(date_string.begin(), date_string.end(), ':', '-');

		//		IO::path_string new_date_str(date_string.begin(), date_string.end());

		//		boost::filesystem::path src_path(filePath);
		//		auto folder_path = src_path.parent_path().generic_wstring();
		//		auto only_name_path = src_path.stem().generic_wstring();
		//		auto ext = src_path.extension().generic_wstring();
		//		auto new_name = folder_path + L"//" + new_date_str + L"-" + only_name_path + ext;
		//		test_file->Close();
		//		try
		//		{
		//			boost::filesystem::rename(filePath, new_name);
		//		}
		//		catch (const boost::filesystem::filesystem_error& e)
		//		{
		//			std::cout << "Error: " << e.what() << std::endl;
		//		}

		//	}
		//	else
		//	{
		//		// Sony xml creation date
		//		const std::string temp_xml = "temp.xml";
		//		std::wstring wtemp_xml = L"temp.xml";
		//		auto tmp_file = makeFilePtr(wtemp_xml);
		//		tmp_file->Open(OpenMode::Create);

		//		const char xml_header[] = { 0x3C , 0x3F ,  0x78 , 0x6D , 0x6C };
		//		const uint32_t xml_header_size = SIZEOF_ARRAY(xml_header);

		//		char buffer[default_block_size];
		//		ZeroMemory(buffer, default_block_size);

		//		auto file_size = test_file->Size();
		//		test_file->setPosition(file_size - default_block_size);
		//		auto bytes_read = test_file->ReadData((ByteArray)buffer, default_block_size);
		//		if (bytes_read == 0)
		//		{
		//			wprintf(L"Error. Read size is 0");
		//			return;
		//		}

		//		bool bFoudXml = false;
		//		// Find xml signauture
		//		for (uint32_t iByte = 0; iByte < bytes_read - xml_header_size; ++iByte)
		//		{
		//			if (memcmp(buffer + iByte, xml_header, xml_header_size) == 0)
		//			{
		//				// write to temp file
		//				tmp_file->WriteData((ByteArray)&buffer[iByte], bytes_read - iByte);
		//				tmp_file->Close();
		//				bFoudXml = true;
		//			}
		//		}

		//		if (bFoudXml)
		//		{
		//			tinyxml2::XMLDocument xml_doc;
		//			auto xml_result = xml_doc.LoadFile(temp_xml.c_str());

		//			if (xml_result == tinyxml2::XML_SUCCESS)
		//			{
		//				auto xml_root = xml_doc.RootElement();
		//				auto xmlCreationDate = xml_root->FirstChildElement("CreationDate");
		//				if (xmlCreationDate)
		//				{
		//					auto xml_date_text = xmlCreationDate->Attribute("value");
		//					if (xml_date_text)
		//					{
		//						std::string original_date(xml_date_text);
		//						auto str_date = parse_string_date(xml_date_text);
		//						std::cout << str_date.c_str();

		//						IO::path_string new_date_str(str_date.begin(), str_date.end());

		//						boost::filesystem::path src_path(filePath);
		//						auto folder_path = src_path.parent_path().generic_wstring();
		//						auto only_name_path = src_path.stem().generic_wstring();
		//						auto ext = src_path.extension().generic_wstring();
		//						auto new_name = folder_path + L"//" + new_date_str + L"-" + only_name_path + ext;

		//						test_file->Close();
		//						try
		//						{
		//							boost::filesystem::rename(filePath, new_name);
		//						}
		//						catch (const boost::filesystem::filesystem_error& e)
		//						{
		//							std::cout << "Error: " << e.what() << std::endl;
		//						}

		//					}
		//				}
		//				//auto val_text = xml_date_element->ToText();

		//			}
		//		}

		//	}

		//}

	}

	inline bool isDigitString(const std::string digit_string)
	{
		for (size_t i = 0; i < digit_string.length(); ++i)
			if (isdigit(digit_string.at(i)) == 0)
				return false;
		return true;
	}
	// path ....img016937500.bin
	inline uint64_t fileNameToOffset(const IO::path_string& filePath, uint32_t skip_size = 3)
	{
		fs::path src_path(filePath);
		//auto folder_path = src_path.parent_path().generic_string();
		auto only_name_path = src_path.stem().generic_string();
		//auto ext = src_path.extension().generic_string();

		uint64_t file_offset = 0;
		std::string digits_string(only_name_path.substr(skip_size));
		if (isDigitString(digits_string))
		{
			file_offset = std::stoll(digits_string);

		}
		else
		{
			printf("Error to parse file \n");
		}


		return file_offset;
	}



	const uint8_t EIGHT_FF[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF , 0x78 , 0x01 };
	const uint32_t EIGHT_FF_SIZE = SIZEOF_ARRAY(EIGHT_FF);

	const uint32_t ACRONIS_BLOCK_SIZE = 256 * 1024;

	class AcronisDecompress
	{
	private:
		File file_;
	public:
		AcronisDecompress(const path_string& acronis_file)
			: file_(acronis_file)
		{}
		uint64_t find_8FF(const uint64_t start_offset)
		{
			uint64_t offset = start_offset;
			DataArray data_array(ACRONIS_BLOCK_SIZE);
			uint32_t bytes_read = 0;

			while (offset < file_.Size())
			{
				file_.setPosition(offset);
				bytes_read = file_.ReadData(data_array.data(), data_array.size());
				if (bytes_read == 0)
					break;

				for (uint32_t iByte = 0; iByte < bytes_read - EIGHT_FF_SIZE; ++iByte)
					if (memcmp(data_array.data() + iByte, EIGHT_FF, EIGHT_FF_SIZE) == 0)
					{
						return offset + iByte;
					}

				offset += bytes_read;
			}
			return start_offset;
		}

		int decode_block(const uint64_t start_offset, const uint64_t end_offset, DataArray& dst_data_array);

		uint64_t saveToFile(const path_string& target_file_name, const uint64_t start_offset = 0)
		{
			if (!this->file_.Open(OpenMode::OpenRead))
			{
				wprintf(L"Error open source file.\n");
				return 0;
			}

			File target_file(target_file_name);
			if (!target_file.Open(OpenMode::Create))
			{
				wprintf(L"Error Create target file.\n");
				return 0;
			}

			uint64_t target_offset = 0;
			uint64_t ff_start = find_8FF(start_offset);
			uint64_t offset = ff_start + 8;
			while (true)
			{
				wprintf(L"offset: %I64d\n", offset);
				auto new_block_offset = find_8FF(offset);
				if (new_block_offset == offset)
				{
					wprintf(L"Not found next 0xFFFFFFFFFF\n");
					return 0;
				}

				DataArray decompressed_data(ACRONIS_BLOCK_SIZE);
				auto iRes = decode_block(offset, new_block_offset, decompressed_data);
				if (iRes != 0)
				{
					wprintf(L"Can't decompress data.\n");
					return new_block_offset;
				}
				target_file.setPosition(target_offset);
				if (target_file.WriteData(decompressed_data.data(), decompressed_data.size()) == 0)
				{
					wprintf(L"Error write to file.\n");
				}
				target_offset += decompressed_data.size();

				offset = new_block_offset + EIGHT_FF_SIZE - 2;
			}
			return offset;
		}

	};


	inline uint64_t alingToValue(const uint64_t theValue, const uint32_t toValue)
	{
		if (toValue == 0)
			return 0;
		uint64_t resValue = theValue / toValue;
		resValue *= toValue;
		return resValue;
	}

	inline uint64_t alingToSector(const uint64_t offset, const uint32_t sector_size = default_sector_size)
	{
		return alingToValue(offset, sector_size);
	}

	inline void replaceBadsFromOtherFile(const path_string& withBads_name, const path_string& withoutBads_name, const path_string& target_name)
	{
		File withBads_file(withBads_name);
		withBads_file.Open(OpenMode::OpenRead);

		File withoutBads_file(withoutBads_name);
		withoutBads_file.Open(OpenMode::OpenRead);

		File target_file(target_name);
		target_file.Open(OpenMode::Create);

		uint32_t bytesReadwithBads = 0;
		uint32_t bytesReadwithoutBads = 0;
		uint32_t bytesWritten = 0;

		DataArray data1(default_block_size);
		DataArray data2(default_block_size);
		DataArray target_data(default_block_size);

		bool bEndWithOutBads = false;

		uint64_t offset = 0;

		uint32_t read_size1 = 0;		
		uint32_t read_size2 = 0;


		while (offset < withBads_file.Size())
		{
			read_size1 = calcBlockSize(offset, withBads_file.Size(), data1.size());
			withBads_file.setPosition(offset);
			bytesReadwithBads = withBads_file.ReadData(data1.data() , read_size1);

			if (offset < withoutBads_file.Size())
			{
				read_size2 = calcBlockSize(offset, withoutBads_file.Size(), data2.size());
				withoutBads_file.setPosition(offset);
				bytesReadwithoutBads = withoutBads_file.ReadData(data2.data(), read_size2);


				uint32_t cmp_bytes = bytesReadwithBads;
				if (bytesReadwithoutBads < cmp_bytes)
					cmp_bytes = bytesReadwithoutBads;

				for (uint32_t iSector = 0; iSector < cmp_bytes; iSector += default_sector_size)
				{
					if (memcmp(data1.data() + iSector, Signatures::bad_sector_marker, Signatures::bad_sector_marker_size) == 0)
						memcpy(data1.data() + iSector, data2.data() + iSector, default_sector_size);
				}
			}

			memcpy(target_data.data(), data1.data(), bytesReadwithBads);
			target_file.WriteData(target_data.data(), bytesReadwithBads);


			offset += bytesReadwithBads;
		}

	}

	inline void replaceBadMarkerToNulls(const path_string& fileName , const uint32_t sector_size = 2048)
	{
		File srcfile(fileName);
		srcfile.OpenRead();

		File targetFile(fileName + L".result");
		targetFile.OpenCreate();

		uint64_t offset = 0;

		
		const char unreadable_marker[] = { 0x55 , 0x4E , 0x52 , 0x45 , 0x41 , 0x44 , 0x41 , 0x42 , 0x4C , 0x45 , 0x53 , 0x45 , 0x43 , 0x54 , 0x4F , 0x52 };
		const uint32_t unreadable_marker_size = SIZEOF_ARRAY(unreadable_marker);

		DataArray sector(sector_size);

		while (offset < srcfile.Size())
		{
			srcfile.setPosition(offset);
			srcfile.ReadData(sector);

			if (memcmp(sector.data(), unreadable_marker, unreadable_marker_size) == 0)
			{
				ZeroMemory(sector.data(), sector.size());
			}

			targetFile.WriteData(sector.data(), sector.size());

			offset += sector.size();
		}

	}
};