#include "config.h"

#include <regex>
#include <Windows.h>

#include "components/utils/string/string.h"
#include "components/diagnostics/logger/logger.h"
#include "components/utils/io/io.h"

void BlamConfigurationSetting::ParseValue()
{
	switch (type)
	{
		case BlamConfigurationSettingType::String:
		{
			valid = true;
			break;
		}
		case BlamConfigurationSettingType::Boolean:
		{
			std::string setting_value_lower = DDL::Utils::String::ToLower(value);

			if (setting_value_lower == "true" || setting_value_lower == "t" || setting_value_lower == "1" || setting_value_lower == "yes" || setting_value_lower == "y")
			{
				value_bool = true;
				valid = true;
			}
			else if (setting_value_lower == "false" || setting_value_lower == "f" || setting_value_lower == "0" || setting_value_lower == "no" || setting_value_lower == "n")
			{
				value_bool = false;
				valid = true;
			}
			else
			{
				DDL::Logger::LogEvent("configuration setting '" + id + "' value could not be converted to a boolean: " + value, DDLLogLevel::Error);
				valid = false;
			}

			break;
		}
		case BlamConfigurationSettingType::Color:
		{
			value_color = BlamColor(255, 255, 255, 255);

			if (value_color.FromString(value))
			{
				valid = true;
			}
			else
			{
				DDL::Logger::LogEvent("configuration setting '" + id + "' value could not be converted to a color: " + value, DDLLogLevel::Error);
				valid = false;
			}

			break;
		}
		case BlamConfigurationSettingType::Int:
		{
			bool contains_letter = std::regex_match(value, std::regex("[^0-9]"));

			if (!contains_letter)
			{
				value_int = stoi(value);
				valid = true;
			}
			else
			{
				DDL::Logger::LogEvent("configuration setting '" + id + "' value could not be converted to an int: " + value, DDLLogLevel::Error);
				valid = false;
			}

			break;
		}
		case BlamConfigurationSettingType::Float:
		{
			bool contains_letter = std::regex_match(value, std::regex("[^0-9.]"));

			if (!contains_letter)
			{
				value_float = stof(value);
				valid = true;
			}
			else
			{
				DDL::Logger::LogEvent("configuration setting '" + id + "' value could not be converted to a float: " + value, DDLLogLevel::Error);
				valid = false;
			}

			break;
		}
		case BlamConfigurationSettingType::Comment:
		{
			if (CONFIG_DEBUG)
			{
				DDL::Logger::LogEvent("configuration setting '" + id + "' appears to be a comment but tried to load as a setting, this should be impossible", DDLLogLevel::Error);
			}
			
			valid = false;
			break;
		}
		default:
		{
			DDL::Logger::LogEvent("configuration setting '" + id + "' appears to use a type which exists, but does not have a parser case for it - notify a developer or submit a bug report", DDLLogLevel::Error);
			valid = false;
			break;
		}
	}
}

BlamConfigurationSetting::BlamConfigurationSetting(std::string line, int _line_number, std::string _comment_delimeter, std::string _filename)
{
	filename = _filename;
	comment_delimeter = _comment_delimeter;
	line_number = _line_number;

	if (line.starts_with(comment_delimeter))
	{
		type = BlamConfigurationSettingType::Comment;
		value = line.substr(1);

		id = "comment_" + std::to_string(line_number);

		valid = true;
	}
	else
	{
		if (DDL::Utils::String::ContainsChar(line, '='))
		{
			std::vector<std::string> line_elements = DDL::Utils::String::Split(line, "=");

			id = line_elements.at(0);
			value = line_elements.at(1);

			if (id.starts_with("s:"))
			{
				type = BlamConfigurationSettingType::String;
				id = DDL::Utils::String::Split(id, ":").at(1);
			}
			else if (id.starts_with("b:"))
			{
				type = BlamConfigurationSettingType::Boolean;
				id = DDL::Utils::String::Split(id, ":").at(1);
			}
			else if (id.starts_with("f:"))
			{
				type = BlamConfigurationSettingType::Float;
				id = DDL::Utils::String::Split(id, ":").at(1);
			}
			else if (id.starts_with("i:"))
			{
				type = BlamConfigurationSettingType::Int;
				id = DDL::Utils::String::Split(id, ":").at(1);
			}
			else if (id.starts_with("c:"))
			{
				type = BlamConfigurationSettingType::Color;
				id = DDL::Utils::String::Split(id, ":").at(1);
			}
			else
			{
				if (DDL::Utils::String::ContainsChar(id, ':'))
				{
					std::vector<std::string> setting_name_elements = DDL::Utils::String::Split(id, ":");

					std::string setting_type_specifier = setting_name_elements.at(0);
					id = setting_name_elements.at(1);

					DDL::Logger::LogEvent("configuration setting '" + id + "' uses an invalid type specifier '" + setting_type_specifier + "', will be treated as string", DDLLogLevel::Warning);
					type = BlamConfigurationSettingType::String;
				}
				else
				{
					DDL::Logger::LogEvent("configuration setting '" + id + "' does not specify its type, will be treated as string", DDLLogLevel::Warning);
					type = BlamConfigurationSettingType::String;
				}
			}

			ParseValue();
		}
		else
		{
			valid = false;
		}
	}
}

bool BlamConfigurationSetting::IsValid()
{
	return valid;
}

std::string* BlamConfigurationSetting::AsString()
{
	if (type != BlamConfigurationSettingType::String)
	{
		if (CONFIG_DEBUG)
		{
			DDL::Logger::LogEvent("accessing non-string configuration setting '"
				+ id + "' as string, consider accessing as the proper type", DDLLogLevel::Warning);
		}
	}

	return &value;
}

bool* BlamConfigurationSetting::AsBool()
{
	if (type == BlamConfigurationSettingType::Boolean)
	{
		return &value_bool;
	}
	else
	{
		DDL::Logger::LogEvent("tried to access non-boolean configuration setting '" + id + "' as bool, this will return nullptr and a fatal error is possible", DDLLogLevel::Error);
		return nullptr;
	}
}

float* BlamConfigurationSetting::AsFloat()
{
	if (type == BlamConfigurationSettingType::Float)
	{
		return &value_float;
	}
	else
	{
		DDL::Logger::LogEvent("tried to access non-float configuration setting '" + id + "' as float, this will return nullptr and a fatal error is possible", DDLLogLevel::Error);
		return nullptr;
	}
}

int* BlamConfigurationSetting::AsInt()
{
	if (type == BlamConfigurationSettingType::Int)
	{
		return &value_int;
	}
	else
	{
		DDL::Logger::LogEvent("tried to access non-int configuration setting '" + id + "' as int, this will return nullptr and a fatal error is possible", DDLLogLevel::Error);
		return nullptr;
	}
}

BlamColor* BlamConfigurationSetting::AsColor()
{
	if (type == BlamConfigurationSettingType::Color)
	{
		return &value_color;
	}
	else
	{
		DDL::Logger::LogEvent("tried to access non-color configuration setting '" + id + "' as color, this will return nullptr and a fatal error is possible", DDLLogLevel::Error);
		return nullptr;
	}
}

std::string BlamConfigurationSetting::ConvertRawValueToString()
{
	switch (type)
	{
		case BlamConfigurationSettingType::Boolean:
		{
			if (value_bool)
			{
				return "true";
			}
			else
			{
				return "false";
			}
		}
		case BlamConfigurationSettingType::Color:
		{
			return value_color.ToString();
		}
		case BlamConfigurationSettingType::String:
		{
			return value;
		}
		case BlamConfigurationSettingType::Int:
		{
			return std::to_string(value_int);
		}
		case BlamConfigurationSettingType::Float:
		{
			return std::to_string(value_float);
		}
		case BlamConfigurationSettingType::Comment:
		{
			return value;
		}
		default:
		{
			DDL::Logger::LogEvent("configuration setting '" + id + "' in file '" + filename + "' uses a type which does not have a conversion case specified, notify a developer or submit a bug report", DDLLogLevel::Warning);
			return value;
		}
	}

	return value;
}

void BlamConfigurationSetting::UpdateValue(std::string new_value)
{
	value = new_value;
	ParseValue();
}

std::string* BlamConfigurationSetting::GetRawValue()
{
	return &value;
}

BlamConfigurationSetting* BlamConfigurationSetting::Copy()
{
	BlamConfigurationSetting* setting_copy = new BlamConfigurationSetting(CreateConfigFileLine(), 0, comment_delimeter, filename);
	return setting_copy;
}

std::string BlamConfigurationSetting::CreateConfigFileLine()
{
	std::string setting_line = "";

	if (type == BlamConfigurationSettingType::Comment)
	{
		setting_line += comment_delimeter + *AsString();
	}
	else
	{
		switch (type)
		{
			case BlamConfigurationSettingType::Boolean:
			{
				setting_line += "b:";
				break;
			}
			case BlamConfigurationSettingType::Color:
			{
				setting_line += "c:";
				break;
			}
			case BlamConfigurationSettingType::Float:
			{
				setting_line += "f:";
				break;
			}
			case BlamConfigurationSettingType::Int:
			{
				setting_line += "i:";
				break;
			}
			case BlamConfigurationSettingType::String:
			{
				setting_line += "s:";
				break;
			}
			default:
			{
				break;
			}
		}

		setting_line += id + "=" + ConvertRawValueToString();
	}

	return setting_line;
}