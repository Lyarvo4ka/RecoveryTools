#include <QtCore/QCoreApplication>

#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

#include <QDebug>
#include <QList>

#include <iostream>

#include "json\jsonreader.h"
#include "json\signaturereader.h"

const int param_count = 6;
const int offset_param = 1;
const int offset_value = 2;
const int disk_file_param = 3;
const int source_value = 4;
const int target_value = 5;

const std::string d_str = "-d";
const std::string f_str = "-f";
const std::string offset_str = "-offset";

#include <filesystem>
namespace fs = std::filesystem;

#include "raw\quicktime.h"
#include "io\diskdevice.h"
#include "raw/zoomh6raw.h"
#include "raw/gopro.h"
#include "raw/canonfragment.h"
#include "raw/quicktime.h"
#include "raw/webmraw.h"
#include "raw/prproj.h"

void initFactoryMananger(RAW::RawFactoryManager & factory_manager)
{
	//factory_manager.Register("djvu", std::make_unique < RAW::DjvuRawFactory>());
	//factory_manager.Register("ole", std::make_unique < RAW::OleRawFactory>());

	//factory_manager.Register("Canon4FileFragmentRaw", std::make_unique<Canon4FileFragmentRawFactory>());
	//Canon4FileFragmentRaw
	//factory_manager.Register("7z", std::make_unique<IO::Raw7zFactory>());
	//factory_manager.Register("prproj" , std::make_unique<RAW::RawAdobePremireFactory>());
	//initVideoFactoryManager(factory_manager);

	//factory_manager.Register("flp", std::make_unique<IO::RawFLPFactory>());
	//factory_manager.Register("DjiDrone", std::make_unique<DjiDroneRawFactory>());


	//initKeysFactoryManager(factory_manager);
	//factory_manager.Register("canonfragment", std::make_unique<RAW::QTFragmentRawFactory>());
	//initAudioFactoryManager(factory_manager);
	//factory_manager.Register("webm", std::make_unique<RAW::RawWebmFactory>());

	//factory_manager.Register("pln", std::make_unique<RAW::PLNRawFactory>());
	//factory_manager.Register("pln_bl", std::make_unique<RAW::PLN_BLRawFactory>());

	//factory_manager.Register("keychain-db", std::make_unique<IO::KeychainRawFactory>());

	//factory_manager.Register("mxf", std::make_unique<IO::RawMXFFactory>());

	//factory_manager.Register("go_pro", std::make_unique<IO::GoProRawFactory>());



	//factory_manager.Register("BlackVue", std::make_unique<IO::BlackVue_QtRawFactory>());
	//factory_manager.Register("mx7", std::make_unique<IO::RawFIFFFactory>());

	//factory_manager.Register("r3d", std::make_unique<IO::StandartRawFactory>());
	//factory_manager.Register("ZOOMHandyRecorder", std::make_unique<IO::RawZOOMHandyRecorder>());
	
	//factory_manager.Register("imd", std::make_unique<IO::RawIMDFactory>());
	//factory_manager.Register("cdw", std::make_unique<IO::RawCWDFactory>());

	//factory_manager.Register("Canon80D", std::make_unique<IO::Canon80D_FragmentRawFactory>());
	
	//factory_manager.Register("canonstartfragment", std::make_unique<RAW::CanonStartFragmentFactory>());
	factory_manager.Register("CanonEOSR6", std::make_unique<RAW::CanonStartFragmentFactory>());


}

//#include "zlib.h"

constexpr uint8_t enc_val = 0x8a;
constexpr uint8_t chiper = 0x59;

constexpr uint8_t res_val = enc_val - chiper;


int main(int argc, char *argv[])
{

	QCoreApplication a(argc, argv);
	IO::IODevicePtr src_device = nullptr;
	
	// 1.-offset 2.offset_val 3.(-d -f) 4.path 5.target
	if (argc == param_count)
	{
		uint64_t start_offset = 0;
		std::string offset_txt(argv[offset_param]);
		if (offset_txt.compare(offset_str) == 0)
		{
			start_offset = boost::lexical_cast<uint64_t>(argv[offset_value]);
			qInfo() << "offset : " << start_offset <<"(sectors)";
		}


		std::string disk_file_string(argv[disk_file_param]);
		if (disk_file_string.compare(d_str) == 0)
		{
			auto drive_number = boost::lexical_cast<uint32_t>(argv[source_value]);

			auto drive_list = IO::ReadPhysicalDrives();
			auto physical_drive = drive_list.find_by_number(drive_number);
			if (!physical_drive)
			{
				qInfo() << "Error open physical drive #" << drive_number;
				return -1;
			}
			start_offset *= physical_drive->getBytesPerSector();
			if (physical_drive)
			{
				qInfo() << "You selected";
				qInfo() << "Number : " << drive_number;
				qInfo() << "Name :" << physical_drive->getDriveName().c_str();
				qInfo() << "Serial number : " << physical_drive->getSerialNumber().c_str();
				qInfo() << "Size : " << physical_drive->getSize() << "(bytes)";
			}
			src_device = std::make_shared<IO::DiskDevice>(physical_drive);
		}
		else if (disk_file_string.compare(f_str) == 0)
		{
			std::string src_path = argv[source_value];
			src_device = IO::makeFilePtr(IO::path_string(src_path.begin(), src_path.end()));
			start_offset *= default_sector_size;
		}

		if (!src_device->Open(IO::OpenMode::OpenRead))
		{
			qInfo() << "Error open source device.";
			return -1;
		}

		std::string targer_path = argv[target_value];
		IO::path_string target_folder(targer_path.begin(), targer_path.end());

		if (!src_device)
			return -1;

		//////////////////////////////////////////////////////////////////////////
		
		auto current_folder = fs::current_path();
		//IO::path_string folderSignatures = LR"(d:\develop\RecoveryProjects\SignatureTestConsole\signatures\other\)";
		IO::path_string folderSignatures = current_folder.generic_wstring();

		SignatureReader signatureReader;
		signatureReader.loadAllSignatures(folderSignatures , L".json");
		QList<JsonFileStruct> listFileStruct = signatureReader.getAllSignatures();
		//QString json_file = R"(d:\develop\libio\RawRecoveryConsole\base\video\video.json)";
		//QString json_file = "prproj.json";
		//QFile file(json_file);
		//if (!file.open(QIODevice::ReadOnly))
		//{
		//	qInfo() << "Error to open file. \"" << file.fileName() << "\"";
		//	return -1;
		//}

		//auto json_str = file.readAll();
		//ReadJsonFIle(json_str, listFileStruct);
		//if ( listFileStruct.empty())
		//{
		//	qInfo() << "Error to read" << file.fileName() << "file. Wrong syntax.";
		//	return -1;
		//}

		RAW::HeaderBase::Ptr headerBase = std::make_shared<RAW::HeaderBase>();
		for (auto theFileStruct : listFileStruct)
			
			headerBase->addFileFormat(toUniqueFileStruct(theFileStruct));

		RAW::RawFactoryManager factory_manager;
		initFactoryMananger(factory_manager);
		//factory_manager.Register("qt_canon", std::make_unique<IO::Canon80D_FragmentRawFactory>());

		RAW::SignatureFinder signatureFinder(src_device, headerBase);

		//start_offset = 0x11DE3200;
		uint64_t header_offset = 0;
		uint32_t counter = 0;
		//const IO::path_string dst_folder = L"d:\\incoming\\43944\\result\\";
		while (start_offset < src_device->Size())
		{
			auto file_struct = signatureFinder.findHeader(start_offset, header_offset);
			if (!file_struct)
			{
				qInfo() << Qt::endl << Qt::endl << Qt::endl << "No more signatures found. Press any key to exit.";
				break;
			}
			qInfo() << "Found signature for [" << file_struct->getName().c_str() << "] file.";
			qInfo() << "Offset : " << header_offset << "(bytes)";

			start_offset = header_offset;
			/*
				if (type ==special) find in other base factory
			*/

			if (file_struct->getAlgorithType().compare("special") == 0)
			{
				RAW::ZoomH6Raw zoomH6Raw(src_device);
					auto bytesWritten = zoomH6Raw.Execute(header_offset, target_folder);
					if (bytesWritten == 0)
						break;
					start_offset += default_sector_size;
			}
			else
			{
				auto raw_factory = factory_manager.Lookup(file_struct->getAlgorithmName());
				RAW::RawAlgorithm* raw_algorithm = nullptr;
				if (!raw_factory)
				{
					RAW::StandartRaw* standard_raw = new RAW::StandartRaw(src_device);
					standard_raw->setMaxFileSize(file_struct->getMaxFileSize());
					standard_raw->setFooters(file_struct->getFooters());
					standard_raw->setTailSize(file_struct->getFooterTailEndSize());

					raw_algorithm = standard_raw;

				}
				else
				{
					raw_algorithm = raw_factory->createRawAlgorithm(src_device);
					RAW::StandartRaw* tmpPtr = dynamic_cast<RAW::StandartRaw*>(raw_algorithm);
					if (tmpPtr)
						tmpPtr->setMaxFileSize(file_struct->getMaxFileSize());
				}

				if (raw_algorithm->Specify(header_offset))
				{
					auto target_file = IO::offsetToPath(target_folder, header_offset, file_struct->getExtension(), default_sector_size);
					auto dst_file = IO::makeFilePtr(target_file);
					if (dst_file->Open(IO::OpenMode::Create))

					{

						auto target_size = raw_algorithm->SaveRawFile(*dst_file, header_offset);

						if (target_size == 0)
							qInfo() << "Error to save file.";



						auto dst_size = dst_file->Size();
						dst_file->Close();
						qInfo() << "Successfully saved " << target_size << "(bytes)" << Qt::endl << Qt::endl;

						uint64_t jump_size = default_sector_size;

						if (raw_algorithm->Verify(target_file))
						{
							target_size /= default_sector_size;
							target_size *= default_sector_size;
							//////////////////////////////////////////////////////////////////////////
							jump_size = target_size;
						}
						else
						{
							// remove file
							IO::path_string new_fileName = target_file + L".bad_file";
							fs::rename(target_file, new_fileName);
							qInfo() << "Renamed to .bad_file";
							//{
							//	qInfo() << "File" << target_file.c_str() << "was removed." << endl;
							//}
							//else
							//	qInfo() << "File" << target_file.c_str() << "Error to delete." << endl;


						}
						//if (jump_size == 0)
						jump_size = default_sector_size;
						start_offset = header_offset + jump_size;

					}
					else
					{
						qInfo() << "Error to create target file." << QString::fromStdWString(target_file);
						qInfo() << "Exit.";
						break;
					}



				}
				else
				{
					qInfo() << "Not specified for " << QString::fromStdString(file_struct->getName()) << "continue search for other signatures." << Qt::endl;
					start_offset += default_sector_size;
				}
				if (raw_algorithm)
					delete raw_algorithm;



			}


		}
		

			qInfo() << "Finished.";
	}
	else
		qInfo() << "Wrong params";
	return a.exec();
}
