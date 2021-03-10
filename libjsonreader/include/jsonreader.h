#pragma once

#include <QtCore>
#include <QVariant>
#include <QDebug>
#include <QList>
#include <QFile>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

#include "raw\abstractraw.h"

using namespace IO;

const QString algorithmName_txt = "algorithmName";
const QString algorithType_txt = "algorithType";
const QString category_txt = "category";

const QString header_txt = "header";
const QString offset_txt = "offset";
const QString footer_txt = "footer";
const QString tailsize_txt = "tailsize";
const QString textdata_txt = "textdata";
const QString hexdata_txt = "hexdata";

const QString maxfilesize_txt = "maxfilesize";
const QString minfilesize_txt = "minfilesize";
const QString extension_txt = "extension";

const QString equally_to_txt = "equally_to";

const QString search_block_txt = "search_block";


struct SignatureHandle
{
	QString value_string;
	int value_int = 0;
	int offset = 0;
	int search_block = 0;
	bool bHex = false;
};



using ArrayOfHeader = QVector<SignatureHandle>;
using ArrayOfFooter = QVector<SignatureHandle>;


struct JsonFileStruct
{
	QString name;
	QString algorithmName;
	QString algorithType;
	QString category;
	ArrayOfHeader headers;
	ArrayOfFooter footer;
	qlonglong maxfilesize = 0;
	qlonglong minfilesize = 0;
	QString extension;
};

void ReadHadersOffset(const QJsonArray & json_array, ArrayOfHeader &header_array)
{
	for (int i = 0; i < json_array.size(); ++i)
	{
		auto theHeader = json_array.at(i);
		SignatureHandle headerOffset;
		if (theHeader.isObject())
		{
			auto text_value = theHeader.toObject().value(textdata_txt);
			if (text_value.isUndefined())
			{
				text_value = theHeader.toObject().value(hexdata_txt);
				if (text_value.isUndefined())
					return;
				headerOffset.bHex = true;
			}
			headerOffset.value_string = text_value.toString();

			auto offset_value = theHeader.toObject().value(offset_txt);
			if (!offset_value.isUndefined())
				headerOffset.value_int = offset_value.toInt();

			header_array.append(headerOffset);
		}
	}

}

void ReadFooter(const QJsonObject footer_object, SignatureHandle& footer)
{
	auto text_value = footer_object.value(hexdata_txt);
	if (!text_value.isUndefined())
	{
		footer.bHex = true;
		footer.value_string = text_value.toString();
	}
	else
	{
		text_value = footer_object.value(textdata_txt);
		if (text_value.isUndefined())
			return;
		footer.value_string = text_value.toString();
	}

	auto tail_size = footer_object.value(tailsize_txt);
	if (!tail_size.isUndefined())
		footer.value_int = tail_size.toInt();

	auto offset_value = footer_object.value(offset_txt);
	if (!offset_value.isUndefined())
		footer.offset = offset_value.toInt();

	auto search_block_value = footer_object.value(search_block_txt);
	if (!search_block_value.isUndefined())
		footer.search_block = search_block_value.toInt();

}


void ReadFooters(const QJsonArray json_array, ArrayOfFooter & footer_array)
{
	for (int i = 0; i < json_array.size(); ++i)
	{
		auto theFooter = json_array.at(i);
		SignatureHandle footerData;
		if (theFooter.isObject())
		{
			ReadFooter(theFooter.toObject(), footerData);
			footer_array.append(footerData);
		}

	}

}



void ReadJsonFIle(const QByteArray & byte_data, QList<JsonFileStruct> & parsedResult)
{
	QJsonDocument json_doc = QJsonDocument::fromJson(byte_data);
	if (json_doc.isNull())
	{
		qInfo() << "Error to parse json file.";
		return;
	}

	auto root = json_doc.object();
	auto signatureKeys = root.keys();

	for (auto signature_name : signatureKeys)
	{
		JsonFileStruct jsonFileStruct;
		jsonFileStruct.name = signature_name;
		qInfo() << "name = " << signature_name << endl;


		//	QVariant
		auto object_value = root.value(signature_name);
		if (object_value.isObject())
		{
			auto json_object = object_value.toObject();
			auto algorithm_name = json_object.value(algorithmName_txt);
			jsonFileStruct.algorithmName = algorithm_name.toString();

			auto category_name = json_object.value(category_txt);
			jsonFileStruct.category = category_name.toString();

			QJsonValue header_value = json_object.value(header_txt);
			if (header_value.isArray())
			{
				auto array_headers = header_value.toArray();
				ReadHadersOffset(array_headers, jsonFileStruct.headers);
			}

			auto footer_value = json_object.value(footer_txt);
			if (footer_value.isObject())
			{
			SignatureHandle footerData;
			ReadFooter(footer_value.toObject(), footerData);
			jsonFileStruct.footer.append(footerData);
			}
			else if (footer_value.isArray())
			{
			ReadFooters(footer_value.toArray(), jsonFileStruct.footer);
			}

			auto maxsize_value = json_object.value(maxfilesize_txt);
			if (!maxsize_value.isUndefined())
				jsonFileStruct.maxfilesize = maxsize_value.toVariant().toLongLong();

			auto minsize_value = json_object.value(minfilesize_txt);
			if (!minsize_value.isUndefined())
				jsonFileStruct.minfilesize = minsize_value.toVariant().toLongLong();


			auto extension_value = json_object.value(extension_txt);
			if (extension_value.isString())
				jsonFileStruct.extension = extension_value.toString();

			auto algorithType_value = json_object.value(algorithType_txt);
			if (extension_value.isString())
				jsonFileStruct.algorithType = algorithType_value.toString();

			parsedResult.append(jsonFileStruct);

		}
		qInfo() << endl;
	}

}
void ReadJsonFile(const QString& jsonFileName, QList<JsonFileStruct>& parsedResult)
{
	QFile file(jsonFileName);
	file.open(QIODevice::ReadOnly);
	auto byte_data = file.readAll();
	ReadJsonFIle(byte_data, parsedResult);
}
IO::DataArray JsonToDataArray(SignatureHandle signHandle)
{
	IO::DataArray::Ptr data_array = nullptr;
	if (signHandle.bHex)
	{
		if (signHandle.value_string.length() % 2 != 0)
			return IO::DataArray(0);

		auto byte_array = QByteArray::fromHex(signHandle.value_string.toLatin1());
		data_array = IO::makeDataArray(byte_array.size());
		memcpy(data_array->data(), byte_array.data(), data_array->size());

	}
	else
	{
		data_array = IO::makeDataArray(signHandle.value_string.length());
		memcpy(data_array->data(), signHandle.value_string.toStdString().c_str(), data_array->size());
	}
	return *data_array.get();
}


RAW::FileStruct* toRAWFileStruct(const JsonFileStruct& jsonFileStruct)
{
	auto raw_file_struct = new RAW::FileStruct(jsonFileStruct.name.toStdString());
	for (auto theHeader : jsonFileStruct.headers)
	{
		auto data_array = JsonToDataArray(theHeader);
		raw_file_struct->addHeader(data_array, theHeader.value_int);
	}

	if (!jsonFileStruct.footer.isEmpty())
	{
		for (auto& theFooter : jsonFileStruct.footer)
		{
			auto data_array = JsonToDataArray(theFooter);
			raw_file_struct->addFooter(std::move(data_array));
			raw_file_struct->setFooterTailEndSize(theFooter.value_int);
			raw_file_struct->setFooterSearchOffset(theFooter.offset, theFooter.search_block);
		}

	}
	raw_file_struct->setAlgorithmName(jsonFileStruct.algorithmName.toStdString());
	raw_file_struct->setExtension(jsonFileStruct.extension.toStdWString());
	raw_file_struct->setMaxFileSize(jsonFileStruct.maxfilesize);
	raw_file_struct->setMinFileSize(jsonFileStruct.minfilesize);
	raw_file_struct->setAlgorithType(jsonFileStruct.algorithType.toStdString());
	return raw_file_struct;
}


std::unique_ptr<RAW::FileStruct> toUniqueFileStruct(const JsonFileStruct& jsonFileStruct)
{
	auto raw_file_struct = toRAWFileStruct(jsonFileStruct);
	auto file_struct = std::unique_ptr<RAW::FileStruct>(raw_file_struct);
	return file_struct;
}

std::shared_ptr<RAW::FileStruct> toSharedFileStruct(const JsonFileStruct& jsonFileStruct)
{
	auto raw_file_struct = toRAWFileStruct(jsonFileStruct);
	auto file_struct = std::shared_ptr<RAW::FileStruct>(raw_file_struct);
	return file_struct;
}