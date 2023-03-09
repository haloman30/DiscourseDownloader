#include "config.h"

#include "components/utils/io/io.h"
#include "components/diagnostics/logger/logger.h"

DDLResult DDL::Settings::Config::LoadConfiguration(std::string filename, std::string comment_delimeter, OUT BlamConfigurationFile** file)
{
	BlamConfigurationFile* new_file = new BlamConfigurationFile(filename, comment_delimeter);
	
	DDLResult load_result = new_file->Load();

	if (DR_SUCCEEDED(load_result))
	{
		*file = new_file;
	}

	return load_result;
}

DDLResult DDL::Settings::Config::LoadConfiguration(std::string filename, std::string defaults_filename, std::string comment_delimeter, OUT BlamConfigurationFile** file)
{
	BlamConfigurationFile* new_file = new BlamConfigurationFile(filename, comment_delimeter);

	DDLResult load_result = DDLResult::Success_OK;

	// Load default settings
	{
		if (DDL::Utils::IO::FileExists(defaults_filename))
		{
			load_result = new_file->LoadDefaults(defaults_filename);
			load_result = new_file->Load(defaults_filename);
		}
		else
		{
			DDL::Logger::LogEvent("cannot load defaults for configuration file '" + defaults_filename + "' - file not found. this may cause errors", DDLLogLevel::Warning);
		}
	}

	// Load user settings
	{
		if (DDL::Utils::IO::FileExists(filename))
		{
			load_result = new_file->Load();
		}
		else
		{
			DDL::Logger::LogEvent("cannot load user configuration file '" + filename + "' - file not found, creating new config");
			DDL::Utils::IO::CreateNewFile(filename, DDL::Utils::IO::GetFileContentsAsString(defaults_filename));
		}
	}

	if (DR_SUCCEEDED(load_result))
	{
		*file = new_file;
	}
	else
	{
		delete new_file;
	}

	return load_result;
}