#include "components/discourse/discourse.h"

#include "components/diagnostics/logger/logger.h"
#include "components/settings/settings.h"
#include "components/utils/io/io.h"
#include "components/utils/string/string.h"
#include "components/utils/json/json.h"
#include "components/utils/network/network.h"
#include "components/utils/list/list.h"

std::vector<int> incomplete_users = std::vector<int>();

bool download_pms()
{
	return false;
}

bool download_user_actions(std::string user_root, std::string username, int user_id)
{
	bool complete_download = true;
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - user actions will NOT be downloaded!", DDLLogLevel::Error);
		return false;
	}

	std::string actions_url = USER_ACTIONS_INFO_URL_FORMAT;
	{
		actions_url = DDL::Utils::String::Replace(actions_url, "<BASE_URL>", config->website_url);
		actions_url = DDL::Utils::String::Replace(actions_url, "<USERNAME>", username);
	}

	std::string actions_data_root = user_root + "actions/";
	DDL::Utils::IO::ValidatePath(actions_data_root);

	bool continue_download = true;
	int offset = 0;
	int page_number = 0;

	while (continue_download)
	{
		std::string url = actions_url + std::to_string(offset);

		int http_code = -1;
		std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(url, &http_code);

		if (http_code == 200)
		{
			rapidjson::Document document = rapidjson::Document();
			document.Parse(response.c_str());

			DDL::Utils::IO::CreateNewFile(actions_data_root + "page_" + std::to_string(page_number) + ".json", response);
		}
		else
		{
			DDL::Logger::LogEvent("failed to download user actions page " + std::to_string(page_number) + " (offset "
				+ std::to_string(offset) + ") for user " + std::to_string(user_id) + ", page will be skipped!", DDLLogLevel::Error);
			complete_download = false;
		}

		if (!config->download_all_user_actions)
		{
			continue_download = false;
		}
		else
		{
			offset += 30;
		}

		page_number++;
	}

	return complete_download;
}

bool download_avatar(std::string user_local_root, std::string avatar_template, int user_id)
{
	bool download_all_sizes = true;
	bool complete_download = true;

	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("error while downloading user avatar: could not get website config, defaulting to download all sizes", DDLLogLevel::Warning);
	}
	else
	{
		download_all_sizes = config->download_all_avatar_sizes;
	}

	int sizes[] = {
		30,
		40,
		45,
		48,
		50,
		67,
		96,
		120,
		135,
		180,
		240,
		360
	};

	for (int size : sizes)
	{
		if (!download_all_sizes && size != 360)
		{
			continue;
		}

		int http_code = -1;
		std::string avatar_url = config->website_url + DDL::Utils::String::Replace(avatar_template, "{size}", std::to_string(size));

		std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(avatar_url, &http_code);

		if (http_code == 200)
		{
			std::string file_path = user_local_root + "avatar_" + std::to_string(size) + avatar_url.substr(avatar_url.find_last_of('.'));
			DDL::Utils::IO::CreateNewFileBinaryMode(file_path, response);
		}
		else
		{
			DDL::Logger::LogEvent("failed to download avatar for user '" + std::to_string(user_id) + "' at size '" + std::to_string(size)
				+ "', this avatar size will NOT be saved (got http " + std::to_string(http_code) + ")", DDLLogLevel::Warning);
			complete_download = false;
		}
	}

	return complete_download;
}

void download_user_list(bool only_download_incomplete)
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - user profiles will NOT be downloaded!", DDLLogLevel::Error);
		return;
	}

	if (config->download_all_user_actions)
	{
		DDL::Logger::LogEvent("configuration is set to download ALL user actions, this could result in users taking a long time to download...",
			DDLLogLevel::Warning);
	}

	bool keep_searching = true;
	int page_num = -1;
	std::string user_list_url = DIRECTORY_LIST_URL_FORMAT;
	{
		user_list_url = DDL::Utils::String::Replace(user_list_url, "<BASE_URL>", config->website_url);
	}

	std::string directory_local_root = JSON_DIRECTORY_ROOT_FORMAT;
	{
		directory_local_root = DDL::Utils::String::Replace(directory_local_root, "<JSON_ROOT>", config->json_path);
	}

	DDL::Utils::IO::ValidatePath(directory_local_root);

	int total_downloaded_users = 0;

	while (keep_searching)
	{
		page_num++;
		int http_code = -1;

		std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(user_list_url + std::to_string(page_num), &http_code);

		if (http_code != 200)
		{
			if (http_code == 403 && config->fail_on_403)
			{
				DDL::Logger::LogEvent("stopping user search because a user list request returned 403 and the user list api is "
					"most likely blocked. if this is not the case, please edit website.cfg", DDLLogLevel::Error);
				keep_searching = false;
			}

			continue;
		}

		rapidjson::Document* directory_page = new rapidjson::Document();
		directory_page->Parse(response.c_str());

		DDL::Utils::IO::CreateNewFile(directory_local_root + "page_" + std::to_string(page_num) + ".json", response);

		int total_user_count = (*directory_page)["meta"]["total_rows_directory_items"].GetInt();

		rapidjson::GenericArray directory_items = (*directory_page)["directory_items"].GetArray();

		if (directory_items.Size() == 0)
		{
			keep_searching = false;
			break;
		}

		for (int i = 0; i < directory_items.Size(); i++)
		{
			rapidjson::Value item = directory_items[i].GetObj();

			std::string username = item["user"]["username"].GetString();
			std::string avatar_template = item["user"]["avatar_template"].GetString();
			int user_id = item["user"]["id"].GetInt();

			if (only_download_incomplete && !DDL::Utils::List::VectorContains(incomplete_users, user_id))
			{
				continue;
			}

			std::string user_data_root = JSON_USER_ROOT_FORMAT;
			{
				user_data_root = DDL::Utils::String::Replace(user_data_root, "<JSON_ROOT>", config->json_path);
				user_data_root = DDL::Utils::String::Replace(user_data_root, "<USER_ID>", std::to_string(user_id));
			}

			std::string user_info_url = USER_INFO_URL_FORMAT;
			{
				user_info_url = DDL::Utils::String::Replace(user_info_url, "<BASE_URL>", config->website_url);
				user_info_url = DDL::Utils::String::Replace(user_info_url, "<USERNAME>", username);
			}

			DDL::Utils::IO::ValidatePath(user_data_root);

			int user_http_code = -1;
			std::string user_response = DDL::Utils::Network::PerformHTTPRequestWithRetries(user_info_url, &user_http_code);

			if (user_http_code == 200)
			{
				rapidjson::Document user_info_document = rapidjson::Document();
				user_info_document.Parse(user_response.c_str());

				rapidjson::Value key = rapidjson::Value("directory_item", user_info_document.GetAllocator());
				rapidjson::Value value = rapidjson::Value(item, user_info_document.GetAllocator());

				user_info_document.AddMember(key, value, user_info_document.GetAllocator());

				std::string json_string = DDL::Utils::Json::Serialize(&user_info_document);

				DDL::Utils::IO::CreateNewFile(user_data_root + "user.json", json_string);
			}
			else if (!only_download_incomplete)
			{
				DDL::Logger::LogEvent("could not download full user info for user '" + std::to_string(user_id)
					+ "', will retry download later (directory entry will be saved)", DDLLogLevel::Warning);

				std::string json_string = DDL::Utils::Json::Serialize(&item);
				DDL::Utils::IO::CreateNewFile(user_data_root + "user_d.json", json_string);

				if (!DDL::Utils::List::VectorContains(incomplete_users, user_id))
				{
					incomplete_users.push_back(user_id);
				}
			}

			// Download avatar
			{
				bool avatar_result = download_avatar(user_data_root, avatar_template, user_id);

				if (!avatar_result)
				{
					DDL::Logger::LogEvent("one or more avatar sizes failed to download, will retry later", DDLLogLevel::Warning);

					if (!DDL::Utils::List::VectorContains(incomplete_users, user_id))
					{
						incomplete_users.push_back(user_id);
					}
				}
			}

			// Download user badges
			{
				std::string badges_url = USER_BADGES_INFO_URL_FORMAT;
				{
					badges_url = DDL::Utils::String::Replace(badges_url, "<BASE_URL>", config->website_url);
					badges_url = DDL::Utils::String::Replace(badges_url, "<USERNAME>", username);
				}

				int badges_http_code = -1;
				std::string badges_response = DDL::Utils::Network::PerformHTTPRequestWithRetries(badges_url, &badges_http_code);

				if (badges_http_code == 200)
				{
					rapidjson::Document badges_document = rapidjson::Document();
					badges_document.Parse(badges_response.c_str());

					DDL::Utils::IO::CreateNewFile(user_data_root + "badges.json", badges_response);
				}
				else
				{
					// badges download failed
				}
			}

			// Download user actions
			if (!download_user_actions(user_data_root, username, user_id))
			{
				DDL::Logger::LogEvent("one or more actions pages failed to download, will retry later", DDLLogLevel::Warning);

				if (!DDL::Utils::List::VectorContains(incomplete_users, user_id))
				{
					incomplete_users.push_back(user_id);
				}
			}

			// Download private messages
			{
				// @todo
			}

			total_downloaded_users++;
			DDL::Logger::LogEvent("downloaded user " + std::to_string(user_id) + " (" + std::to_string(total_downloaded_users) + "/" + std::to_string(total_user_count) + ")...");
		}

		//DDL::Logger::LogEvent("downloading users... (" + std::to_string(total_downloaded_users) + "/"
		//	+ std::to_string(total_user_count) + ")");
	}
}

void DDL::Discourse::Downloader::DownloadUsers()
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - user profiles will NOT be downloaded!", DDLLogLevel::Error);
		return;
	}

	if (!config->download_users)
	{
		DDL::Logger::LogEvent("user profile downloading has been disabled in config - if this is not what you want, please edit website.cfg",
			DDLLogLevel::Warning);
		return;
	}

	DDL::Logger::LogEvent("downloading user data");

	download_user_list(false);

	DDL::Logger::LogEvent("finished downloading user data");
}