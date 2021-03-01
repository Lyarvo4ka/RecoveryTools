#pragma once

#include "io/dataarray.h"
//#include <vector>


namespace RAW
{
	using namespace IO;

	using SignatureArray = std::vector<DataArray>;
	using SignatureArrayPtr = std::vector<DataArray::Ptr>;
	using FotersArray = SignatureArray;

	class SignatureOffset
	{
		uint32_t signature_offset_ = 0;
		SignatureArray signatureArray_;

	public:
		using Ptr = std::unique_ptr<SignatureOffset>;
		SignatureOffset()
		{

		}
		SignatureOffset(DataArray dataArray, uint32_t signature_offset = 0)
		{
			signature_offset_ = signature_offset;
			addSignature(dataArray);
		}
		void setOffset(const uint32_t signature_offset)
		{
			signature_offset_ = signature_offset;
		}
		uint32_t getOffset() const
		{
			return signature_offset_;
		}
		void addSignature(const DataArray& dataArray)
		{
			if (dataArray.isValid())
				signatureArray_.emplace_back(dataArray);
		}
		bool findSignature(const ByteArray data, uint32_t size)
		{
			for (auto& theSignature : signatureArray_)
			{
				if (theSignature.compareData(data, size, signature_offset_))
					return true;
			}
			return false;
		}
		bool findSignature(const DataArray& data_array)
		{
			return findSignature(data_array.data(), data_array.size());
		}
		std::size_t signaturesCount() const
		{
			return signatureArray_.size();
		}

		//bool find(const DataArray& data_array)
		//{
		//	auto iter = std::find(signatureArray_.begin(), signatureArray_.end(), &data_array);
		//	return (iter != signatureArray_.end()) ? true : false;
		//}

	};


	using HeaderArray = std::vector<SignatureOffset>;
	using HeaderArrayPtr = std::vector<SignatureOffset::Ptr>;


	// RAWFile
	// RAWHandle
	// RAWDescription
	// RAWType
	//class RAWDescriptor
	class FileStruct
	{
		HeaderArray headers_;
		std::string formatName_;
		std::string algorithmName_;
		std::string algorithType_;
		std::string category_;
		path_string extension_;
		FotersArray footers_;
		uint32_t footerTailEndSize_ = 0;
		uint64_t maxFileSize_ = 0;
		uint64_t minFileSize_ = 0;
		uint32_t footer_offset_ = 0;
		uint32_t search_block_ = 0;
		bool bValid = false;
	public:
		using Ptr = std::unique_ptr<FileStruct>;

		static Ptr createPtr(const std::string& formatName)
		{
			return std::make_unique<FileStruct>(formatName);
		}

		Ptr clone()
		{
			return std::make_unique<FileStruct>(*this);
		}
		FileStruct(const std::string& formatName)
			: formatName_(formatName)
		{

		}
		bool isValid() const
		{
			return !headers_.empty();
		}
		std::string getName() const
		{
			return formatName_;
		}
		void setAlgorithmName(const std::string& algorithmName)
		{
			algorithmName_ = algorithmName;
		}
		std::string getAlgorithmName() const
		{
			return algorithmName_;
		}
		std::size_t headersCount() const
		{
			return headers_.size();
		}
		void addHeader(const DataArray& dataArray, uint32_t offset)
		{
			auto iter = findByOffset(offset);

			if (iter != headers_.end())
			{
				if (!(*iter).findSignature(dataArray))
					(*iter).addSignature(dataArray);
				else
				{
					printf("This signature is already present\r\n");
				}
			}
			else
				headers_.emplace_back(SignatureOffset(dataArray, offset));
		}

		void addFooter(const DataArray& footer)
		{
			footers_.emplace_back(footer);

		}
		const FotersArray& getFooters() const
		{
			return footers_;
		}
		void setFooterTailEndSize(uint32_t footerTailEndSize)
		{
			footerTailEndSize_ = footerTailEndSize;
		}
		uint32_t getFooterTailEndSize() const
		{
			return footerTailEndSize_;
		}
		void setFooterSearchOffset(uint32_t footer_offset, uint32_t search_block)
		{
			footer_offset_ = footer_offset;
			search_block_ = search_block;
		}
		void setMaxFileSize(uint64_t maxFileSize)
		{
			maxFileSize_ = maxFileSize;
		}
		uint64_t getMaxFileSize() const
		{
			return maxFileSize_;
		}
		void setMinFileSize(const uint64_t minFileSize)
		{
			minFileSize_ = minFileSize;
		}
		uint64_t getMinFileSize() const
		{
			return minFileSize_;
		}
		bool compareWithAllHeaders(ByteArray data, uint32_t size) const
		{
			// deep copy (by value) 
			for (auto theHeader : headers_)
			{
				if (!theHeader.findSignature(data, size))
					return false;
			}
			return true;
		}
		void setExtension(const path_string& extension)
		{
			extension_ = extension;
		}
		path_string getExtension() const
		{
			return extension_;
		}
		void setAlgorithType(const std::string algorithType)
		{
			algorithType_ = algorithType;
		}
		std::string getAlgorithType() const
		{
			return algorithType_;
		}
	private:
		HeaderArray::iterator findByOffset(uint32_t header_offset)
		{
			return std::find_if(headers_.begin(), headers_.end(),
				[header_offset](const SignatureOffset& singOffset)
				{
					return singOffset.getOffset() == header_offset;
				}
			);

		}

	};

}
