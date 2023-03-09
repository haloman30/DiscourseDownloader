#include "discourse.h"

#include "components/3rdparty/rapidjson/document.h"

#include "components/utils/json/json.h"
#include "components/utils/network/network.h"
#include "components/utils/string/string.h"
#include "components/utils/io/io.h"
#include "components/settings/settings.h"
#include "components/diagnostics/logger/logger.h"

std::vector<DiscourseCategory*> downloaded_categories = std::vector<DiscourseCategory*>();

std::string category_list_url_format = "<BASE_URL>/categories.json?include_subcategories=true";
std::string category_info_url_format = "<BASE_URL>/c/<CAT_ID>/show.json";

std::string topic_list_url_format = "<BASE_URL>/c/<CAT_SLUG>/<CAT_ID>.json?page=<PAGE>";
std::string topic_info_url_format = "<BASE_URL>/t/<TOPIC_ID>.json";
std::string topic_posts_url_format = "<BASE_URL>/t/<TOPIC_ID>/posts.json?<POST_LIST>";


std::string json_category_info_path = "<JSON_ROOT>/c/<CAT_ID>/";

//post_ids[]=<POST_ID>

DDLResult download_topic_posts(DiscourseTopic* topic)
{
	if (!topic)
	{
		DDL::Logger::LogEvent("tried to download topics, but topic was nullptr - skipping topic", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}

	if (!topic->json_file)
	{
		DDL::Logger::LogEvent("tried to download topics, but topic json document was nullptr - skipping topic", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}

	// trim post stream to remove first 20 items, keep first 20 from topic json

	// if needed, download all remaining posts
}

DDLResult download_category_topics(DiscourseCategory* category)
{
	if (!category)
	{
		DDL::Logger::LogEvent("tried to download category topics, but category was nullptr - skipping category", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}

	if (!category->json_file)
	{
		DDL::Logger::LogEvent("tried to download category topics, but category json document was nullptr - skipping category", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}
}

DDLResult download_category(DiscourseCategory* category)
{
	if (!category)
	{
		DDL::Logger::LogEvent("tried to download category, but category was nullptr - skipping category", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}

	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - skipping category", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}

	if (!DDL::Utils::IO::FileExists(config->json_path))
	{
		DDL::Utils::IO::ValidatePath(config->json_path);
	}

	std::string json_string = DDL::Utils::Json::Serialize(category->json_file);
	std::string category_directory = json_category_info_path;
	{
		category_directory = DDL::Utils::String::Replace(category_directory, "<JSON_ROOT>", config->json_path);
		category_directory = DDL::Utils::String::Replace(category_directory, "<CAT_ID>", std::to_string(category->category_id));
	}

	DDL::Utils::IO::ValidatePath(category_directory);

	DDL::Utils::IO::CreateNewFile(category_directory + "show.json", json_string);

	// download category info, prepare folders

	// download each topic within category

	DDL::Logger::LogEvent("finished downloading category with id '" + std::to_string(category->category_id) + "'", DDLLogLevel::Error);
	return DDLResult::Success_OK;
}

void add_categories_from_array(rapidjson::GenericArray<false, rapidjson::Value> category_list)
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - skipping category list", DDLLogLevel::Error);
		return;
	}

	for (int i = 0; i < category_list.Size(); i++)
	{
		rapidjson::Value category_json = category_list[i].GetObj();

		DiscourseCategory* category = new DiscourseCategory();

		category->json_file = new rapidjson::Document();
		category->json_file->Parse(DDL::Utils::Json::Serialize(&category_json).c_str());
		category->category_id = category_json["id"].GetInt();

		// Get extra category info
		{
			int category_id = -1;
			int http_code = -1;

			std::string category_extra_info_url = category_info_url_format;
			{
				category_extra_info_url = DDL::Utils::String::Replace(category_extra_info_url, "<BASE_URL>", config->website_url);
				category_extra_info_url = DDL::Utils::String::Replace(category_extra_info_url, "<CAT_ID>", std::to_string(category->category_id));
			}

			std::string category_info_response = DDL::Utils::Network::PerformHTTPRequestWithRetries(category_extra_info_url, &http_code);

			if (http_code == 200)
			{
				rapidjson::Document extra_info_json = rapidjson::Document();
				extra_info_json.Parse(category_info_response.c_str());

				for (rapidjson::Value::ConstMemberIterator it = extra_info_json["category"].MemberBegin(); it != extra_info_json["category"].MemberEnd(); it++)
				{
					if (!category->json_file->HasMember(it->name))
					{
						rapidjson::Value key = rapidjson::Value(it->name.GetString(), category->json_file->GetAllocator());
						rapidjson::Value value = rapidjson::Value(it->value, category->json_file->GetAllocator());

						category->json_file->AddMember(key, value, category->json_file->GetAllocator());
					}
				}
			}
			else
			{
				DDL::Logger::LogEvent("failed to get additional category info for category with id '"
					+ std::to_string(category->category_id) + "', category will be saved without this information", DDLLogLevel::Warning);
			}
		}

		if (category_json.HasMember("subcategory_list"))
		{
			rapidjson::GenericArray subcategory_list = category_json["subcategory_list"].GetArray();
			add_categories_from_array(subcategory_list);
		}

		downloaded_categories.push_back(category);
	}
}

void DDL::Discourse::DownloadWebContent()
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - web content will NOT be downloaded!", DDLLogLevel::Error);
		return;
	}

	std::string category_list_url = DDL::Utils::String::Replace(category_list_url_format, "<BASE_URL>", config->website_url);
	int http_code = -1;

	std::string category_response = DDL::Utils::Network::PerformHTTPRequestWithRetries(category_list_url, &http_code);

	if (http_code != 200)
	{
		DDL::Logger::LogEvent("category listing response returned '" + std::to_string(http_code)
			+ "', expected 200 - web content will NOT be downloaded!", DDLLogLevel::Error);
		return;
	}

	rapidjson::Document* document = new rapidjson::Document();
	document->Parse(category_response.c_str());

	rapidjson::GenericArray category_list = (*document)["category_list"]["categories"].GetArray();
	add_categories_from_array(category_list);

	for (DiscourseCategory* category : downloaded_categories)
	{
		download_category(category);
	}
}