#include "settings.h"

#include "components/diagnostics/logger/logger.h"
#include "components/utils/io/io.h"
#include "components/utils/json/json.h"
#include "components/utils/string/string.h"

DDLSiteConfig::DDLSiteConfig(std::string _file_path) : BlamConfigurationFile(_file_path, ";")
{

}

DDLResult DDLSiteConfig::Load()
{
	BlamConfigurationFile::Load();


	DDL::Logger::LogEvent("Loading DDLSiteConfig...");

	if (!DDL::Utils::IO::FileExists(file_path))
	{
		DDL::Logger::LogEvent("Failed to load DDLSiteConfig - file '" + file_path + "' does not exist", DDLLogLevel::Error);
		return DDLResult::Error_FileNotFound;
	}

	std::string config_contents = DDL::Utils::IO::GetFileContentsAsString(file_path);

	rapidjson::Document json;
	json.Parse(config_contents.c_str());

	site_url = DDL::Utils::Json::GetValueAsString(&json, "site_url");

	DDL::Logger::LogEvent("DDLSiteConfig loaded successfully");

	return DDLResult::Success_OK;
}