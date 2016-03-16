/*
 * JsonSerializeFormat.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */


#include "StdInc.h"
#include "JsonSerializeFormat.h"

#include "../JsonNode.h"

JsonSerializeHelper::JsonSerializeHelper(JsonSerializeHelper && other):
	owner(other.owner),
	thisNode(other.thisNode),
	restoreState(false),
	parentNode(other.parentNode)
{
	std::swap(restoreState, other.restoreState);
}

JsonSerializeHelper::~JsonSerializeHelper()
{
	if(restoreState)
		owner.current = parentNode;
}

JsonNode & JsonSerializeHelper::get()
{
	return * thisNode;
}

JsonSerializeFormat * JsonSerializeHelper::operator->()
{
	return &owner;
}

JsonSerializeHelper::JsonSerializeHelper(JsonSerializeFormat & owner_, JsonNode * thisNode_):
	owner(owner_),
	thisNode(thisNode_),
	restoreState(true),
	parentNode(owner.current)
{
	owner.current = thisNode;
}

JsonSerializeHelper::JsonSerializeHelper(JsonSerializeHelper & parent, const std::string & fieldName):
	owner(parent.owner),
	thisNode(&(parent.thisNode->operator[](fieldName))),
	restoreState(true),
	parentNode(parent.thisNode)
{
	owner.current = thisNode;
}

JsonStructSerializer JsonSerializeHelper::enterStruct(const std::string & fieldName)
{
	return JsonStructSerializer(*this, fieldName);
}

//JsonStructSerializer
JsonStructSerializer::JsonStructSerializer(JsonStructSerializer && other):
	JsonSerializeHelper(std::move(static_cast<JsonSerializeHelper &>(other)))
{

}

JsonStructSerializer::JsonStructSerializer(JsonSerializeFormat & owner_, const std::string & fieldName):
	JsonSerializeHelper(owner_, &(owner_.current->operator[](fieldName)))
{

}

JsonStructSerializer::JsonStructSerializer(JsonSerializeHelper & parent, const std::string & fieldName):
	JsonSerializeHelper(parent, fieldName)
{

}

//JsonSerializeFormat::LIC
JsonSerializeFormat::LIC::LIC(const std::vector<bool> & Standard, const TDecoder & Decoder, const TEncoder & Encoder):
	standard(Standard), decoder(Decoder), encoder(Encoder)
{
	any.resize(standard.size(), false);
	all.resize(standard.size(), false);
	none.resize(standard.size(), false);
}


JsonSerializeFormat::LICSet::LICSet(const std::set<si32>& Standard, const TDecoder & Decoder, const TEncoder & Encoder):
	standard(Standard), decoder(Decoder), encoder(Encoder)
{

}


//JsonSerializeFormat
JsonSerializeFormat::JsonSerializeFormat(JsonNode & root_, const bool saving_):
	saving(saving_),
	root(&root_),
	current(root)
{

}

JsonStructSerializer JsonSerializeFormat::enterStruct(const std::string & fieldName)
{
	JsonStructSerializer res(*this, fieldName);

	return res;
}
