#include "components/discourse/discourse.h"

#include "components/diagnostics/logger/logger.h"
#include "components/settings/settings.h"
#include "components/utils/io/io.h"
#include "components/utils/string/string.h"
#include "components/utils/json/json.h"
#include "components/utils/network/network.h"
#include "components/utils/list/list.h"

void DDL::Discourse::Downloader::DownloadSiteInfo()
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - user profiles will NOT be downloaded!", DDLLogLevel::Error);
		return;
	}

	if (!config->download_misc)
	{
		DDL::Logger::LogEvent("miscellaneous site info downloading has been disabled in config - if this is "
			"not what you want, please edit website.cfg", DDLLogLevel::Warning);
		return;
	}

	DDL::Logger::LogEvent("downloading miscellaneous site info...");

	// Download site info
	{
		DDL::Logger::LogEvent("downloading site info");

		std::string data_root = JSON_SITE_ROOT_FORMAT;
		{
			data_root = DDL::Utils::String::Replace(data_root, "<JSON_ROOT>", config->json_path);
		}

		std::string site_info_url = SITE_INFO_URL_FORMAT;
		{
			site_info_url = DDL::Utils::String::Replace(site_info_url, "<BASE_URL>", config->website_url);
		}

		DDL::Utils::IO::ValidatePath(data_root);

		int http_code = -1;
		std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(site_info_url, &http_code);

		if (http_code == 200)
		{
			rapidjson::Document document = rapidjson::Document();
			document.Parse(response.c_str());

			DDL::Utils::IO::CreateNewFile(data_root + "site.json", response);
		}
		else
		{
			DDL::Logger::LogEvent("failed to download site info", DDLLogLevel::Error);
		}
	}

	// Download groups
	{
		DDL::Logger::LogEvent("downloading groups");

		std::string groups_dir = JSON_GROUPS_ROOT_FORMAT;
		groups_dir = DDL::Utils::String::Replace(groups_dir, "<JSON_ROOT>", config->json_path);
		
		std::string groups_list_url = GROUPS_LIST_URL_FORMAT;
		groups_list_url = DDL::Utils::String::Replace(groups_list_url, "<BASE_URL>", config->website_url);

		DDL::Utils::IO::ValidatePath(groups_dir);

		bool continue_scanning = true;
		int page = -1;

		while (continue_scanning)
		{
			page++;

			int http_code = -1;
			std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(groups_list_url + std::to_string(page), &http_code);

			if (http_code == 200)
			{
				rapidjson::Document document = rapidjson::Document();
				document.Parse(response.c_str());

				rapidjson::GenericArray groups = document["groups"].GetArray();

				if (groups.Size() == 0)
				{
					continue_scanning = false;
					break;
				}

				DDL::Utils::IO::CreateNewFile(groups_dir + "page_" + std::to_string(page) + ".json", response);

				for (int i = 0; i < groups.Size(); i++)
				{
					rapidjson::Value group_info = groups[i].GetObj();

					std::string group_name = group_info["name"].GetString();
					int group_id = group_info["id"].GetInt();

					std::string group_info_url = GROUPS_INFO_URL_FORMAT;
					{
						group_info_url = DDL::Utils::String::Replace(group_info_url, "<BASE_URL>", config->website_url);
						group_info_url = DDL::Utils::String::Replace(group_info_url, "<NAME>", group_name);
					}

					// Download group info
					{
						int group_info_http_code = -1;
						std::string group_info_response = DDL::Utils::Network::PerformHTTPRequestWithRetries(group_info_url, &group_info_http_code);

						if (group_info_http_code == 200)
						{
							rapidjson::Document document = rapidjson::Document();
							document.Parse(group_info_response.c_str());

							DDL::Utils::IO::CreateNewFile(groups_dir + std::to_string(group_id) + "_" + group_name + ".json", group_info_response);
						}
						else
						{
							DDL::Logger::LogEvent("failed to download info for group " + group_name + " (got http "
								+ std::to_string(group_info_http_code) + ")!", DDLLogLevel::Error);
						}
					}

					// Download flair
					if (!group_info["flair_url"].IsNull())
					{
						std::string flair_url = group_info["flair_url"].GetString();

						if (flair_url.length() > 0)
						{
							int flair_http_code = -1;
							std::string flair_response = DDL::Utils::Network::PerformHTTPRequestWithRetries(flair_url, &flair_http_code);

							if (flair_http_code == 200)
							{
								std::string extension = flair_url.substr(flair_url.find_last_of('.'));
								DDL::Utils::IO::CreateNewFile(groups_dir + std::to_string(group_id) + "_" + group_name + "_flair" + extension, flair_response);
							}
							else
							{
								DDL::Logger::LogEvent("failed to download flair image for group " + group_name + ", image will be skipped (got http "
									+ std::to_string(flair_http_code) + ")!", DDLLogLevel::Error);
							}
						}
					}
				}
			}
			else
			{
				DDL::Logger::LogEvent("group list page " + std::to_string(page) + ", some groups will not be downloaded (got http "
					+ std::to_string(http_code) + ")!", DDLLogLevel::Error);
			}
		}
	}

	DDL::Logger::LogEvent("finished downloading miscellaneous site info");
}