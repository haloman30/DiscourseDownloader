#include "config.h"

#include <fstream>
#include <sstream>

#include "components/utils/io/io.h"
#include "components/utils/string/string.h"
#include "components/diagnostics/logger/logger.h"

BlamConfigurationFile::BlamConfigurationFile(std::string _filename, std::string _comment_delimeter)
{
	filename = _filename;
	comment_delimeter = _comment_delimeter;
	sections = std::map<std::string, BlamConfigurationSection*>();
}

BlamConfigurationFile::~BlamConfigurationFile()
{
	ClearSections();
}

void BlamConfigurationFile::ClearSections()
{
	std::map<std::string, BlamConfigurationSection*> orig_sections = sections;

	sections.clear();

	for (std::pair<std::string, BlamConfigurationSection*> section_pair : orig_sections)
	{
		delete section_pair.second;
	}

	orig_sections.clear();
}

DDLResult BlamConfigurationFile::Load()
{
	return Load(filename);
}

DDLResult BlamConfigurationFile::Load(std::string _filename)
{
	if (DDL::Utils::IO::FileExists(_filename))
	{
		std::ifstream config_file_stream = std::ifstream(_filename);

		BlamConfigurationSection* current_section = nullptr;
		bool section_already_exists = false;

		if (sections.contains(CONFIG_DUMMY_SECTION_NAME))
		{
			current_section = sections.at(CONFIG_DUMMY_SECTION_NAME);
			section_already_exists = true;
		}
		else
		{
			current_section = new BlamConfigurationSection(CONFIG_DUMMY_SECTION_NAME, filename);
			section_already_exists = false;
		}

		std::string line;
		int line_number = 0;

		while (getline(config_file_stream, line))
		{
			if (line != "")
			{
				if (line.starts_with("("))
				{
					if (line.ends_with(")"))
					{
						std::string new_section_name = DDL::Utils::String::Replace(line, "(", "");
						new_section_name = DDL::Utils::String::Replace(new_section_name, ")", "");

						if (!section_already_exists)
						{
							AddNewSection(current_section);
						}

						if (sections.contains(new_section_name))
						{
							DDL::Logger::LogEvent("closing configuration section " + current_section->name + ", updating values for existing section " + new_section_name);
							current_section = sections.at(new_section_name);
							section_already_exists = true;
						}
						else
						{
							DDL::Logger::LogEvent("closing configuration section " + current_section->name + ", starting new section " + new_section_name);
							current_section = new BlamConfigurationSection(new_section_name, filename);
							section_already_exists = false;
						}
					}
					else
					{
						DDL::Logger::LogEvent("found unclosed configuration section header on line " + std::to_string(line_number) + " in file '" + filename + "', this header will be ignored", DDLLogLevel::Warning);
					}
				}
				else
				{
					BlamConfigurationSetting* new_setting = new BlamConfigurationSetting(line, line_number, comment_delimeter, filename);

					if (new_setting->IsValid())
					{
						current_section->AddNewSetting(new_setting);
					}
					else
					{
						DDL::Logger::LogEvent("failed to interpret line as a valid setting (line #" + std::to_string(line_number) + "), skipping", DDLLogLevel::Warning);
					}
				}
			}

			line_number++;
		}

		config_file_stream.close();

		if (!section_already_exists)
		{
			AddNewSection(current_section);
		}

		DDL::Logger::LogEvent("finished loading configuration file '" + filename + "' (" + _filename + ")");

		loaded = true;

		return DDLResult::Success_OK;
	}
	else
	{
		DDL::Logger::LogEvent("cannot load config file '" + filename + "', file does not exist", DDLLogLevel::Error);
		return DDLResult::Error_FileNotFound;
	}
}

DDLResult BlamConfigurationFile::LoadDefaults(std::string _filename)
{
	defaults_filename = _filename;

	if (!DDL::Utils::IO::FileExists(_filename))
	{
		return DDLResult::Error_FileNotFound;
	}

	if (!DDL::Utils::IO::IsFile(_filename))
	{
		return DDLResult::Error_FileInvalid;
	}

	std::string default_contents = DDL::Utils::IO::GetFileContentsAsString(_filename);

	std::istringstream config_defaults_stream = std::istringstream(default_contents);

	BlamConfigurationSection* current_section = new BlamConfigurationSection(CONFIG_DUMMY_SECTION_NAME, filename);
	bool section_already_exists = true;

	std::string line;
	int line_number = 0;

	while (getline(config_defaults_stream, line))
	{
		if (line != "")
		{
			if (line.starts_with("("))
			{
				if (line.ends_with(")"))
				{
					std::string new_section_name = DDL::Utils::String::Replace(line, "(", "");
					new_section_name = DDL::Utils::String::Replace(new_section_name, ")", "");

					if (!section_already_exists)
					{
						DDL::Logger::LogEvent("configuration section '" + current_section->name + "' does not exist in configuration file, adding from default");
						AddNewSection(current_section);
					}

					if (!sections.contains(new_section_name))
					{
						current_section = new BlamConfigurationSection(new_section_name, filename);
						section_already_exists = false;
					}
					else
					{
						current_section = sections.at(new_section_name);
						section_already_exists = true;
					}
				}
				else
				{
					DDL::Logger::LogEvent("found unclosed configuration section header on line " + std::to_string(line_number) + " default configuration file " + _filename + " (file " + filename + "), notify a developer or submit a bug report", DDLLogLevel::Error);
				}
			}
			else
			{
				BlamConfigurationSetting* new_default_setting = new BlamConfigurationSetting(line, line_number, comment_delimeter, filename);

				if (new_default_setting->IsValid())
				{
					current_section->AddNewDefaultSetting(new_default_setting);

					if (!current_section->HasOption(new_default_setting->id))
					{
						DDL::Logger::LogEvent("default setting '" + new_default_setting->id + "' does not exist in configuration file, adding from default");

						BlamConfigurationSetting* new_setting = new BlamConfigurationSetting(line, line_number, comment_delimeter, filename);
						current_section->AddNewSetting(new_setting);
					}
				}
				else
				{
					DDL::Logger::LogEvent("failed to interpret line as a valid setting in defaults file " + _filename + " for file '" + filename + "' (line #" + std::to_string(line_number) + "), notify a developer or submit a bug report", DDLLogLevel::Error);
				}
			}
		}

		line_number++;
	}

	if (!section_already_exists)
	{
		DDL::Logger::LogEvent("configuration section '" + current_section->name + "' does not exist in configuration file, adding from default");
		AddNewSection(current_section);
	}

	DDL::Logger::LogEvent("finished loading defaults for configuration file '" + filename + "'");

	return DDLResult::Success_OK;
}

DDLResult BlamConfigurationFile::Reload()
{
	DDL::Logger::LogEvent("configuration file '" + filename + "' is being reloaded, some changes may not be applied until application restarts", DDLLogLevel::Warning);
	
	ClearSections();

	LoadDefaults(defaults_filename);
	Load(defaults_filename);

	DDLResult load_result = Load();

	return load_result;
}

void BlamConfigurationFile::Save()
{
	// Back up old file just in case
	{
		std::string old_contents = DDL::Utils::IO::GetFileContentsAsString(filename);
		DDL::Utils::IO::CreateNewFile(filename + ".bak", old_contents);
	}

	// Build new file
	{
		std::string new_contents = "";

		std::map<std::string, BlamConfigurationSection*>::iterator it;

		for (it = sections.begin(); it != sections.end(); it++)
		{
			BlamConfigurationSection* section = it->second;

			if (section->name != CONFIG_DUMMY_SECTION_NAME)
			{
				new_contents += "\n";
				new_contents += "(" + section->name + ")";
				new_contents += "\n";
			}

			std::map<std::string, BlamConfigurationSetting*>::iterator sit;

			for (sit = section->settings.begin(); sit != section->settings.end(); sit++)
			{
				BlamConfigurationSetting* setting = sit->second;

				bool write_to_file = true;

				if (section->default_settings.contains(setting->id))
				{
					BlamConfigurationSetting* default_setting = section->default_settings.at(setting->id);

					if (setting->type == default_setting->type)
					{
						bool value_modified = false;

						if (setting->type == BlamConfigurationSettingType::Boolean)
						{
							if (*setting->AsBool() != *default_setting->AsBool())
							{
								value_modified = true;
							}
						}
						else if (setting->type == BlamConfigurationSettingType::Color)
						{
							if (setting->AsColor()->r != default_setting->AsColor()->r)
							{
								value_modified = true;
							}

							if (setting->AsColor()->g != default_setting->AsColor()->g)
							{
								value_modified = true;
							}

							if (setting->AsColor()->b != default_setting->AsColor()->b)
							{
								value_modified = true;
							}

							if (setting->AsColor()->a != default_setting->AsColor()->a)
							{
								value_modified = true;
							}
						}
						else if (setting->type == BlamConfigurationSettingType::Float)
						{
							if (*setting->AsFloat() != *default_setting->AsFloat())
							{
								value_modified = true;
							}
						}
						else if (setting->type == BlamConfigurationSettingType::Int)
						{
							if (*setting->AsInt() != *default_setting->AsInt())
							{
								value_modified = true;
							}
						}
						else if (setting->type == BlamConfigurationSettingType::String)
						{
							if (*setting->AsString() != *default_setting->AsString())
							{
								value_modified = true;
							}
						}

						if (!value_modified)
						{
							DDL::Logger::LogEvent("configuration setting '" + setting->id + "' in section '" + section->name + "' in file '" + filename + "' will not be saved - setting value has not been changed from default");
							write_to_file = false;
						}
					}
				}

				if (write_to_file)
				{
					if (setting->type != BlamConfigurationSettingType::Comment)
					{
						std::string setting_line = setting->CreateConfigFileLine();
						new_contents += setting_line + "\n";
					}
				}
			}
		}

		DDL::Utils::IO::CreateNewFile(filename, new_contents);
	}

	DDL::Logger::LogEvent("configuration file '" + filename + "' saved to disk, previous contents saved as '" + filename + ".bak' if needed");
}

bool BlamConfigurationFile::IsLoaded()
{
	return loaded;
}

void BlamConfigurationFile::AddNewSection(BlamConfigurationSection* section)
{
	int unique_suffix = 0;

	while (sections.contains(section->name))
	{
		std::string duplicate_name = section->name + "_" + std::to_string(unique_suffix);
		DDL::Logger::LogEvent("configuration file '" + filename + "' already contains a section named " + section->name + ", renaming section to " + duplicate_name, DDLLogLevel::Warning);
		section->name = duplicate_name;

		unique_suffix++;
	}

	sections.insert(std::pair<std::string, BlamConfigurationSection*>(section->name, section));
}

bool BlamConfigurationFile::HasConfigurationSection(std::string section_name)
{
	if (sections.find(section_name) != sections.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

BlamConfigurationSection* BlamConfigurationFile::GetConfigurationSection(std::string section_name)
{
	if (sections.find(section_name) != sections.end())
	{
		return sections.at(section_name);
	}
	else
	{
		BlamConfigurationSection* new_section = new BlamConfigurationSection(section_name, filename);

		AddNewSection(new_section);

		return new_section;
	}
}

void BlamConfigurationFile::RestoreDefaultSettings()
{
	for (std::pair<std::string, BlamConfigurationSection*> section_pair : sections)
	{
		BlamConfigurationSection* section = section_pair.second;

		if (section)
		{
			section->RestoreDefaultSettings();
		}
	}

	DDL::Logger::LogEvent("configuration file '" + filename + "' was reset to defaults");
}

std::string* BlamConfigurationFile::GetString(std::string section_name, std::string option)
{
	BlamConfigurationSection* section = GetConfigurationSection(section_name);

	if (!section)
	{
		return nullptr;
	}

	return section->GetString(option);
}

bool* BlamConfigurationFile::GetBool(std::string section_name, std::string option)
{
	BlamConfigurationSection* section = GetConfigurationSection(section_name);

	if (!section)
	{
		return nullptr;
	}

	return section->GetBool(option);
}

float* BlamConfigurationFile::GetFloat(std::string section_name, std::string option)
{
	BlamConfigurationSection* section = GetConfigurationSection(section_name);

	if (!section)
	{
		return nullptr;
	}

	return section->GetFloat(option);
}

int* BlamConfigurationFile::GetInt(std::string section_name, std::string option)
{
	BlamConfigurationSection* section = GetConfigurationSection(section_name);

	if (!section)
	{
		return nullptr;
	}

	return section->GetInt(option);
}

BlamColor* BlamConfigurationFile::GetColor(std::string section_name, std::string option)
{
	BlamConfigurationSection* section = GetConfigurationSection(section_name);

	if (!section)
	{
		return nullptr;
	}

	return section->GetColor(option);
}