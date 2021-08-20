#pragma once

#ifndef JSONREADER_H
#define JSONREADER_H

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

void ReadHadersOffset(const QJsonArray& json_array, ArrayOfHeader& header_array);

void ReadFooter(const QJsonObject footer_object, SignatureHandle& footer);


void ReadFooters(const QJsonArray json_array, ArrayOfFooter& footer_array);



void ReadJsonFIle(const QByteArray& byte_data, QList<JsonFileStruct>& parsedResult);
void ReadJsonFile(const QString& jsonFileName, QList<JsonFileStruct>& parsedResult);
IO::DataArray JsonToDataArray(SignatureHandle signHandle);


RAW::FileStruct* toRAWFileStruct(const JsonFileStruct& jsonFileStruct);


std::unique_ptr<RAW::FileStruct> toUniqueFileStruct(const JsonFileStruct& jsonFileStruct);

std::shared_ptr<RAW::FileStruct> toSharedFileStruct(const JsonFileStruct& jsonFileStruct);


#endif