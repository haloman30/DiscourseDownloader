#include "main.h"

#include <iostream>
#include <string>

#include "components/settings/settings.h"
#include "components/diagnostics/logger/logger.h"
#include "components/utils/network/network.h"
#include "components/utils/io/io.h"
#include "components/discourse/discourse.h"

int main(int args_count, char** args)
{
	DDL::Logger::LogEvent("=== DiscourseDownloader v" + std::string(DISCOURSEDL_VERSION) + " ===");
	DDL::Logger::LogEvent("Developed by haloman30 - https://haloman30.com");
	DDL::Logger::LogEvent("Github URL: https://github.com/haloman30/DiscourseDownloader");
	DDL::Logger::LogEvent("Gitlab URL: https://gitlab.elaztek.com/haloman30/discoursedownloader/");
	DDL::Logger::LogEvent("==================================");

	// Initialization
	{
		DDL::Logger::StartLogger();

		bool config_exists = DDL::Utils::IO::FileExists("website.cfg");

		if (!DDL::Settings::LoadAllConfigurations())
		{
			DDL::Logger::LogEvent("website configuration file failed to load, see log for details. program will now exit.", DDLLogLevel::Error);
			DDL::Logger::ShutdownLogger();
			return -1;
		}

		if (!config_exists)
		{
			DDL::Logger::LogEvent("!!! NEW CONFIGURATION FILE CREATED !!!");

			DDL::Logger::LogEvent("DiscourseDownloader has created a new configuration");
			DDL::Logger::LogEvent("file in the application directory, website.cfg.");
			DDL::Logger::LogEvent("");
			DDL::Logger::LogEvent("Please edit this file to point to the forum you wish");
			DDL::Logger::LogEvent("to archive, as well as any other settings you want to");
			DDL::Logger::LogEvent("change.");
			DDL::Logger::LogEvent("");
			DDL::Logger::LogEvent("The application will now exit to allow you to modify");
			DDL::Logger::LogEvent("these options.");

			DDL::Logger::LogEvent("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

			DDL::Logger::ShutdownLogger();
			return 0;
		}
	}

	DDL::Discourse::DownloadWebContent();

	// Shutdown
	{
		DDL::Settings::CleanupConfigurations();
		DDL::Logger::ShutdownLogger();
	}
}