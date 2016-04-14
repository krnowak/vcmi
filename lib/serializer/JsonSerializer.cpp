/*
 * JsonSerializer.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */


#include "StdInc.h"
#include "JsonSerializer.h"

#include "../JsonNode.h"

JsonSerializer::JsonSerializer(JsonNode & root_):
	JsonSerializeFormat(root_, true)
{

}

void JsonSerializer::serializeBool(const std::string & fieldName, bool & value)
{
	if(value)
		current->operator[](fieldName).Bool() = true;
}

void JsonSerializer::serializeBool(const std::string & fieldName, boost::logic::tribool & value)
{
	if(!boost::logic::indeterminate(value))
		current->operator[](fieldName).Bool() = value;
}

void JsonSerializer::serializeEnum(const std::string & fieldName, const std::string & trueValue, const std::string & falseValue, bool & value)
{
	current->operator[](fieldName).String() = value ? trueValue : falseValue;
}

void JsonSerializer::serializeFloat(const std::string & fieldName, double & value, const double & defaultValue)
{
	if(defaultValue != value)
		current->operator[](fieldName).Float() = value;
}

void JsonSerializer::serializeFloat(const std::string & fieldName, double & value)
{
	current->operator[](fieldName).Float() = value;
}

void JsonSerializer::serializeIntEnum(const std::string & fieldName, const std::vector<std::string> & enumMap, const si32 defaultValue, si32 & value)
{
	if(defaultValue != value)
		current->operator[](fieldName).String() = enumMap.at(value);
}

void JsonSerializer::serializeIntId(const std::string & fieldName, const TDecoder & decoder, const TEncoder & encoder, const si32 defaultValue, si32 & value)
{
	if(defaultValue != value)
	{
		std::string identifier = encoder(value);
		serializeString(fieldName, identifier);
	}
}

void JsonSerializer::serializeLIC(const std::string & fieldName, const TDecoder & decoder, const TEncoder & encoder, const std::vector<bool> & standard, std::vector<bool> & value)
{
	assert(standard.size() == value.size());
	if(standard == value)
		return;

	writeLICPart(fieldName, "anyOf", encoder, value);
}

void JsonSerializer::serializeLIC(const std::string & fieldName, LIC & value)
{
	if(value.any != value.standard)
	{
		writeLICPart(fieldName, "anyOf", value.encoder, value.any);
	}

	writeLICPart(fieldName, "allOf", value.encoder, value.all);
	writeLICPart(fieldName, "noneOf", value.encoder, value.none);
}

void JsonSerializer::serializeLIC(const std::string & fieldName, LICSet & value)
{
	if(value.any != value.standard)
	{
		writeLICPart(fieldName, "anyOf", value.encoder, value.any);
	}

	writeLICPart(fieldName, "allOf", value.encoder, value.all);
	writeLICPart(fieldName, "noneOf", value.encoder, value.none);
}


void JsonSerializer::serializeString(const std::string & fieldName, std::string & value)
{
	if(value != "")
		current->operator[](fieldName).String() = value;
}

void JsonSerializer::writeLICPart(const std::string & fieldName, const std::string & partName, const TEncoder & encoder, const std::vector<bool> & data)
{
	std::vector<std::string> buf;
	buf.reserve(data.size());

	for(si32 idx = 0; idx < data.size(); idx++)
	{
		if(data[idx])
		{
			buf.push_back(encoder(idx));
		}
	}

	std::sort(buf.begin(), buf.end());

	auto & target = current->operator[](fieldName)[partName].Vector();

	for(auto & s : buf)
	{
		JsonNode val(JsonNode::DATA_STRING);
		val.String() = s;
		target.push_back(std::move(val));
	}
}

void JsonSerializer::writeLICPart(const std::string & fieldName, const std::string & partName, const TEncoder & encoder, const std::set<si32> & data)
{
	auto & target = current->operator[](fieldName)[partName].Vector();

	for(const si32 item : data)
	{
		JsonNode val(JsonNode::DATA_STRING);
		val.String() = encoder(item);
		target.push_back(std::move(val));
	}
}

