#pragma once
#include "raw/quicktime.h"

class FileInfo
{
	IO::path_string filepath_;
public:
	FileInfo(const IO::path_string& filepath) 
	:filepath_(filepath_)
	{
	}
	IO::path_string getFilepath()
	{
		return filepath_;
	}

};


class Analyzer
{
public:
	virtual void analyze() = 0;
};

class QuickTimeAnalyzer
	: public Analyzer
{
	FileInfo fileInfo_;
public:
	QuickTimeAnalyzer(const IO::path_string& filepath)
		:fileInfo_(filepath)
	{

	}
	void analyze() override
	{
		auto testFilePtr = IO::makeFilePtr(fileInfo_.getFilepath());
		testFilePtr->OpenRead();

		RAW::QuickTimeRaw qtRaw(testFilePtr);
		auto qtHandle = qtRaw.readQtAtom(0);
		if (qtHandle.isValid())
		{
			if (qtHandle.compareKeyword("ftyp")
			{
				IO::DataArray buff(qtHandle.size());
				qtFilePtr->setPosition(0);
				qtFilePtr->ReadData(buff);
				//if (memcmp(buff.data(), m4a_text, m4a_text_size) == 0)
				//	return IO::path_string(L"m4a");

			}

		}

		//return IO::path_string();

	}
	IO::path_string analyze_extension()
	{

	}
	FileInfo getInfo() const
	{
		return fileInfo_;
	}
};

