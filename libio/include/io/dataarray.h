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

		DataArray(const uint32_t size)
			:data_(size)
		{
		}

		~DataArray()
		{
			clear();
		}
		uint32_t size() const
		{
			return static_cast<uint32_t>(data_.size());
		}
		ByteArray data()
		{
			return data_.data();
		}
		ByteArray data() const
		{
			return const_cast<ByteArray>(data_.data());
		}
		bool isValid() const
		{
			return !data_.empty();
		}
		void resize(const uint32_t new_size)
		{
			if (new_size > 0)
			if (size() != new_size)
			{
				clear();
				data_.resize(new_size);
			}
		}
		void clear()
		{
			data_.clear();
		}
		static bool compareData(const DataArray& left, const DataArray& right)
		{
			if (left.size() == right.size())
				return (memcmp(left.data(), right.data(), left.size()) == 0);
			return false;
		}
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

		bool compareData(const ByteArray data, uint32_t size, uint32_t offset = 0)
		{
			if (size >= this->size())
			{

				if (std::memcmp(data_.data(), data + offset, this->size()) == 0)
					return true;
			}
			return false;
		}
		bool compareData(const DataArray& dataArray, uint32_t offset = 0)
		{
			return compareData(dataArray.data(), dataArray.size(), offset);
		}

	};

	inline DataArray::Ptr makeDataArray(uint32_t size)
	{
		return std::make_unique<DataArray>(size);
	}
}