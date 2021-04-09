#pragma once
#include "iodevice.h"

namespace IO
{
	class DiskDevice
		: public IODevice
	{
	private:
		IOEnginePTR io_engine_ ;
		PhysicalDrivePtr physical_drive_;
		DeviceInfo deviceInfo_;
	public:
		DiskDevice(PhysicalDrivePtr physical_drive);
		bool Open(OpenMode open_mode) override;
		void Close() override;
		bool isOpen() override;
		void setPosition(uint64_t offset) override;
		uint64_t getPosition() const override;


		uint32_t ReadData(ByteArray data, uint32_t read_size) override;
		uint32_t WriteData(ByteArray data, uint32_t write_size) override;

		uint64_t Size() const override;
		DeviceInfo getDeviceInfo() const override;
		void setIOEngine(IOEnginePTR ioengine_ptr);
		PhysicalDrivePtr getPhysicalDrive();

		uint32_t ReadDataNotAligned(ByteArray data, uint32_t read_size);
		uint32_t ReadBlock(ByteArray data, uint32_t read_size);
		uint32_t WriteBlock(ByteArray data, uint32_t write_size);
	};
}
