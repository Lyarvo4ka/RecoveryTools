#pragma once

#include "io/iodevice.h"


class ConsoleParser
{
public:
	virtual ~ConsoleParser() {};
	virtual void parse(int argc, char* argv[]) = 0;
};




void printErrorWrongParams();

class ConsoleRawParser
	: public ConsoleParser
{
	uint64_t offset_ = 0;
	IO::IODevicePtr iodevice_ = nullptr;
	IO::path_string targetValue_;

	bool valid_ = false;
public:
	enum  ConsoleParams { offset_param = 1, offset_value, source_param, source_value, target_value, param_count };
public:
	void parse(int argc, char* argv[]) override;
	bool isValid() const
	{
		return valid_;
	}
	void parseDisk(uint32_t driveNumber);
	void parseDiskOrFileParams(const std::string& disk_file_string, char* argv[]);

	IO::IODevicePtr getDevice()
	{
		return iodevice_;
	}
	auto getOffset() const
	{
		return offset_;
	}
	auto getTargetValue() const
	{
		return targetValue_;
	}
};
