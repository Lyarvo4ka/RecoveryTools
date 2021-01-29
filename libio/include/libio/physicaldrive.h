#pragma once

#include "constants.h"
#include <windows.h>
#include <initguid.h>
#include <winioctl.h>

#include <setupapi.h>
#pragma comment(lib, "setupapi.lib")

#include <memory>
#include <algorithm>
#include <boost/algorithm/string.hpp>
//#include <d:/develop/boost/boost/boost/algorithm/string.hpp>


namespace IO
{
	class SystemIO
	{
	public:
		virtual HANDLE  CreateFile(
			LPCTSTR               lpFileName,
			DWORD                 dwDesiredAccess,
			DWORD                 dwShareMode,
			LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			DWORD                 dwCreationDisposition,
			DWORD                 dwFlagsAndAttributes,
			HANDLE                hTemplateFile
			)
		{
			return ::CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		}
		virtual BOOL ReadFile(
			HANDLE       hFile,
			LPVOID       lpBuffer,
			DWORD        nNumberOfBytesToRead,
			LPDWORD      lpNumberOfBytesRead,
			LPOVERLAPPED lpOverlapped)
		{
			return ::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
		}
		virtual BOOL WriteFile(
			HANDLE       hFile,
			LPVOID       lpBuffer,
			DWORD        nNumberOfBytesToWrite,
			LPDWORD      lpNumberOfBytesWritten,
			LPOVERLAPPED lpOverlapped)
		{
			return ::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
		}
		virtual HANDLE OpenPhysicalDrive(const path_string & drive_path)
		{
			HANDLE hDevice = this->CreateFile(drive_path.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
			return hDevice;
		}

	};

	inline void exchange_uint8(uint8_t * data, uint32_t size)
	{
		uint16_t *pBuffer = (uint16_t *)data;
		const uint8_t bits_8 = 8;
		const uint8_t bytes = sizeof(uint16_t);

		for (uint32_t i = 0; i < size / bytes; ++i)
		{
			pBuffer[i] = pBuffer[i] << bits_8 | pBuffer[i] >> bits_8;
		}
	}

	class PhysicalDrive
	{
	private:
		uint32_t drive_number_ = 0;
		uint32_t bytes_per_sector_ = 0;
		uint64_t size_ = 0;
		uint32_t transfer_length_ = 0;
		path_string path_ = L"";
		path_string drive_name_ = L"";
		std::string serial_number_ = "";
	public:
		PhysicalDrive()
		{

		}
		void setPath(const path_string & path)
		{
			path_ = path;
		}
		path_string getPath() const
		{
			return path_;
		}
		void setDriveName(const std::wstring & drive_name)
		{
			drive_name_ = drive_name;
		}
		std::wstring getDriveName() const
		{
			return drive_name_;
		}
		void setDriveNumber(uint32_t drive_number)
		{
			drive_number_ = drive_number;
		}
		uint32_t getDriveNumber() const
		{
			return drive_number_;
		}
		void setSize(uint64_t size)
		{
			this->size_ = size;
		}
		uint64_t getSize() const
		{
			return size_;
		}
		uint64_t getNumberSectors() const
		{
			return (getBytesPerSector() != 0 ) ? getSize()/ getBytesPerSector() : 0;
		}
		void setTransferLength(uint32_t transfer_length)
		{
			transfer_length_ = transfer_length;
		}
		uint32_t getTransferLength() const
		{
			return transfer_length_;
		}
		void setBytesPerSector(uint32_t bytes_per_sector)
		{
			bytes_per_sector_ = bytes_per_sector;
		}
		uint32_t getBytesPerSector() const
		{
			return bytes_per_sector_;
		}
		void setSerialNumber(const std::string & serial_number)
		{
			serial_number_ = serial_number;
		}
		std::string getSerialNumber() const
		{
			return serial_number_;
		}

	};

	using PhysicalDrivePtr = std::shared_ptr<PhysicalDrive>;



	class DriveAttributesReader
	{
	private:
		HDEVINFO hDevInfo_ = INVALID_HANDLE_VALUE;
		SP_DEVICE_INTERFACE_DATA spDeviceInterfaceData_ = {0};

	public:
		DriveAttributesReader()
		{
			initHDevInfo(hDevInfo_);
		}

		~DriveAttributesReader()
		{
			closeHDevInfo(hDevInfo_);
		}

		BOOL initHDevInfo(HDEVINFO & hDevInfo)
		{
			if (hDevInfo != INVALID_HANDLE_VALUE)
				closeHDevInfo(hDevInfo);

			hDevInfo = ::SetupDiGetClassDevs(&DiskClassGuid,
				NULL,
				0,
				DIGCF_PROFILE | DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
			if (hDevInfo == INVALID_HANDLE_VALUE)
				return FALSE;

			return TRUE;
		}

		BOOL closeHDevInfo(HDEVINFO &hDevInfo)
		{
			if (hDevInfo != INVALID_HANDLE_VALUE)
			{
				::SetupDiDestroyDeviceInfoList(hDevInfo);
				hDevInfo = INVALID_HANDLE_VALUE;
			}
			return TRUE;
		}

		BOOL isHDevInfoValid() const
		{
			return (hDevInfo_ == INVALID_HANDLE_VALUE);
		}

		BOOL setupDeviceInterfaceData(HDEVINFO &hDevInfo, SP_DEVICE_INTERFACE_DATA & spDeviceInterfaceData, uint32_t member_index)
		{
			::ZeroMemory(&spDeviceInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
			spDeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

			BOOL bResult = ::SetupDiEnumDeviceInterfaces(
				hDevInfo,
				0,
				&DiskClassGuid,
				member_index,
				&spDeviceInterfaceData
			);

			return bResult;
		}
		
		BOOL isValidGUID(GUID guid)
		{
			static char null_array[sizeof(GUID)] = { 0 };
			ZeroMemory(null_array, sizeof(GUID));

			return (memcmp(&guid, null_array, sizeof(GUID)) != 0 ) ? TRUE : FALSE;

		}

		BOOL readDriveAttribute(uint32_t member_index, PhysicalDrivePtr physical_drive)
		{
			path_string drive_path;
			if (!setupDeviceInterfaceData(hDevInfo_, spDeviceInterfaceData_, member_index))
				return FALSE;

			SP_DEVINFO_DATA spDevInfoData;

			if (!readDevicePath(spDevInfoData, drive_path))
				return FALSE;
			physical_drive->setPath(drive_path);

			std::wstring drive_name;
			if (!readDeviceName(spDevInfoData, drive_name))
				return FALSE;
			physical_drive->setDriveName(drive_name);

			DISK_GEOMETRY_EX disk_geometry_ex = { 0 };
			if (!readDeviceGeometryEx(physical_drive->getPath(), disk_geometry_ex) )
				return FALSE;

			physical_drive->setBytesPerSector(disk_geometry_ex.Geometry.BytesPerSector);
			physical_drive->setSize(disk_geometry_ex.DiskSize.QuadPart);

			STORAGE_ADAPTER_DESCRIPTOR storage_descriptor = { 0 };
			if (!readDeviceDescriptor(drive_path, storage_descriptor))
				return FALSE;

			physical_drive->setTransferLength(storage_descriptor.MaximumTransferLength);

			uint32_t drive_number = 0;
			if (!readDeviceNumber(drive_path, drive_number))
				return FALSE;

			physical_drive->setDriveNumber(drive_number);

			std::string serial_number;
			if (!readSerialNumber(drive_path, serial_number))
				return FALSE;

			physical_drive->setSerialNumber(serial_number);


			return TRUE;
		}

		uint32_t getPathStringSize(HDEVINFO & hDevInfo, SP_DEVICE_INTERFACE_DATA & spDeviceInterfaceData)
		{
			DWORD path_size = 0;
			 ::SetupDiGetDeviceInterfaceDetail(hDevInfo_,
				&spDeviceInterfaceData_,
				NULL,
				0,
				&path_size,
				NULL);
			 return path_size;
		}
		BOOL readDevicePath(SP_DEVINFO_DATA & spDevInfoData,  path_string & drive_path)
		{

			DWORD iErrorCode = 0;
			BOOL bStatus = FALSE;

			uint32_t path_size = getPathStringSize(hDevInfo_, spDeviceInterfaceData_);
			if (path_size == 0)
				return FALSE;

			iErrorCode = ::GetLastError();

			if ( iErrorCode != ERROR_INSUFFICIENT_BUFFER)
				return FALSE;

			DWORD dwInterfaceDetailDataSize = path_size;


			char * pTmpBuffer = new char[dwInterfaceDetailDataSize];
			PSP_DEVICE_INTERFACE_DETAIL_DATA pspOUTDevIntDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)pTmpBuffer;

			
			::ZeroMemory(pspOUTDevIntDetailData, dwInterfaceDetailDataSize);

			::ZeroMemory(&spDevInfoData, sizeof(SP_DEVINFO_DATA));
			spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);


			pspOUTDevIntDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			bStatus = ::SetupDiGetDeviceInterfaceDetail(hDevInfo_,
					&spDeviceInterfaceData_,
					pspOUTDevIntDetailData,
					dwInterfaceDetailDataSize,
					NULL,
					&spDevInfoData);
				if (bStatus)
				{
					drive_path = pspOUTDevIntDetailData->DevicePath;
				}

			iErrorCode = ::GetLastError();

			delete[] pTmpBuffer;

			return bStatus;
		}
		uint32_t getNameStringSize(HDEVINFO & hDevInfo, SP_DEVINFO_DATA & spDevInfoData, DWORD & dwRegDataType)
		{
			DWORD ReturnedBytes = 0;
			::SetupDiGetDeviceRegistryProperty(hDevInfo_,
				&spDevInfoData,
				SPDRP_FRIENDLYNAME,
				&dwRegDataType,
				NULL,
				NULL,
				&ReturnedBytes);
			return ReturnedBytes;
		}


		BOOL readDeviceName(SP_DEVINFO_DATA & spDevInfoData , std::wstring & drive_name)
		{
			DWORD dwRegDataType = 0;
			DWORD dwReturnedBytes = 0;
			DWORD dwErrorCode = 0;

			uint32_t nameBufferSize = getNameStringSize(hDevInfo_, spDevInfoData, dwRegDataType);


			if (nameBufferSize == 0)
				return FALSE;

			BYTE * buffer = new BYTE[nameBufferSize];
			ZeroMemory(buffer, nameBufferSize);

			BOOL bResult = FALSE;
			bResult = ::SetupDiGetDeviceRegistryProperty(hDevInfo_,
				&spDevInfoData,
				SPDRP_FRIENDLYNAME,
				&dwRegDataType,
				buffer,
				nameBufferSize,
				&dwReturnedBytes);

			dwErrorCode = ::GetLastError();

			drive_name = (wchar_t*)buffer;
			delete []  buffer;

			return bResult;
		}

		BOOL readDeviceGeometryEx(const path_string & drive_path , DISK_GEOMETRY_EX & disk_geometry_ex)
		{
			SystemIO system_io;
			HANDLE hDevice = system_io.OpenPhysicalDrive(drive_path);

			if (hDevice == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = ::GetLastError();
				return FALSE;
			}

			DWORD returned_bytes = 0;
			BOOL bResult = FALSE;
			bResult = ::DeviceIoControl(hDevice,  // device to be queried
				IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,  // operation to perform
				NULL, 0, // no input buffer
				&disk_geometry_ex, sizeof(DISK_GEOMETRY_EX),     // output buffer
				&returned_bytes,                 // # bytes returned
				(LPOVERLAPPED)NULL);  // synchronous I/O

			::CloseHandle(hDevice);
			return bResult;
		}
		BOOL readDeviceNumber(const path_string & drive_path, uint32_t & drive_number)
		{
			SystemIO system_io;
			HANDLE hDevice = system_io.OpenPhysicalDrive(drive_path);

			if (hDevice == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = ::GetLastError();
				return FALSE;
			}

			DWORD returned_bytes = 0;
			STORAGE_DEVICE_NUMBER ioDeviceNumber;
			BOOL bResult = FALSE;
			bResult = ::DeviceIoControl(hDevice,  // device to be queried
				IOCTL_STORAGE_GET_DEVICE_NUMBER,  // operation to perform
				NULL, 0, // no input buffer
				&ioDeviceNumber, sizeof(ioDeviceNumber),     // output buffer
				&returned_bytes,                 // # bytes returned
				(LPOVERLAPPED)NULL);  // synchronous I/O

			::CloseHandle(hDevice);
			drive_number = ioDeviceNumber.DeviceNumber;
			return bResult;
		}
		BOOL readDeviceDescriptor(const path_string & drive_path, STORAGE_ADAPTER_DESCRIPTOR & storage_descriptor)
		{
			SystemIO system_io;
			HANDLE hDevice = system_io.OpenPhysicalDrive(drive_path);

			if (hDevice == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = ::GetLastError();
				return FALSE;
			}

			storage_descriptor.Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);

			STORAGE_PROPERTY_QUERY	pQueryProperty;
			memset(&pQueryProperty, 0, sizeof(pQueryProperty));
			pQueryProperty.PropertyId = StorageAdapterProperty;
			pQueryProperty.QueryType = PropertyStandardQuery;

			BOOL bResult = FALSE;
			DWORD dwBufferSize = 0;
			bResult = ::DeviceIoControl(hDevice,  // device to be queried
				IOCTL_STORAGE_QUERY_PROPERTY,  // operation to perform
				&pQueryProperty, sizeof(STORAGE_PROPERTY_QUERY), // no input buffer
				&storage_descriptor, sizeof(STORAGE_ADAPTER_DESCRIPTOR),     // output buffer
				&dwBufferSize,                 // # bytes returned
				(LPOVERLAPPED)NULL);  // synchronous I/O

			::CloseHandle(hDevice);

			return bResult;
		}
		BOOL readSerialNumber(const path_string & device_path , std::string & serial_number)
		{
			SystemIO system_io;
			HANDLE hDevice = system_io.OpenPhysicalDrive(device_path);

			if (hDevice == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = ::GetLastError();
				return FALSE;
			}
			const uint32_t size_buff = 512;
			BYTE outBuff[size_buff] = { 0 };
			STORAGE_DEVICE_DESCRIPTOR *pDeviceDesc = (STORAGE_DEVICE_DESCRIPTOR*)outBuff;

			::ZeroMemory(outBuff, size_buff);

			STORAGE_PROPERTY_QUERY	pQueryProperty;
			memset(&pQueryProperty, 0, sizeof(pQueryProperty));
			pQueryProperty.PropertyId = StorageDeviceProperty;
			pQueryProperty.QueryType = PropertyStandardQuery;

			BOOL bResult = FALSE;
			DWORD dwOutSize = 0;

			bResult = ::DeviceIoControl(hDevice,  // device to be queried
				IOCTL_STORAGE_QUERY_PROPERTY,  // operation to perform
				&pQueryProperty, sizeof(STORAGE_PROPERTY_QUERY), // no input buffer
				outBuff, sizeof(outBuff),     // output buffer
				&dwOutSize,                 // # bytes returned
				(LPOVERLAPPED)NULL);  // synchronous I/O

			CloseHandle(hDevice);

			if (bResult == FALSE)
			{
				DWORD iErrorCode = GetLastError();

				return readSerialFromSmart(device_path , serial_number);
			}
			else
				if (pDeviceDesc->SerialNumberOffset)
				{
					std::string serial_number_tmp = (LPSTR)outBuff + pDeviceDesc->SerialNumberOffset;
					if (serial_number_tmp.size() > 0)
					{
						boost::trim(serial_number_tmp);
						serial_number = serial_number_tmp;
						return TRUE;

					}
					else
					{
						return readSerialFromSmart(device_path, serial_number);
					}

				}


			return FALSE;
		}
		BOOL readSerialFromSmart(const path_string & drive_path , std::string & serial_number)
		{
			SystemIO system_io;
			HANDLE hDevice = system_io.OpenPhysicalDrive(drive_path);

			if (hDevice == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = GetLastError();
				return FALSE;
			}
			const uint32_t smart_size = 1024;
			BYTE buff[smart_size] = { 0 };
			ZeroMemory(buff, smart_size);

			PSENDCMDINPARAMS sendCommand = (PSENDCMDINPARAMS)&buff;
			sendCommand->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;

			PSENDCMDOUTPARAMS outCommand = (PSENDCMDOUTPARAMS)&buff;

			BOOL bResult = FALSE;
			DWORD dwBufferSize = 0;
			bResult = DeviceIoControl(hDevice,  // device to be queried
				SMART_RCV_DRIVE_DATA,  // operation to perform
				sendCommand, (sizeof(SENDCMDINPARAMS)), // no input buffer
				outCommand, sizeof(buff),     // output buffer
				&dwBufferSize,                 // # bytes returned
				(LPOVERLAPPED)NULL);  // synchronous I/O

			CloseHandle(hDevice);

			if (bResult == FALSE)
			{
				DWORD iErrorCode = GetLastError();
				return FALSE;
			}

			IDENTIFY_DISK_ATA *PASPORT = NULL;
			PASPORT = (PIDENTIFY_DISK_ATA)outCommand->bBuffer;

			const uint32_t sn_size = sizeof(PASPORT->SerialNumber);
			uint8_t serial_buffer[sn_size + 1];
			ZeroMemory(serial_buffer, sn_size + 1);

			memcpy(serial_buffer, PASPORT->SerialNumber, sn_size);
			exchange_uint8(serial_buffer, sn_size);

			std::string sSerial((const char *)serial_buffer);

			boost::trim(sSerial);
			serial_number = sSerial;

			return TRUE;
		}




	};




	class ListDrives
	{
	private:
		std::vector<PhysicalDrivePtr> drive_list_;
	public:
		void add(PhysicalDrivePtr physical_drive)
		{
			if (!this->find_by_number(physical_drive->getDriveNumber()))
				drive_list_.push_back(physical_drive);
		}
		void remove(uint32_t drive_number)
		{
			for (auto it = drive_list_.begin(); it != drive_list_.end(); ++it)
			{
				if ((*it)->getDriveNumber() == drive_number)
					it = drive_list_.erase(it);

				if (it == drive_list_.end())
					break;

			}
		}
		void remove_all()
		{
			drive_list_.clear();
		}
		PhysicalDrivePtr index(uint32_t index_number)
		{
			if (index_number < getSize())
				return drive_list_.at(index_number);
			return nullptr;
		}
		PhysicalDrivePtr find_by_number(uint32_t drive_number)
		{
			for (auto find_index : drive_list_)
			{
				if (find_index->getDriveNumber() == drive_number)
					return find_index;
			}
			return nullptr;
		}
		uint32_t getSize() const
		{
			return (uint32_t)drive_list_.size();
		}

		void sort()
		{
			std::sort(drive_list_.begin(), drive_list_.end(), [](PhysicalDrivePtr drive1, PhysicalDrivePtr drive2)
			{
				// '<' is increasing order
				// '>' is descending order
				return drive1->getDriveNumber() < drive2->getDriveNumber();
			});
		}
	};

	inline ListDrives ReadPhysicalDrives()
	{
		ListDrives list_drives;
		DriveAttributesReader attribute_reader;
		uint32_t member_index = 0;
		while (true)
		{
			auto physical_drive = std::make_shared<PhysicalDrive>();
			if (!attribute_reader.readDriveAttribute(member_index, physical_drive))
				break;

			list_drives.add(physical_drive);
			++member_index;
		}
		list_drives.sort();
		return list_drives;
	}

/*
		BOOL GetUsbSerial(IDevice * pDevice)
		{
			//HANDLE hDevice = INVALID_HANDLE_VALUE;
			//if (pDevice->GetPath().size() == 0)
			//	return FALSE;
			//hDevice = CreateFile(pDevice->GetPath().c_str(),
			//				   GENERIC_READ | GENERIC_WRITE,
			//				   FILE_SHARE_READ | FILE_SHARE_WRITE  ,
			//				   NULL,
			//				   OPEN_EXISTING,
			//				   0,
			//				   NULL);

			//if ( hDevice == INVALID_HANDLE_VALUE )
			//{
			//	DWORD dwError = GetLastError();
			//	return FALSE;
			//}

			//char buffer[1000];
			//MEDIA_SERIAL_NUMBER_DATA UsbSerialNumber;
			//ZeroMemory(&UsbSerialNumber,sizeof(MEDIA_SERIAL_NUMBER_DATA));

			//DWORD dataSize = 0;
			//BOOL bResult = DeviceIoControl(hDevice,
			//							   IO_GET_MEDIA_SERIAL_NUMBER,
			//							   NULL,
			//							   0,
			//							   &buffer,//(LPVOID)
			//							   sizeof(buffer),
			//							   &dataSize,
			//							   (LPOVERLAPPED)NULL);

			//if (bResult == 0)
			//{
			//	DWORD dwError = GetLastError();

			//	int k = 1;
			//	k = 1;

			//}

			//int k = 1;
			//k = 1;


		}
		BOOL GetSystemDevice(CDiviceList & _deviceList)
		{
			if (_deviceList.GetCount() == 0)
				return FALSE;

			UINT iSysDirSize = 0;
			TCHAR cSystemDirectory[MAX_PATH];
			ZeroMemory(cSystemDirectory, sizeof(TCHAR)*MAX_PATH);


			iSysDirSize = ::GetSystemDirectory(cSystemDirectory, MAX_PATH);
			if (iSysDirSize == 0)
				return FALSE;

			wstring sSystemVolName = L"\\\\.\\";
			sSystemVolName += cSystemDirectory[0];
			sSystemVolName += cSystemDirectory[1];

			HANDLE hVolume = CreateFile(sSystemVolName.c_str(),
				FILE_READ_ATTRIBUTES | SYNCHRONIZE | FILE_TRAVERSE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			if (hVolume == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = GetLastError();
				return FALSE;
			}

			STORAGE_DEVICE_NUMBER ioDeviceNumber;
			DWORD dwBufferSize = 0;
			BOOL bResult = DeviceIoControl(hVolume,  // device to be queried
				IOCTL_STORAGE_GET_DEVICE_NUMBER,  // operation to perform
				NULL, 0, // no input buffer
				&ioDeviceNumber, sizeof(ioDeviceNumber),     // output buffer
				&dwBufferSize,                 // # bytes returned
				(LPOVERLAPPED)NULL);  // synchronous I/O
			CloseHandle(hVolume);

			if (bResult)
			{
				DevicePtr ptrDev = _deviceList.FindDevice(ioDeviceNumber.DeviceNumber);
				if (ptrDev != NULL)
					ptrDev->SetSystemDisk(true);

				return TRUE;
			}



			return FALSE;
		}
	*/
}
