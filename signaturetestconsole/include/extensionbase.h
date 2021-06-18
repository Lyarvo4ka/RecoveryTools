#pragma once

#include "raw/AbstractRaw.h"

using FormatName = std::string;
using ListFormatName = std::list<FormatName>;
using ExtensionName = std::string;

class ExtensionBase
{
	std::map<ExtensionName, ListFormatName > extensionsMap_;

public:
	void add(const ExtensionName& extensionName, const ListFormatName& listFormantName)
	{
		auto iter = extensionsMap_.find(extensionName);
		if (iter == extensionsMap_.end())
			extensionsMap_.emplace(extensionName, listFormantName);
	}

	ListFormatName find(const ExtensionName& extensionName)
	{
		ListFormatName listFormatName;
		auto iter = extensionsMap_.find(extensionName);
		if (iter != extensionsMap_.end())
			listFormatName = iter->second;
		return listFormatName;
	}
};

class SignatureBase
{
	std::map< FormatName, RAW::FileStruct> headerBase_;

public:
	void add(const JsonFileStruct & jsonFileStruct)
	{
		auto fileStruct = toUniqueFileStruct(jsonFileStruct);
		headerBase_.emplace(jsonFileStruct.name.toStdString(), *fileStruct);
	}
	RAW::FileStruct find( const FormatName & formatName)
	{
		auto findIter = headerBase_.find(formatName);
		if (findIter != headerBase_.end())
			return findIter->second;
		throw;
	}
};