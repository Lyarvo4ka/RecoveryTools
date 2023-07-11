#pragma once
#include "raw/quicktime.h"

class FileInfo
{
	IO::path_string filepath_;
	IO::path_string extension_;
public:
	FileInfo(const IO::path_string& filepath) 
	:filepath_(filepath)
	{
	}
	IO::path_string getFilepath() const
	{
		return filepath_;
	}
	void setExtension(const IO::path_string& extension)
	{
		extension_ = extension;
	}
	IO::path_string getExtension() const
	{
		return extension_;
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
	void extensionFromFtypText (const std::string& ftyp_text)
	{

		if (ftyp_text.compare("crx") == 0 )
			fileInfo_.setExtension( L"CR3");
		else if (ftyp_text.compare("m4a") == 0)
			fileInfo_.setExtension(L"m4a");
		else if (ftyp_text.compare("qt") == 0)
			fileInfo_.setExtension(L"mp4");
		else if (ftyp_text.compare("mp4") == 0)
			fileInfo_.setExtension(L"mp4");
		else if (ftyp_text.compare("avc") == 0)
			fileInfo_.setExtension(L"mp4");
		else if (ftyp_text.compare("3gp") == 0)
			fileInfo_.setExtension(L"3gp");
	}
	void analyze() override
	{
		auto testFilePtr = IO::makeFilePtr(fileInfo_.getFilepath());
		testFilePtr->OpenRead();

		RAW::QuickTimeRaw qtRaw(testFilePtr);
		auto qtHandle = qtRaw.readQtAtom(0);
		if (qtHandle.isValid())
		{
			if (qtHandle.compareKeyword("ftyp"))
			{
				auto ftypData = qtRaw.readFtypData(qtHandle);
				std::string ftyp_text((const char *)ftypData.data());
				ftyp_text.erase(std::remove(ftyp_text.begin(), ftyp_text.end(), ' ') , ftyp_text.end() ); // std::remove(str.begin(), str.end(), 'a'), str.end()
				std::cout << ftyp_text << std::endl;
				extensionFromFtypText(ftyp_text);

				//auto ftypList = getListFromArray(ftypData);

				int k = 1;
				k = 2;

			}
			else
				std::cout << "not ftyp handle" << std::endl;

		}
	}

	FileInfo getInfo() const
	{
		return fileInfo_;
	}
};

