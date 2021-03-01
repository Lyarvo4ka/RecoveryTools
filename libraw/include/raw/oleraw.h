#pragma once

#include "AbstractRaw.h"
#include "factories.h"

#include <algorithm>

#include "io/finder.h"

namespace RAW
{


	class OleRAW
		: public DefaultRaw
	{
		uint64_t sizeToWrite = 0;
	public:
		OleRAW(IO::IODevicePtr device)
			: DefaultRaw(device)
		{

		}

		uint64_t SaveRawFile(File& target_file, const uint64_t start_offset)   override
		{
			if (sizeToWrite > 0)
				return appendToFile(target_file, start_offset, sizeToWrite);

			return 0;
		}
		bool Specify(const uint64_t start_offset)
		{

			//OLEReader ole_reader(this->getDevice() , start_offset);
			//ole_reader.read();
			//sizeToWrite = ole_reader.getFileSize();
			//if (sizeToWrite == 0)
			//	return false;
			return true;
		}



	};

	class OleRawFactory :
		public RawFactory
	{
	public:
		RawAlgorithm* createRawAlgorithm(IO::IODevicePtr device) override
		{
			return new OleRAW(device);
		}
	};

	constexpr std::wstring_view Workbook_text = L"Workbook";
	constexpr std::wstring_view WorkDocument_text = L"WordDocument";
	constexpr std::wstring_view PowerPoint_text = L"PowerPoint Document";
	constexpr std::wstring_view FamilyMembers_text = L"FamilyMembers";
	constexpr std::wstring_view JDraftViewerInfo_text = L"JDraftViewerInfo";
	constexpr std::wstring_view IOT_text = L"IOT";

	static std::list< std::wstring_view> listOleKeywords = { Workbook_text ,WorkDocument_text,PowerPoint_text ,FamilyMembers_text ,JDraftViewerInfo_text , IOT_text };

	path_string getExtensionFromKeyword(std::wstring_view keyword)
	{
		if (keyword == Workbook_text)
			return L".xls";
		else if (keyword == WorkDocument_text)
			return L".doc";
		else if (keyword == PowerPoint_text)
			return L".ppt";
		else if (keyword == FamilyMembers_text)
			return L".par";
		else if (keyword == JDraftViewerInfo_text)
			return L".dft";
		else if (keyword == IOT_text)
			return L".dft";

		return L"";
	}

	class OleAnalyzer
	{
		std::wstring extension_;
	public:
		void analyze(const path_string& file_path)
		{
			auto filePtr = IO::makeFilePtr(file_path);
			filePtr->OpenRead();

			//OLEReader oleReader(filePtr);
			//oleReader.read();
			//oleReader.readRoot();
			//filePtr->Close();

			//for (auto& keyword_entry : listOleKeywords)
			//{
			//	if (oleReader.findKeywordEntry(keyword_entry))
			//	{
			//		auto new_extension = getExtensionFromKeyword(keyword_entry);
			//		if (!new_extension.empty())
			//		{
			//			extension_ = new_extension;
			//			break;
			//		}
			//	}
			//}
			//auto new_extemsion = getExtensionFromKeyword()

		}
		std::wstring get_extension() const
		{
			return extension_;
		}
	};



	//void testOLE()
	//{
	//	auto foldername = LR"(d:\incoming\46460\raw\)";

	//	IO::Finder finder;
	//	finder.FindFiles(foldername);

	//	for (auto& fileName : finder.getFiles())
	//	{

	//		//auto filename = LR"(d:\ole_test\2012-08-05-10-29-0000344.doc)";
	//		//auto filename = LR"(d:\ole_test\Assy.dft)";

	//		auto filePtr = IO::makeFilePtr(fileName);
	//		filePtr->OpenRead();

	//		OLEReader oleReader(filePtr);
	//		oleReader.read();
	//		oleReader.readRoot();
	//		filePtr->Close();

	//		OleAnalyzer ole_analyzer;
	//		ole_analyzer.analyze(fileName);
	//		auto new_extension = ole_analyzer.get_extension();

	//		if (!new_extension.empty())
	//			fs::rename(fileName, fileName + new_extension);



	//	}

	//}
}