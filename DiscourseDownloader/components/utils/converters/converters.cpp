#include "components/utils/converters/converters.h"

#include <regex>
#include <Windows.h>
#include <codecvt>

#include "components/utils/string/string.h"
#include "components/diagnostics/logger/logger.h"

// Convert a string to a bool
bool DDL::Converters::StringToBool(std::string string)
{
	if (string == "true" || string == "yes" || string == "t" || string == "y" || string == "1" || string == "on")
	{
		return true;
	}
	else if (string == "false" || string == "no" || string == "f" || string == "n" || string == "0" || string == "off")
	{
		return false;
	}
	else
	{
		DDL::Logger::LogEvent("### WARNING attempted to convert string to boolean with unexpected string value '" + string + "', setting to false. this may cause unintended side-effects", DDLLogLevel::Warning);
		return false;
	}
}

// Converts a string with a hexadecimal byte code to its char equivalent.
bool DDL::Converters::HexStringToChar(std::string hex, char* character)
{
	std::string hex_trim = hex;

	//remove prefix
	if (DDL::Utils::String::StartsWith(hex, "0x", true))
	{
		hex_trim = DDL::Utils::String::Replace(hex, "0x", "");
	}

	if (hex_trim.length() == 2)
	{
		if (hex == "0x3F")
		{
			int e = 0;
		}

		const char* byte_text_cstr = hex_trim.c_str();

		char byte = (char)strtol(byte_text_cstr, NULL, 16);

		std::string byte_str = std::string(1, byte);

		//character = &byte_str[0];
		memcpy(character, byte_str.c_str(), 1);

		//ok
		return true;
	}
	else
	{
		//invalid length
		return false;
	}
}

float DDL::Converters::StringToFloat(std::string string)
{
	float float_value = 0.0f;

	bool contains_letter = std::regex_match(string, std::regex("[^0-9.]"));

	if (!contains_letter)
	{
		float_value = stof(string);
	}
	else
	{
		DDL::Logger::LogEvent("### WARNING attempted to convert string to float with unexpected string value '" + string + "', setting to 0. this may cause unintended side-effects", DDLLogLevel::Warning);
	}
	
	return float_value;
}

int DDL::Converters::StringToInt(std::string string)
{
	int int_value = 0;

	bool contains_letter = std::regex_match(string, std::regex("[^0-9]"));

	if (!contains_letter)
	{
		int_value = stoi(string);
	}
	else
	{
		DDL::Logger::LogEvent("### WARNING attempted to convert string to int with unexpected string value '" + string + "', setting to 0. this may cause unintended side-effects", DDLLogLevel::Warning);
	}

	return int_value;
}

int64_t DDL::Converters::StringToInt64(std::string string)
{
	int64_t int64_value = 0;

	bool contains_letter = std::regex_match(string, std::regex("[^0-9]"));

	if (!contains_letter)
	{
		int64_value = stoll(string);
	}
	else
	{
		DDL::Logger::LogEvent("### WARNING attempted to convert string to int64 with unexpected string value '" + string + "', setting to 0. this may cause unintended side-effects", DDLLogLevel::Warning);
	}

	return int64_value;
}

double DDL::Converters::StringToDouble(std::string string)
{
	double double_value = 0;

	bool contains_letter = std::regex_match(string, std::regex("[^0-9.]"));

	if (!contains_letter)
	{
		double_value = stod(string);
	}
	else
	{
		DDL::Logger::LogEvent("### WARNING attempted to convert string to double with unexpected string value '" + string + "', setting to 0. this may cause unintended side-effects", DDLLogLevel::Warning);
	}

	return double_value;
}

bool DDL::Converters::IsStringInt(std::string string)
{
	bool contains_letter = std::regex_match(string, std::regex("[^0-9]"));

	if (contains_letter)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool DDL::Converters::IsStringFloat(std::string string)
{
	bool contains_letter = std::regex_match(string, std::regex("[^0-9.]"));

	if (contains_letter)
	{
		return false;
	}
	else
	{
		return true;
	}
}

std::string DDL::Converters::BoolToString(bool value)
{
	if (value)
	{
		return "true";
	}

	return "false";
}

std::string DDL::Converters::FloatToString(float value, int max_decimals)
{
	std::string float_str = std::to_string(value);
	
	if (max_decimals == 0)
	{
		float_str = float_str.substr(0, float_str.find('.') + 1);
	}
	else if (max_decimals > 0)
	{
		if (DDL::Utils::String::ContainsChar(float_str, '.'))
		{
			int end_index = float_str.find('.') + max_decimals + 1;

			if (end_index < float_str.size())
			{
				float_str = float_str.substr(0, end_index);
			}
			else
			{
				int decimals_to_add = end_index - float_str.size();

				for (int i = 0; i < decimals_to_add; i++)
				{
					float_str += "0";
				}
			}
		}
	}

	return float_str;
}