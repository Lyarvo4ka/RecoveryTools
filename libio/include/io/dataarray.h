#pragma once

#include <memory>
#include "constants.h"

namespace IO
{
	//using Size = uint32_t;

	class DataArray
	{
		std::vector<uint8_t> data_;

	public:
		using Ptr = std::unique_ptr<DataArray>;
		
		DataArray() = delete;
		//DataArray(const DataArray&) = delete;

		DataArray(const uint32_t size);

		~DataArray();
		uint32_t size() const;
		ByteArray data();
		ByteArray data() const;
		bool isValid() const;
		void resize(const uint32_t new_size);
		void clear();
		static bool compareData(const DataArray& left, const DataArray& right);
		bool compareData(const ByteArray data, uint32_t size, uint32_t offset = 0);
		bool compareData(const DataArray& dataArray, uint32_t offset = 0);
		friend bool operator == (const DataArray& left, const DataArray& right)
		{
			return compareData(left, right);
		}	

		friend bool operator == (const DataArray::Ptr & left, const DataArray::Ptr & right)
		{
			return compareData(*left , *right);
		}
	
		uint8_t & operator[](size_t index)
		{
			return data_[index];
		}

		uint8_t operator[](size_t index) const
		{
			return data_[index];
		}




	};

	DataArray::Ptr makeDataArray(uint32_t size);
	std::list<std::string> getListFromArray(const DataArray& textArray);
}