#pragma once
#include "physicaldrive.h"
#include <string>
#include <string_view>
#include <functional>
#include "dataarray.h"
#include "error.h"

#include "ioengine.h"

namespace IO
{

	inline bool isMultiple(uint64_t number, uint32_t multiple_by)
	{
		if (multiple_by == 0)
			return false;
		return (number % multiple_by == 0);
	}




	struct DeviceInfo
	{
		std::string deviceTypeName;
		std::wstring name;
		uint32_t deviceID;

	};


	class IODevice
	{
	public:
		virtual ~IODevice() = 0 {};
		virtual bool Open(OpenMode) = 0;
		virtual void Close() = 0;
		virtual bool isOpen() = 0;
		virtual void setPosition(uint64_t offset) = 0;
		virtual uint64_t getPosition()  const = 0;
		virtual uint32_t ReadData(ByteArray data, uint32_t read_size) = 0;
		virtual uint32_t WriteData(ByteArray data, uint32_t read_size) = 0;
		virtual uint64_t Size() const = 0;
		virtual DeviceInfo getDeviceInfo() const = 0;
	};


		inline Error::IOStatus makeErrorStatus(IODevice & io_device , Error::IOErrorsType error_type)
		{
			auto error_message = Error::getDiskOrFileError(error_type, io_device.getDeviceInfo().deviceTypeName);
			auto lastError = ::GetLastError();
			Error::IOStatus error_status(error_type, error_message, lastError);
			return error_status;

		}
		//inline Error::IOStatus makeErrorStatus(IODevice * io_device, Error::IOErrorsType error_type)
		//{
		//	auto error_message = Error::getDiskOrFileError(error_type, io_device->getDeviceInfo().deviceTypeName);
		//	auto lastError = ::GetLastError();
		//	Error::IOStatus error_status(error_type, error_message, lastError);
		//	return error_status;

		//}


	using IODevicePtr = std::shared_ptr<IODevice>;



}
