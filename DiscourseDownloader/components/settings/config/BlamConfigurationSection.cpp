#include "config.h"

#include "components/diagnostics/logger/logger.h"
#include "components/utils/io/io.h"
#include "components/utils/string/string.h"

BlamConfigurationSection::BlamConfigurationSection(std::string _name, std::string _filename)
{
	name = _name;
	filename = _filename;
	settings = std::map<std::string, BlamConfigurationSetting*>();
}

BlamConfigurationSection::~BlamConfigurationSection()
{
	std::map<std::string, BlamConfigurationSetting*> orig_settings = settings;
	std::map<std::string, BlamConfigurationSetting*> orig_default_settings = default_settings;

	settings.clear();
	default_settings.clear();

	for (std::pair<std::string, BlamConfigurationSetting*> setting_pair: orig_settings)
	{
		delete setting_pair.second;
	}

	for (std::pair<std::string, BlamConfigurationSetting*> setting_pair : orig_default_settings)
	{
		delete setting_pair.second;
	}

	orig_settings.clear();
	orig_default_settings.clear();
}

void BlamConfigurationSection::AddNewSetting(BlamConfigurationSetting* new_setting)
{
	if (!settings.contains(new_setting->id))
	{
		settings.insert(std::pair<std::string, BlamConfigurationSetting*>(new_setting->id, new_setting));
		//DDL::Logger::LogEvent("added new configuration setting '" + new_setting->id + "' in file '" + filename + "'");
	}
	else
	{
		if (CONFIG_DEBUG)
		{
			DDL::Logger::LogEvent("tried to add duplicate configuration setting '" + new_setting->id + "' to section " + name + " in file '" + filename
				+ "', value will be updated - this may or may not be what you want", DDLLogLevel::Info);
		}

		BlamConfigurationSetting* old_setting = settings.at(new_setting->id);

		if (old_setting->type == new_setting->type)
		{
			if (CONFIG_DEBUG)
			{
				DDL::Logger::LogEvent("configuration setting '" + new_setting->id + "' in section " + name + " in file '" + filename + "' updated from '"
					+ *old_setting->GetRawValue() + "' to '" + *new_setting->GetRawValue() + "'", DDLLogLevel::Warning);
			}
			
			old_setting->UpdateValue(*new_setting->GetRawValue());
		}
		else
		{
			if (CONFIG_DEBUG)
			{
				DDL::Logger::LogEvent("tried to update configuration setting '" + new_setting->id + "' in section " + name + " in file '" + filename
					+ "', but setting types do not match - value will NOT be updated", DDLLogLevel::Warning);
			}
		}
	}
}

void BlamConfigurationSection::AddNewDefaultSetting(BlamConfigurationSetting* new_setting)
{
	if (!default_settings.contains(new_setting->id))
	{
		default_settings.insert(std::pair<std::string, BlamConfigurationSetting*>(new_setting->id, new_setting));
		//DDL::Logger::LogEvent("added new default configuration setting '" + new_setting->id + "' in file '" + filename + "'");
	}
	else
	{
		if (CONFIG_DEBUG)
		{
			DDL::Logger::LogEvent("tried to add duplicate default configuration setting '" + new_setting->id + "' to section " + name + " in file '" + filename + "', setting will NOT be added", DDLLogLevel::Warning);
		}
	}
}

bool BlamConfigurationSection::HasOption(std::string option)
{
	if (settings.contains(option))
	{
		return true;
	}
	else
	{
		if (default_settings.contains(option))
		{
			if (CONFIG_DEBUG)
			{
				DDL::Logger::LogEvent("default setting '" + option + "' does not exist in configuration file, adding from default");
			}

			BlamConfigurationSetting* new_setting = default_settings.at(option)->Copy();
			AddNewSetting(new_setting);
			return true;
		}
	}

	return false;
}

std::string* BlamConfigurationSection::GetString(std::string option)
{
	if (settings.contains(option))
	{
		return settings.at(option)->AsString();
	}
	else
	{
		if (default_settings.contains(option))
		{
			DDL::Logger::LogEvent("default setting '" + option + "' does not exist in configuration file, adding from default");

			BlamConfigurationSetting* new_setting = default_settings.at(option)->Copy();
			AddNewSetting(new_setting);

			return new_setting->AsString();
		}
	}

	DDL::Logger::LogEvent("configuration setting '" + option + "' does not exist in file and has no known default, creating new option with value 'unspecified' - this could cause undesired behavior", DDLLogLevel::Warning);

	BlamConfigurationSetting* new_setting = new BlamConfigurationSetting("s:" + option + "=unspecified", 0, ";", filename);
	AddNewSetting(new_setting);

	return new_setting->AsString();
}

bool* BlamConfigurationSection::GetBool(std::string option)
{
	if (settings.contains(option))
	{
		return settings.at(option)->AsBool();
	}
	else
	{
		if (default_settings.contains(option))
		{
			DDL::Logger::LogEvent("default setting '" + option + "' does not exist in configuration file, adding from default");
			
			BlamConfigurationSetting* new_setting = default_settings.at(option)->Copy();
			AddNewSetting(new_setting);

			return new_setting->AsBool();
		}
	}

	DDL::Logger::LogEvent("configuration setting '" + option + "' does not exist in file and has no known default, creating new option with value 'false' - this could cause undesired behavior", DDLLogLevel::Warning);

	BlamConfigurationSetting* new_setting = new BlamConfigurationSetting("b:" + option + "=false", 0, ";", filename);
	AddNewSetting(new_setting);

	return new_setting->AsBool();
}

float* BlamConfigurationSection::GetFloat(std::string option)
{
	if (settings.contains(option))
	{
		return settings.at(option)->AsFloat();
	}
	else
	{
		if (default_settings.contains(option))
		{
			DDL::Logger::LogEvent("default setting '" + option + "' does not exist in configuration file, adding from default");
			
			BlamConfigurationSetting* new_setting = default_settings.at(option)->Copy();
			AddNewSetting(new_setting);

			return new_setting->AsFloat();
		}
	}

	DDL::Logger::LogEvent("configuration setting '" + option + "' does not exist in file and has no known default, creating new option with value '0.0f' - this could cause undesired behavior", DDLLogLevel::Warning);

	BlamConfigurationSetting* new_setting = new BlamConfigurationSetting("f:" + option + "=0.0f", 0, ";", filename);
	AddNewSetting(new_setting);

	return new_setting->AsFloat();
}

int* BlamConfigurationSection::GetInt(std::string option)
{
	if (settings.contains(option))
	{
		return settings.at(option)->AsInt();
	}
	else
	{
		if (default_settings.contains(option))
		{
			DDL::Logger::LogEvent("default setting '" + option + "' does not exist in configuration file, adding from default");
			
			BlamConfigurationSetting* new_setting = default_settings.at(option)->Copy();
			AddNewSetting(new_setting);

			return new_setting->AsInt();
		}
	}

	DDL::Logger::LogEvent("configuration setting '" + option + "' does not exist in file and has no known default, creating new option with value '0' - this could cause undesired behavior", DDLLogLevel::Warning);

	BlamConfigurationSetting* new_setting = new BlamConfigurationSetting("i:" + option + "=0", 0, ";", filename);
	AddNewSetting(new_setting);

	return new_setting->AsInt();
}

BlamColor* BlamConfigurationSection::GetColor(std::string option)
{
	if (settings.contains(option))
	{
		return settings.at(option)->AsColor();
	}
	else
	{
		if (default_settings.contains(option))
		{
			DDL::Logger::LogEvent("default setting '" + option + "' does not exist in configuration file, adding from default");
			
			BlamConfigurationSetting* new_setting = default_settings.at(option)->Copy();
			AddNewSetting(new_setting);

			return new_setting->AsColor();
		}
	}

	DDL::Logger::LogEvent("configuration setting '" + option + "' does not exist in file and has no known default, creating new option with value '0.0f' - this could cause undesired behavior", DDLLogLevel::Warning);

	BlamConfigurationSetting* new_setting = new BlamConfigurationSetting("c:" + option + "=#B000B5", 0, ";", filename);
	AddNewSetting(new_setting);

	return new_setting->AsColor();
}

void BlamConfigurationSection::RestoreDefaultSettings()
{
	// Remove original settings
	for (std::pair<std::string, BlamConfigurationSetting*> setting_pair : settings)
	{
		delete setting_pair.second;
	}

	settings.clear();

	// Copy over defaults
	for (std::pair<std::string, BlamConfigurationSetting*> setting_pair : default_settings)
	{
		AddNewSetting(setting_pair.second->Copy());
	}
}