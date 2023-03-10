#include "main.h"

#include <iostream>
#include <string>

#include "components/settings/settings.h"
#include "components/diagnostics/logger/logger.h"
#include "components/utils/network/network.h"
#include "components/utils/io/io.h"
#include "components/discourse/discourse.h"
#include "components/settings/switches/switches.h"

int main(int args_count, char* args[])
{
	DDL::Logger::LogEvent("=== DiscourseDownloader v" + std::string(DISCOURSEDL_VERSION) + " ===");
	DDL::Logger::LogEvent("Developed by haloman30 - https://haloman30.com");
	DDL::Logger::LogEvent("Github URL: https://github.com/haloman30/DiscourseDownloader");
	DDL::Logger::LogEvent("Gitlab URL: https://gitlab.elaztek.com/haloman30/discoursedownloader/");
	DDL::Logger::LogEvent("==================================");

	// Initialization
	{
		DDL::Logger::StartLogger();

		DDL::Settings::Switches::ParseSwitches(args_count, args);
		DDL::Settings::Config::SetConfigDebugEnabled(DDL::Settings::Switches::IsSwitchPresent("config_debug"));

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

	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("website configuration file failed to load - GetSiteConfig() returned nullptr, program will exit", DDLLogLevel::Error);

		DDL::Settings::CleanupConfigurations();
		DDL::Logger::ShutdownLogger();
		return -1;
	}

	if (config->log_level_debug)
	{
		DDL::Logger::LogEvent("log level debug :: info", DDLLogLevel::Info);
		DDL::Logger::LogEvent("log level debug :: warning", DDLLogLevel::Warning);
		DDL::Logger::LogEvent("log level debug :: error", DDLLogLevel::Error);
	}

	if (!config->skip_download)
	{
		DDL::Discourse::DownloadWebContent();
	}
	else
	{
		DDL::Logger::LogEvent("skipping download step based on configuration");
	}

	if (config->perform_html_build)
	{
		DDL::Discourse::BuildArchiveWebsite();
	}
	else
	{
		DDL::Logger::LogEvent("skipping html build step based on configuration");
	}

	DDL::Logger::LogEvent("######################### DOWNLOAD COMPLETE #########################");

	if (!config->disable_long_finish_message)
	{
		DDL::Logger::LogEvent("All website archive tasks are complete. Please look through the");
		DDL::Logger::LogEvent("logs for any potential errors to ensure your archive is complete.");
		DDL::Logger::LogEvent("");
		DDL::Logger::LogEvent("Be sure to hold onto the downloaded json data, as this can be reused");
		DDL::Logger::LogEvent("later to rebuild the HTML website in the event of new features or");
		DDL::Logger::LogEvent("changes.");
		DDL::Logger::LogEvent("");
		DDL::Logger::LogEvent("If you're using this tool to download a website that is");
		DDL::Logger::LogEvent("preparing to shut down, you are *strongly* encouraged to upload this");
		DDL::Logger::LogEvent("archive somewhere like https://archive.org/ to ensure that it can't");
		DDL::Logger::LogEvent("be lost in the future.");
		DDL::Logger::LogEvent("");
		DDL::Logger::LogEvent("The HTML archive directory contains a complete, ready-to-use website");
		DDL::Logger::LogEvent("intended for easy public viewing and can be safely uploaded to any");
		DDL::Logger::LogEvent("webserver capable of hosting HTML and CSS.");
		DDL::Logger::LogEvent("");
		DDL::Logger::LogEvent("Thank you for using DiscourseDownloader and participating in the");
		DDL::Logger::LogEvent("preservation of web content!");
	}
	else
	{
		DDL::Logger::LogEvent("All website archive tasks are complete. Please look through the");
		DDL::Logger::LogEvent("logs for any potential errors to ensure your archive is complete.");
		DDL::Logger::LogEvent("");
		DDL::Logger::LogEvent("Thank you for using DiscourseDownloader and participating in the");
		DDL::Logger::LogEvent("preservation of web content!");
	}

	// Shutdown
	{
		DDL::Settings::CleanupConfigurations();
		DDL::Logger::ShutdownLogger();
	}
}