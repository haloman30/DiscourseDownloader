#include "json.h"

#include <components/3rdparty/rapidjson/stringbuffer.h>
#include <components/3rdparty/rapidjson/writer.h>

#include "components/diagnostics/logger/logger.h"
#include "components/utils/string/string.h"

std::string DDL::Utils::Json::GetValueAsString(rapidjson::Document* document, const char* key)
{
	if (!document->HasMember(key))
	{
		DDL::Logger::LogEvent("Json document does not contain member '" + std::string(key) + "'", DDLLogLevel::Error);
		return "";
	}

	if (!(*document)[key].IsString())
	{
		DDL::Logger::LogEvent("Json document contains member '" + std::string(key) + "', but value was not of expected type string", DDLLogLevel::Error);
		return "";
	}

	return (*document)[key].GetString();
}

int DDL::Utils::Json::GetValueAsInt(rapidjson::Document* document, const char* key)
{
	if (!document->HasMember(key))
	{
		DDL::Logger::LogEvent("Json document does not contain member '" + std::string(key) + "'", DDLLogLevel::Error);
		return 0;
	}

	if (!(*document)[key].IsInt())
	{
		DDL::Logger::LogEvent("Json document contains member '" + std::string(key) + "', but value was not of expected type int", DDLLogLevel::Error);
		return 0;
	}

	return (*document)[key].GetInt();
}

bool DDL::Utils::Json::GetValueAsBool(rapidjson::Document* document, const char* key)
{
	if (!document->HasMember(key))
	{
		DDL::Logger::LogEvent("Json document does not contain member '" + std::string(key) + "'", DDLLogLevel::Error);
		return false;
	}

	if (!(*document)[key].IsBool())
	{
		DDL::Logger::LogEvent("Json document contains member '" + std::string(key) + "', but value was not of expected type bool", DDLLogLevel::Error);
		return false;
	}

	return (*document)[key].GetBool();
}

uint64_t DDL::Utils::Json::GetValueAsUInt64(rapidjson::Document* document, const char* key)
{
	if (!document->HasMember(key))
	{
		DDL::Logger::LogEvent("Json document does not contain member '" + std::string(key) + "'", DDLLogLevel::Error);
		return 0;
	}

	if (!(*document)[key].IsUint64())
	{
		DDL::Logger::LogEvent("Json document contains member '" + std::string(key) + "', but value was not of expected type uint64_t", DDLLogLevel::Error);
		return 0;
	}

	return (*document)[key].GetUint64();
}

std::string DDL::Utils::Json::Serialize(rapidjson::Document* object)
{
	std::string serialized_json = "";

	if (object)
	{
		rapidjson::StringBuffer buffer = rapidjson::StringBuffer();
		rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(buffer);
		object->Accept(writer);

		serialized_json = buffer.GetString();
	}

	return serialized_json;
}

std::string DDL::Utils::Json::Serialize(rapidjson::Value* object)
{
	std::string serialized_json = "";

	if (object)
	{
		rapidjson::StringBuffer buffer = rapidjson::StringBuffer();
		rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(buffer);
		object->Accept(writer);

		serialized_json = buffer.GetString();
	}

	return serialized_json;
}

std::string DDL::Utils::Json::FormatVectorAsJSONArray(std::vector<std::string> list)
{
	std::string json = "{";

	for (int i = 0; i < list.size(); i++)
	{
		json += list.at(i);

		if (i != (list.size() - 1))
		{
			json += ",";
		}
	}

	json += "}";

	return json;
}

std::vector<std::string> DDL::Utils::Json::ParseJSONArray(std::optional<std::string> json)
{
	std::vector<std::string> vector = std::vector<std::string>();

	if (json.has_value())
	{
		std::string json_string = json.value();

		json_string = DDL::Utils::String::Replace(json_string, "{", "");
		json_string = DDL::Utils::String::Replace(json_string, "}", "");

		vector = DDL::Utils::String::Split(json_string, ",");
	}

	return vector;
}