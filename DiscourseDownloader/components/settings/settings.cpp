#include "settings.h"

#include "components/diagnostics/logger/logger.h"
#include "components/utils/string/string.h"
#include "components/utils/converters/converters.h"

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
	ddl_website_config.max_http_retries = *site_config->GetInt("website_config", "max_http_retries");
	ddl_website_config.http_retry_use_backoff = *site_config->GetBool("website_config", "http_retry_use_backoff");
	ddl_website_config.http_backoff_increment = *site_config->GetInt("website_config", "http_backoff_increment");
	ddl_website_config.max_posts_per_request = *site_config->GetInt("website_config", "max_posts_per_request");
	ddl_website_config.topic_url_collection_notify_interval = *site_config->GetInt("website_config", "topic_url_collection_notify_interval");
	ddl_website_config.sanity_check_on_finish = *site_config->GetBool("website_config", "sanity_check_on_finish");
	ddl_website_config.thorough_sanity_check = *site_config->GetBool("website_config", "thorough_sanity_check");
	ddl_website_config.download_subcategory_topics = *site_config->GetBool("website_config", "download_subcategory_topics");
	ddl_website_config.use_category_id_filter = *site_config->GetBool("website_config", "use_category_id_filter");
	ddl_website_config.category_id_filter = *site_config->GetString("website_config", "category_id_filter");
	ddl_website_config.use_filter_as_blacklist = *site_config->GetBool("website_config", "use_filter_as_blacklist");
	ddl_website_config.download_skip_existing_categories = *site_config->GetBool("website_config", "download_skip_existing_categories");
	ddl_website_config.download_skip_existing_topics = *site_config->GetBool("website_config", "download_skip_existing_topics");
	ddl_website_config.download_skip_existing_posts = *site_config->GetBool("website_config", "download_skip_existing_posts");
	ddl_website_config.override_user_agent = *site_config->GetBool("website_config", "override_user_agent");
	ddl_website_config.user_agent = *site_config->GetString("website_config", "user_agent");
	ddl_website_config.strict_topic_count_checks = *site_config->GetBool("website_config", "strict_topic_count_checks");
	ddl_website_config.request_retry_delay = *site_config->GetInt("website_config", "request_retry_delay");

	ddl_website_config.html_path = *site_config->GetString("paths", "html_dir");
	ddl_website_config.json_path = *site_config->GetString("paths", "json_dir");
	ddl_website_config.site_directory_root = *site_config->GetString("paths", "site_directory_root");

	ddl_website_config.cookie_name = *site_config->GetString("cookies", "cookie_name");
	ddl_website_config.cookie = *site_config->GetString("cookies", "cookie");

	ddl_website_config.disable_long_finish_message = *site_config->GetBool("misc", "disable_long_finish_message");
	ddl_website_config.log_level_debug = *site_config->GetBool("misc", "log_level_debug");

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

		if (ddl_website_config.use_category_id_filter)
		{
			std::vector<std::string> category_ids_str = DDL::Utils::String::Split(ddl_website_config.category_id_filter, ",");

			for (std::string category_id_str : category_ids_str)
			{
				if (DDL::Converters::IsStringInt(category_id_str))
				{
					ddl_website_config.category_id_whitelist.push_back(DDL::Converters::StringToInt(category_id_str));
				}
				else
				{
					DDL::Logger::LogEvent("error while parsing category_id_filter in settings: could not parse '"
						+ category_id_str + "' as an integer - this category id will be skipped", DDLLogLevel::Warning);
				}
			}
		}
	}

	return true;
}

void DDL::Settings::CleanupConfigurations()
{
	delete site_config;
	site_config = nullptr;
}