#include "settings.h"

#include "components/diagnostics/logger/logger.h"

BlamConfigurationFile* site_config = nullptr;
WebsiteConfig ddl_website_config = WebsiteConfig();

WebsiteConfig* DDL::Settings::GetSiteConfig()
{
	if (site_config)
	{
		return &ddl_website_config;
	}

	return nullptr;
}

bool DDL::Settings::LoadAllConfigurations()
{
	DDLResult result = DDLResult::Success_OK;

	result = DDL::Settings::Config::LoadConfiguration("website.cfg", "./data/website.cfg.default", ";", &site_config);

	if (DR_FAILED(result))
	{
		DDL::Logger::LogEvent("Failed to load website.cfg, see log for details.", DDLLogLevel::Warning);

		delete site_config;
		site_config = nullptr;

		return false;
	}

	ddl_website_config.website_url = *site_config->GetString("website_config", "website_url");
	ddl_website_config.max_get_more_topics = *site_config->GetInt("website_config", "max_get_more_topics");
	ddl_website_config.download_users = *site_config->GetBool("website_config", "download_users");

	ddl_website_config.cookie_name = *site_config->GetString("cookies", "cookie_name");
	ddl_website_config.cookie = *site_config->GetString("cookies", "cookie");

	return true;
}

void DDL::Settings::CleanupConfigurations()
{
	delete site_config;
	site_config = nullptr;
}