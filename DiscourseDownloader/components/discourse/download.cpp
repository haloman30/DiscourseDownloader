#include "discourse.h"

#include "components/3rdparty/rapidjson/document.h"

#include "components/utils/json/json.h"
#include "components/utils/network/network.h"
#include "components/utils/string/string.h"
#include "components/utils/io/io.h"
#include "components/settings/settings.h"
#include "components/diagnostics/logger/logger.h"

std::vector<DDLDownloadRetryInfo*> pending_retries = std::vector<DDLDownloadRetryInfo*>();

std::vector<DiscourseCategory*> downloaded_categories = std::vector<DiscourseCategory*>();

std::string category_list_url_format = "<BASE_URL>/categories.json?include_subcategories=true";
std::string category_info_url_format = "<BASE_URL>/c/<CAT_ID>/show.json";

std::string topic_list_url_format = "<BASE_URL>/c/<CAT_SLUG>/<CAT_ID>.json?page=";
std::string topic_info_url_format = "<BASE_URL>/t/<TOPIC_ID>.json";
std::string topic_posts_url_format = "<BASE_URL>/t/<TOPIC_ID>/posts.json?";

std::string user_list_url_format = "<BASE_URL>/directory_items.json?period=all&page=";

std::string json_category_info_path = "<JSON_ROOT>/c/<CAT_ID>/";

//post_ids[]=<POST_ID>

typedef std::vector<int> topic_post_chunk;

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

DDLResult download_category_topics(DiscourseCategory* category, std::vector<std::string>* topic_url_list)
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - skipping category topics", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}

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

	if (!topic_url_list)
	{
		DDL::Logger::LogEvent("tried to download category topics, but topic_url_list was nullptr - skipping category", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}

	std::string topic_dir_base = json_category_info_path + "topics/";
	{
		topic_dir_base = DDL::Utils::String::Replace(topic_dir_base, "<JSON_ROOT>", config->json_path);
		topic_dir_base = DDL::Utils::String::Replace(topic_dir_base, "<CAT_ID>", std::to_string(category->category_id));
	}

	DDL::Utils::IO::ValidatePath(topic_dir_base);

	int requests_until_next_notify = 6;

	//for (std::string topic_url : *topic_url_list)
	for (int ti = 0; ti < topic_url_list->size(); ti++)
	{
		std::string topic_url = topic_url_list->at(ti);

		int http_code = -1;
		std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(topic_url, &http_code);

		if (http_code == 200)
		{
			rapidjson::Document* topic_json = new rapidjson::Document();
			topic_json->Parse(response.c_str());

			int topic_id = (*topic_json)["id"].GetInt();
			int reported_post_count = (*topic_json)["posts_count"].GetInt();

			std::string post_directory = topic_dir_base + std::to_string(topic_id) + "/posts/";
			DDL::Utils::IO::ValidatePath(post_directory);

			DDL::Utils::IO::CreateNewFile(topic_dir_base + std::to_string(topic_id) + "/topic.json", response);

			rapidjson::GenericArray topic_post_ids = (*topic_json)["post_stream"]["stream"].GetArray();

			if (topic_post_ids.Size() > 20)
			{
				std::string post_chunk_local_base = topic_dir_base + std::to_string(topic_id) + "/post_chunk_";
				std::string post_chunk_url_base = topic_posts_url_format;
				{
					post_chunk_url_base = DDL::Utils::String::Replace(post_chunk_url_base, "<BASE_URL>", config->website_url);
					post_chunk_url_base = DDL::Utils::String::Replace(post_chunk_url_base, "<TOPIC_ID>", std::to_string(topic_id));
				}

				std::vector<topic_post_chunk> post_chunks = std::vector<topic_post_chunk>();
				{
					std::vector<int> post_ids = std::vector<int>();

					for (int i = 0; i < topic_post_ids.Size(); i++)
					{
						post_ids.push_back(topic_post_ids[i].GetInt());
					}

					while (post_ids.size() > 20)
					{
						topic_post_chunk chunk = topic_post_chunk();

						for (int i = (post_ids.size() - 1); i != 0; i--)
						{
							chunk.push_back(post_ids[i]);
							post_ids.erase(post_ids.begin() + i);

							if (chunk.size() == 20)
							{
								break;
							}
						}

						post_chunks.push_back(chunk);
					}

					post_chunks.push_back(post_ids);
				}

				int collected_post_count = 0;

				for (topic_post_chunk chunk : post_chunks)
				{
					std::string chunk_request_suffix = "";

					for (int i = 0; i < chunk.size(); i++)
					{
						chunk_request_suffix += "post_ids[]=" + std::to_string(chunk[i]);

						if (i != (chunk.size() - 1))
						{
							chunk_request_suffix += "&";
						}
					}

					int chunk_http_code = -1;
					std::string chunk_response = DDL::Utils::Network::PerformHTTPRequestWithRetries(post_chunk_url_base + chunk_request_suffix, &chunk_http_code);

					if (chunk_http_code == 200)
					{
						rapidjson::Document chunk_document = rapidjson::Document();
						chunk_document.Parse(chunk_response.c_str());

						rapidjson::GenericArray posts_json = chunk_document["post_stream"]["posts"].GetArray();

						for (int i = 0; i < posts_json.Size(); i++)
						{
							rapidjson::Value post = posts_json[i].GetObj();
							int post_id = post["id"].GetInt();

							std::string post_json_string = DDL::Utils::Json::Serialize(&post);

							DDL::Utils::IO::CreateNewFile(post_directory + std::to_string(post_id) + ".json", post_json_string);
							collected_post_count++;
						}
					}
					else
					{
						// deal with this later
					}
				}

				if (collected_post_count != reported_post_count)
				{
					DDL::Logger::LogEvent("collected post count does not match topic reported post count, some posts may be missed!", DDLLogLevel::Warning);
					DDL::Logger::LogEvent("- collected            : " + std::to_string(collected_post_count), DDLLogLevel::Warning);
					DDL::Logger::LogEvent("- reported posts_count : " + std::to_string(reported_post_count), DDLLogLevel::Warning);
				}
			}
			else
			{
				rapidjson::GenericArray posts_json = (*topic_json)["post_stream"]["posts"].GetArray();

				for (int i = 0; i < posts_json.Size(); i++)
				{
					rapidjson::Value post = posts_json[i].GetObj();
					int post_id = post["id"].GetInt();

					std::string post_json_string = DDL::Utils::Json::Serialize(&post);

					DDL::Utils::IO::CreateNewFile(post_directory + std::to_string(post_id) + ".json", post_json_string);
				}
			}
		}
		else
		{
			// @todo: deal with this later
		}

		if (requests_until_next_notify <= 0)
		{
			requests_until_next_notify = config->topic_url_collection_notify_interval;
			DDL::Logger::LogEvent("saved " + std::to_string(ti) + "/" + std::to_string(topic_url_list->size()) + " topics so far...");
		}
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

	// Download external files for category
	{
		if (category->json_file->HasMember("uploaded_logo"))
		{
			if (!(*(category->json_file))["uploaded_logo"].IsNull())
			{
				int logo_http_code = -1;
				std::string logo_url = "https:" + std::string((*(category->json_file))["uploaded_logo"]["url"].GetString());
				std::string logo_file_path = category_directory + "logo" + logo_url.substr(logo_url.find_last_of('.'));

				std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(logo_url, &logo_http_code);

				if (logo_http_code == 200)
				{
					if (!DDL::Utils::IO::CreateNewFileBinaryMode(logo_file_path, response))
					{
						DDL::Logger::LogEvent("error writing logo file for category with id '"
							+ std::to_string(category->category_id) + "', category logo will not be saved", DDLLogLevel::Error);
					}
				}
				else
				{
					DDLDownloadRetryInfo* retry_info = new DDLDownloadRetryInfo();
					{
						retry_info->local_path = logo_file_path;
						retry_info->url = logo_url;
					}

					pending_retries.push_back(retry_info);
				}
			}
		}
	}

	std::vector<std::string> topic_urls = std::vector<std::string>();

	// Build complete topic list
	{
		DDL::Logger::LogEvent("building topic url list for category " + std::to_string(category->category_id) + ", this may take a while...");

		//"<BASE_URL>/c/<CAT_SLUG>/<CAT_ID>.json?page=<PAGE>";
		std::string topic_fetch_url_base = topic_list_url_format;
		{
			topic_fetch_url_base = DDL::Utils::String::Replace(topic_fetch_url_base, "<BASE_URL>", config->website_url);
			topic_fetch_url_base = DDL::Utils::String::Replace(topic_fetch_url_base, "<CAT_SLUG>", (*category->json_file)["slug"].GetString());
			topic_fetch_url_base = DDL::Utils::String::Replace(topic_fetch_url_base, "<CAT_ID>", std::to_string(category->category_id));
		}

		std::string local_json_dir = category_directory + "topic_pages/";
		bool has_more_topics = true;
		int topic_list_page = 0;

		DDL::Utils::IO::ValidatePath(local_json_dir);

		int requests_until_next_notify = config->topic_url_collection_notify_interval;

		while (has_more_topics)
		{
			std::string topic_fetch_url = topic_fetch_url_base + std::to_string(topic_list_page);
			int http_code = -1;

			std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(topic_fetch_url, &http_code);

			if (http_code == 200)
			{
				DDL::Utils::IO::CreateNewFile(local_json_dir + std::to_string(topic_list_page) + ".json", response);

				rapidjson::Document list_document = rapidjson::Document();
				list_document.Parse(response.c_str());

				rapidjson::Value topic_list_json = list_document["topic_list"].GetObj();

				if (!topic_list_json.HasMember("more_topics_url"))
				{
					has_more_topics = false;
				}

				rapidjson::GenericArray topics_json = topic_list_json["topics"].GetArray();

				for (int i = 0; i < topics_json.Size(); i++)
				{
					rapidjson::Value topic_info_json = topics_json[i].GetObj();

					std::string topic_info_url = topic_info_url_format;
					{
						topic_info_url = DDL::Utils::String::Replace(topic_info_url, "<BASE_URL>", config->website_url);
						topic_info_url = DDL::Utils::String::Replace(topic_info_url, "<TOPIC_ID>", std::to_string(topic_info_json["id"].GetInt()));
					}

					topic_urls.push_back(topic_info_url);
				}
			}
			else if (http_code == 301)
			{
				topic_list_page = 0;
				topic_urls.clear();
				DDL::Logger::LogEvent("got http 301, resetting page url back to 0 and clearing existing urls");
			}
			else
			{
				DDLDownloadRetryInfo* retry_info = new DDLDownloadRetryInfo();
				{
					retry_info->local_path = local_json_dir + std::to_string(topic_list_page) + ".json";
					retry_info->url = topic_fetch_url;
				}

				pending_retries.push_back(retry_info);

				// will retry page json later
			}

			if (requests_until_next_notify <= 0)
			{
				requests_until_next_notify = config->topic_url_collection_notify_interval;
				DDL::Logger::LogEvent("collected " + std::to_string(topic_urls.size()) + "/"
					+ std::to_string((*category->json_file)["topic_count"].GetInt()) + " topic urls so far...");
			}

			topic_list_page++;
			requests_until_next_notify--;
		}

		DDL::Logger::LogEvent("finished topic url list for category " + std::to_string(category->category_id)
			+ ", collected " + std::to_string(topic_urls.size()) + "topics");

		if (topic_urls.size() != (*category->json_file)["topic_count"].GetInt())
		{
			DDL::Logger::LogEvent("collected url count does not match topic_count from category json, some topics may be missed!", DDLLogLevel::Warning);
			DDL::Logger::LogEvent("- url count   : " + std::to_string(topic_urls.size()), DDLLogLevel::Warning);
			DDL::Logger::LogEvent("- topic_count : " + std::to_string((*category->json_file)["topic_count"].GetInt()), DDLLogLevel::Warning);
		}
	}

	// download each topic within category
	download_category_topics(category, &topic_urls);

	DDL::Logger::LogEvent("finished downloading category with id '" + std::to_string(category->category_id) + "'");
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