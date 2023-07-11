#include "io/dataarray.h"

namespace IO
{
	DataArray::DataArray(const uint32_t size)
		:data_(size)
	{
	}
	DataArray::~DataArray()
	{
		clear();
	}
	uint32_t DataArray::size() const
	{
		return static_cast<uint32_t>(data_.size());
	}
	ByteArray DataArray::data()
	{
		return data_.data();
	}
	ByteArray DataArray::data() const
	{
		return const_cast<ByteArray>(data_.data());
	}
	bool DataArray::isValid() const
	{
		return !data_.empty();
	}
	void DataArray::resize(const uint32_t new_size)
	{
		if (new_size > 0)
			if (size() != new_size)
			{
				clear();
				data_.resize(new_size);
			}
	}
	void DataArray::clear()
	{
		data_.clear();
	}
	bool DataArray::compareData(const DataArray& left, const DataArray& right)
	{
		if (left.size() == right.size())
			return (memcmp(left.data(), right.data(), left.size()) == 0);
		return false;
	}
	bool DataArray::compareData(const ByteArray data, uint32_t size, uint32_t offset)
	{
		if (size >= this->size())
		{

			if (std::memcmp(data_.data(), data + offset, this->size()) == 0)
				return true;
		}
		return false;
	}
	bool DataArray::compareData(const DataArray& dataArray, uint32_t offset)
	{
		return compareData(dataArray.data(), dataArray.size(), offset);
	}
	DataArray::Ptr makeDataArray(uint32_t size)
	{
		return std::make_unique<DataArray>(size);
	}
	std::list<std::string> getListFromArray(const DataArray& textArray)
	{
		std::list<std::string> listName;
		std::uint64_t pos = 0;
		std::string text;
		while (pos < textArray.size())
		{
			if (textArray[pos] == 0)
			{
				listName.emplace_back(text);
				text.clear();
			}
			else
				text.append((const char*)textArray[pos]);
		}

		return listName;
	}
}