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
	ddl_website_config.perform_html_build = *site_config->GetBool("website_config", "perform_html_build");
	ddl_website_config.skip_download = *site_config->GetBool("website_config", "skip_download");
	ddl_website_config.max_posts_per_request = *site_config->GetInt("website_config", "max_posts_per_request");
	ddl_website_config.topic_url_collection_notify_interval = *site_config->GetInt("website_config", "topic_url_collection_notify_interval");

	ddl_website_config.html_path = *site_config->GetString("paths", "html_dir");
	ddl_website_config.json_path = *site_config->GetString("paths", "json_dir");
	ddl_website_config.site_directory_root = *site_config->GetString("paths", "site_directory_root");

	ddl_website_config.cookie_name = *site_config->GetString("cookies", "cookie_name");
	ddl_website_config.cookie = *site_config->GetString("cookies", "cookie");

	// Perform any required alterations to config settings
	{
		if (ddl_website_config.website_url.ends_with('/'))
		{
			ddl_website_config.website_url = ddl_website_config.website_url.substr(0, ddl_website_config.website_url.length() - 1);
		}

		if (ddl_website_config.html_path.ends_with('/'))
		{
			ddl_website_config.html_path = ddl_website_config.html_path.substr(0, ddl_website_config.html_path.length() - 1);
		}

		if (ddl_website_config.json_path.ends_with('/'))
		{
			ddl_website_config.json_path = ddl_website_config.json_path.substr(0, ddl_website_config.json_path.length() - 1);
		}

		if (!ddl_website_config.site_directory_root.ends_with('/'))
		{
			ddl_website_config.site_directory_root += "/";
		}

		ddl_website_config.html_path = ddl_website_config.site_directory_root + ddl_website_config.html_path;
		ddl_website_config.json_path = ddl_website_config.site_directory_root + ddl_website_config.json_path;
	}

	return true;
}

void DDL::Settings::CleanupConfigurations()
{
	delete site_config;
	site_config = nullptr;
}