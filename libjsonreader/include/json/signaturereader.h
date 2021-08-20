#pragma once

#include "io/finder.h"
#include "JsonReader.h"


struct JsonExtensions
{
	QString extensionName;
	QList<QString> listFormatName;
};

class AbstractJsonReader
{
public:
	virtual void readJsonFile(const IO::path_string& jsonFilePath) = 0;
};

inline void loadAllJsonFiles(AbstractJsonReader & abstractJsonReader , const IO::path_string& folder, const IO::path_string& extension)
{
	IO::Finder finder;
	finder.add_extension(extension);
	finder.FindFiles(folder);
	auto listFiles = finder.getFiles();
	for (auto signFile : listFiles)
		abstractJsonReader.readJsonFile(signFile);
}

inline QByteArray readByteArrayFromFile(const IO::path_string & filenaem)
{
	QString json_file = QString::fromStdWString(filenaem);
	QFile file(json_file);
	if (!file.open(QIODevice::ReadOnly))
	{
		//qInfo() << "Error to open file. \"" << file.fileName() << "\"";
		return QByteArray();
	}
	return file.readAll();
}
class SignatureReader
	: public AbstractJsonReader
{
	QList<JsonFileStruct> listSignatures_;
	//QList<JsonExtensions> listExtensions_;

public:
	void loadAllSignatures(const IO::path_string& signaturesFolder , const IO::path_string & extension)
	{
		loadAllJsonFiles(*this, signaturesFolder, extension);
	}
	void readJsonFile(const IO::path_string& signaturesFile) override
	{
		QList<JsonFileStruct> listFileStruct;

		auto json_str = readByteArrayFromFile(signaturesFile);
		ReadJsonFIle(json_str, listFileStruct);
		if (listFileStruct.empty())
		{
			qInfo() << "Error to read" << QString::fromStdWString(signaturesFile) << "file. Wrong syntax.";
			return;
		}

		for (auto theFileStruct : listFileStruct)
			listSignatures_.append(theFileStruct);
	}

	QList<JsonFileStruct> getAllSignatures() const
	{
		return listSignatures_;
	}


};

class ExtensionReader
	: public AbstractJsonReader
{
	QList<JsonExtensions> listExtensions_;
public:
	void loadAllExtensions(const IO::path_string& extensionsFolder, const IO::path_string& extension)
	{
		loadAllJsonFiles(*this, extensionsFolder, extension);
	}

	void readJsonFile(const IO::path_string& extensionFilePath) override
	{
		auto byte_data = readByteArrayFromFile(extensionFilePath);

		QJsonDocument json_doc = QJsonDocument::fromJson(byte_data);
		if (json_doc.isNull())
		{
			qInfo() << "Error to read" << QString::fromStdWString(extensionFilePath) << "file. Wrong syntax.";
			return;
		}
		auto root = json_doc.object();
		auto extensions = root.keys();

		for (auto ext : extensions)
		{
			JsonExtensions extension;
			extension.extensionName = ext;
			auto object_value = root.value(ext);
			if (object_value.isArray())
			{
				auto formatNameArray = object_value.toArray();
				for (auto formatName : formatNameArray)
				{
					extension.listFormatName.append(formatName.toString());
					std::cout << formatName.toString().toStdString() << std::endl;

				}
			}

			listExtensions_.append(extension);
		}
	}
	QList<JsonExtensions> getAllSignatures() const
	{
		return listExtensions_;
	}
};
