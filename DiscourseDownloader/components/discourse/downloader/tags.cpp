#include "components/discourse/discourse.h"

#include "components/diagnostics/logger/logger.h"
#include "components/settings/settings.h"
#include "components/utils/io/io.h"
#include "components/utils/string/string.h"
#include "components/utils/json/json.h"
#include "components/utils/network/network.h"
#include "components/utils/list/list.h"

void download_tag_extras(std::string tag_id, std::string tag_data_root)
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - tag extras will NOT be downloaded!", DDLLogLevel::Error);
		return;
	}

	std::string tag_extras_url = TAG_INFO_URL_FORMAT;
	{
		tag_extras_url = DDL::Utils::String::Replace(tag_extras_url, "<BASE_URL>", config->website_url);
		tag_extras_url = DDL::Utils::String::Replace(tag_extras_url, "<NAME>", tag_id);
	}

	std::string tag_pages_root = tag_data_root + "pages/";

	if (config->download_all_tag_extras)
	{
		int page = -1;
		bool continue_searching = true;

		while (continue_searching)
		{
			page++;

			std::string page_url = tag_extras_url + std::to_string(page);

			int http_code = -1;
			std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(page_url, &http_code);

			if (http_code == 200)
			{
				rapidjson::Document document = rapidjson::Document();
				document.Parse(response.c_str());

				DDL::Utils::IO::CreateNewFile(tag_pages_root + "page_" + std::to_string(page) + ".json", response);

				rapidjson::GenericArray topics = document["topic_list"]["topics"].GetArray();

				if (topics.Size() == 0)
				{
					continue_searching = false;
				}
			}
			else
			{
				DDL::Logger::LogEvent("failed to download tag topic list page " + std::to_string(page) + ", skipping!", DDLLogLevel::Error);
			}
		}
	}
	else
	{
		std::string page_url = tag_extras_url + "0";

		int http_code = -1;
		std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(page_url, &http_code);

		if (http_code == 200)
		{
			rapidjson::Document document = rapidjson::Document();
			document.Parse(response.c_str());

			DDL::Utils::IO::CreateNewFile(tag_data_root + "tag_info.json", response);
		}
		else
		{
			DDL::Logger::LogEvent("failed to download tag extras page, extra info will be skipped!", DDLLogLevel::Error);
		}
	}
}

void DDL::Discourse::Downloader::DownloadTags()
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - tags will NOT be downloaded!", DDLLogLevel::Error);
		return;
	}

	if (!config->download_topics)
	{
		DDL::Logger::LogEvent("topic downloading has been disabled in config, tags will not be downloaded - if this is "
			"not what you want, please edit website.cfg", DDLLogLevel::Warning);
		return;
	}

	DDL::Logger::LogEvent("downloading tags...");

	if (config->download_all_tag_extras)
	{
		DDL::Logger::LogEvent("downloading all tag extras has been enabled in config - tag downloading could "
			"take a while, please be patient...", DDLLogLevel::Warning);
	}

	std::string tags_root = JSON_TAGS_ROOT_FORMAT;
	tags_root = DDL::Utils::String::Replace(tags_root, "<JSON_ROOT>", config->json_path);

	std::string tag_info_url = TAG_LIST_URL_FORMAT;
	tag_info_url = DDL::Utils::String::Replace(tag_info_url, "<BASE_URL>", config->website_url);

	DDL::Utils::IO::ValidatePath(tags_root);

	int http_code = -1;
	std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(tag_info_url, &http_code);

	if (http_code == 200)
	{
		rapidjson::Document tag_list = rapidjson::Document();
		tag_list.Parse(response.c_str());

		DDL::Utils::IO::CreateNewFile(tags_root + "tags.json", response);

		rapidjson::GenericArray tags = tag_list["tags"].GetArray();

		for (int i = 0; i < tags.Size(); i++)
		{
			rapidjson::Value tag_json = tags[i].GetObj();
			std::string tag_id = tag_json["id"].GetString();

			DDL::Logger::LogEvent("downloading tag '" + tag_id + "'...");

			std::string tag_data_root = tags_root + tag_id + "/";
			DDL::Utils::IO::ValidatePath(tag_data_root);

			std::string tag_data_str = DDL::Utils::Json::Serialize(&tag_json);
			DDL::Utils::IO::CreateNewFile(tag_data_root + "tag.json", tag_data_str);

			download_tag_extras(tag_id, tag_data_root);
		}
	}
	else
	{
		DDL::Logger::LogEvent("failed to download tag list page, tags will not be downloaded!", DDLLogLevel::Error);
	}

	DDL::Logger::LogEvent("finished downloading tags");
}