#pragma once

#include "raw/AbstractRaw.h"

class ExtensionExtractor
{
	RAW::HeaderBase::Ptr headerBase_ = std::make_shared< RAW::HeaderBase>();
public:

	void setHeaderBase(RAW::HeaderBase::Ptr headerBase)
	{
		headerBase_ = headerBase;
	}
	void extract_extensions(const IO::path_list& listFiles)
	{
		const uint32_t DefaultReadSize = 33280;
		DataArray buffer(DefaultReadSize);
		for (auto filepath : listFiles)
		{
			uint32_t read_size = DefaultReadSize;
			File file(filepath);
			file.OpenRead();
			if (file.Size() < buffer.size())
				read_size = file.Size();

			file.ReadData(buffer.data(), read_size);
			file.Close();

			auto file_struct = headerBase_->find(buffer.data(), read_size);
			if (file_struct)
			{
				qInfo() << filepath << "-->" << QString::fromStdWString(file_struct->getExtension()) ;
				auto ext = file_struct->getExtension();
				auto filePathWithExt = filepath + file_struct->getExtension();
				fs::rename(filepath, filePathWithExt);
			}
		}
	}
};
