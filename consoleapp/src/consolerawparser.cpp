#include "consolerawparser.h"
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <string>
using namespace std::string_literals;

const auto d_param_str = "-d"s;
const auto f_param_str = "-f"s;
const auto offset_param_str = "-offset"s;

#include "io/diskdevice.h"
#include "io/file.h"




void printErrorWrongParams()
{
	std::cout << "Wrong params";
}

void ConsoleRawParser::parse(int argc, char* argv[])
{
	if (argc != param_count)
		return;

	std::string offset_txt(argv[offset_param]);
	if (offset_txt == offset_param_str)
	{
		offset_ = boost::lexical_cast<uint64_t>(argv[offset_value]);
		std::cout << "offset : " << offset_ << "(sectors)" << std::endl;
	}
	std::string disk_file_string(argv[source_param]);

	parseDiskOrFileParams(disk_file_string, argv);

	std::string targer_path = argv[target_value];
	IO::path_string offsetsFilename(targer_path.begin(), targer_path.end());
	targetValue_ = offsetsFilename;

	if (iodevice_ == nullptr)
	{
		printErrorWrongParams();
		return;
	}
	if (targetValue_.empty())
	{
		printErrorWrongParams();
		return;
	}

	valid_ = true;

}

void ConsoleRawParser::parseDisk(uint32_t driveNumber)
{
	auto drive_list = IO::ReadPhysicalDrives();
	auto physical_drive = drive_list.find_by_number(driveNumber);
	if (!physical_drive)
	{
		std::cout << "Error open physical drive #" << driveNumber << std::endl;
		return;
	}

	std::cout << "You selected" << std::endl;
	std::cout << "Number : " << driveNumber << std::endl;
	std::cout << "Name :" << physical_drive->getDriveName().c_str() << std::endl;
	std::cout << "Serial number : " << physical_drive->getSerialNumber().c_str() << std::endl;
	std::cout << "Size : " << physical_drive->getSize() << "(bytes)" << std::endl;

	iodevice_ = std::make_shared<IO::DiskDevice>(physical_drive);

}

void ConsoleRawParser::parseDiskOrFileParams(const std::string& disk_file_string, char* argv[])
{
	if (disk_file_string.compare(d_param_str) == 0)
	{
		auto drive_number = boost::lexical_cast<uint32_t>(argv[source_value]);
		parseDisk(drive_number);
	}
	else if (disk_file_string.compare(f_param_str) == 0)
	{
		std::string src_path = argv[source_value];
		iodevice_ = IO::makeFilePtr(IO::path_string(src_path.begin(), src_path.end()));
	}
}
