#include "discourse.h"

#include <map>

#include "components/3rdparty/rapidjson/document.h"

#include "components/utils/json/json.h"
#include "components/utils/network/network.h"
#include "components/utils/string/string.h"
#include "components/utils/io/io.h"
#include "components/settings/settings.h"
#include "components/diagnostics/logger/logger.h"
#include "components/utils/converters/converters.h"

std::vector<DiscourseCategory*> downloaded_categories = std::vector<DiscourseCategory*>();
DDLResumeInfo download_resume_info = DDLResumeInfo();
DDLResumeInfo new_resume_file = DDLResumeInfo();

std::string category_list_url_format = "<BASE_URL>/categories.json?include_subcategories=true";
std::string category_info_url_format = "<BASE_URL>/c/<CAT_ID>/show.json";

std::string topic_list_url_format = "<BASE_URL>/c/<CAT_SLUG>/<CAT_ID>.json?page=";
std::string topic_info_url_format = "<BASE_URL>/t/<TOPIC_ID>.json";
std::string topic_posts_url_format = "<BASE_URL>/t/<TOPIC_ID>/posts.json?";

std::string user_list_url_format = "<BASE_URL>/directory_items.json?period=all&page=";

std::string json_category_info_path = "<JSON_ROOT>/c/<CAT_ID>/";
std::string data_cache_entry_format = "<URL>|<TOPIC_ID>|<POST_COUNT>|<POST_IDS>";

//post_ids[]=<POST_ID>

typedef std::vector<int> topic_post_chunk;

DDLResult download_topic_posts(DiscourseTopic* topic)
{
	if (!topic)
	{
		DDL::Logger::LogEvent("tried to download topics, but topic was nullptr - skipping topic", DDLLogLevel::Error);
		return DDLResult::Error_NullPointer;
	}

	// trim post stream to remove first 20 items, keep first 20 from topic json

	// if needed, download all remaining posts
}

void save_resume_file()
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("failed to save resume info - could not get website config!", DDLLogLevel::Error);
		return;
	}

	std::string resume_file_contents = "";
	{
		resume_file_contents += "category_id=" + std::to_string(new_resume_file.category_id) + "\n";
		resume_file_contents += "last_saved_topic=" + std::to_string(new_resume_file.last_saved_topic) + "\n";
		resume_file_contents += "last_user_id=" + std::to_string(new_resume_file.last_user_id) + "\n";
		
		if (new_resume_file.download_step == DDLResumeInfo::DownloadStep::TOPICS)
		{
			resume_file_contents += "download_step=TOPICS";
		}
		else if (new_resume_file.download_step == DDLResumeInfo::DownloadStep::USERS)
		{
			resume_file_contents += "download_step=USERS";
		}
		else
		{
			resume_file_contents += "download_step=INVALID";
		}
	}

	DDL::Utils::IO::CreateNewFile(config->site_directory_root + "/resume", resume_file_contents);
}

DDLResult download_category_topics(DiscourseCategory* category, std::map<int, std::string>* topic_url_list)
{
	bool incomplete_download = false;

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

	new_resume_file.download_step = DDLResumeInfo::DownloadStep::TOPICS;
	new_resume_file.category_id = category->category_id;

	std::string topic_dir_base = json_category_info_path + "topics/";
	{
		topic_dir_base = DDL::Utils::String::Replace(topic_dir_base, "<JSON_ROOT>", config->json_path);
		topic_dir_base = DDL::Utils::String::Replace(topic_dir_base, "<CAT_ID>", std::to_string(category->category_id));
	}

	DDL::Utils::IO::ValidatePath(topic_dir_base);

	int requests_until_next_notify = config->topic_url_collection_notify_interval;

	std::map<int, std::string>::iterator it;
	bool found_resume_starting_point = false;
	int ti = -1;

	for (it = topic_url_list->begin(); it != topic_url_list->end(); it++)
	{
		ti++;

		if (download_resume_info.download_step == DDLResumeInfo::DownloadStep::TOPICS)
		{
			if (!found_resume_starting_point)
			{
				if (category->category_id == download_resume_info.category_id)
				{
					if (download_resume_info.last_saved_topic == it->first)
					{
						found_resume_starting_point = true;
						DDL::Logger::LogEvent("resuming topic download in category " + std::to_string(category->category_id)
							+ " at topic " + std::to_string(it->first));
					}
					else
					{
						continue;
					}
				}
				else
				{
					found_resume_starting_point = true;
				}
			}
		}
		else
		{
			found_resume_starting_point = true;
		}

		std::string topic_url = it->second;

		int http_code = -1;
		std::string response = DDL::Utils::Network::PerformHTTPRequestWithRetries(topic_url, &http_code);

		if (http_code == 200)
		{
			DiscourseTopic* topic = new DiscourseTopic();

			rapidjson::Document* topic_json = new rapidjson::Document();
			topic_json->Parse(response.c_str());

			int topic_id = (*topic_json)["id"].GetInt();
			int reported_post_count = (*topic_json)["posts_count"].GetInt();

			std::string topic_directory = topic_dir_base + std::to_string(topic_id) + "/";
			std::string post_directory = topic_directory + "posts/";

			if (config->download_skip_existing_topics)
			{
				if (DDL::Utils::IO::IsDirectory(topic_directory)
					&& DDL::Utils::IO::FileExists(topic_directory + "topic.json")
					&& DDL::Utils::IO::IsDirectory(topic_directory + "posts/"))
				{
					DDL::Logger::LogEvent("skipping topic " + std::to_string(topic->topic_id)
						+ " as it appears to already exist (note: some posts could be missing in this case)");
					continue;
				}
			}

			DDL::Utils::IO::ValidatePath(post_directory);

			DDL::Utils::IO::CreateNewFile(topic_directory + "topic.json", response);

			rapidjson::GenericArray topic_post_ids = (*topic_json)["post_stream"]["stream"].GetArray();

			if (topic_post_ids.Size() > 20)
			{
				std::string post_chunk_local_base = topic_directory + "chunks/post_chunk_";
				std::string post_chunk_url_base = topic_posts_url_format;
				{
					post_chunk_url_base = DDL::Utils::String::Replace(post_chunk_url_base, "<BASE_URL>", config->website_url);
					post_chunk_url_base = DDL::Utils::String::Replace(post_chunk_url_base, "<TOPIC_ID>", std::to_string(topic_id));
				}

				DDL::Utils::IO::ValidatePath(topic_directory + "chunks/");

				std::vector<topic_post_chunk> post_chunks = std::vector<topic_post_chunk>();
				{
					std::vector<int> post_ids = std::vector<int>();

					for (int i = 0; i < topic_post_ids.Size(); i++)
					{
						if (config->download_skip_existing_posts)
						{
							if (DDL::Utils::IO::IsDirectory(post_directory)
								&& DDL::Utils::IO::FileExists(post_directory + std::to_string(topic_post_ids[i].GetInt()) + ".json"))
							{
								DDL::Logger::LogEvent("skipping post " + std::to_string(topic_post_ids[i].GetInt())
									+ " as it appears to already exist");
								continue;
							}
						}

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

				int chunk_index = -1;
				for (topic_post_chunk chunk : post_chunks)
				{
					chunk_index++;
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
						DDL::Utils::IO::CreateNewFile(post_chunk_local_base + std::to_string(chunk_index) + ".json", chunk_response);

						rapidjson::Document chunk_document = rapidjson::Document();
						chunk_document.Parse(chunk_response.c_str());

						rapidjson::GenericArray posts_json = chunk_document["post_stream"]["posts"].GetArray();

						for (int i = 0; i < posts_json.Size(); i++)
						{
							rapidjson::Value post = posts_json[i].GetObj();
							int post_id = post["id"].GetInt();

							std::string post_json_string = DDL::Utils::Json::Serialize(&post);

							DDL::Utils::IO::CreateNewFile(post_directory + std::to_string(post_id) + ".json", post_json_string);

							rapidjson::Document* post_json_file = new rapidjson::Document();
							post_json_file->Parse(post_json_string.c_str());

							topic->posts.push_back(post_id);

							delete post_json_file;
							collected_post_count++;
						}
					}
					else
					{
						DDL::Logger::LogEvent("got http " + std::to_string(chunk_http_code) + " while trying to download post chunk #"
							+ std::to_string(chunk_index) + ", these posts will NOT be downloaded!", DDLLogLevel::Error);
						incomplete_download = true;
					}
				}

				bool post_count_mismatch = false;

				if (config->strict_topic_count_checks)
				{
					if (collected_post_count != reported_post_count)
					{
						post_count_mismatch = true;
					}
				}
				else
				{
					if (collected_post_count < reported_post_count)
					{
						post_count_mismatch = true;
					}
				}

				if (post_count_mismatch)
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

					rapidjson::Document* post_json_file = new rapidjson::Document();
					post_json_file->Parse(post_json_string.c_str());

					topic->posts.push_back(post_id);

					delete post_json_file;
				}
			}

			topic->topic_id = topic_id;
			topic->request_url = topic_url;
			topic->posts_count = (*topic_json)["posts_count"].GetInt();

			category->topics.push_back(topic);
			delete topic_json;

			new_resume_file.last_saved_topic = topic_id;
			save_resume_file();
		}
		else
		{
			DDL::Logger::LogEvent("got http " + std::to_string(http_code) + " while trying to download topic from url '"
				+ topic_url + "', this topic will NOT be downloaded!", DDLLogLevel::Error);
			incomplete_download = true;
		}

		requests_until_next_notify--;

		if (requests_until_next_notify <= 0)
		{
			if (found_resume_starting_point)
			{
				requests_until_next_notify = config->topic_url_collection_notify_interval;
				DDL::Logger::LogEvent("saved " + std::to_string(ti) + "/" + std::to_string(topic_url_list->size()) + " topics so far...");
			}
		}
	}

	if (incomplete_download)
	{
		DDL::Logger::LogEvent("some topics were not downloaded, you should probably retry these topics later", DDLLogLevel::Warning);
		return DDLResult::Error_IncompleteDownload;
	}

	return DDLResult::Success_OK;
}

DDLResult download_category(DiscourseCategory* category)
{
	bool incomplete_download = false;

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

	if (config->download_skip_existing_categories)
	{
		if (DDL::Utils::IO::IsDirectory(category_directory)
			&& DDL::Utils::IO::FileExists(category_directory + "show.json")
			&& DDL::Utils::IO::IsDirectory(category_directory + "topics/"))
		{
			DDL::Logger::LogEvent("skipping category " + std::to_string(category->category_id)
				+ " as it appears to already exist (note: some topics could be missing in this case)");
			return DDLResult::Success_OK;
		}
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
						incomplete_download = true;
					}
				}
				else
				{
					DDL::Logger::LogEvent("got http " + std::to_string(logo_http_code) + " while downloading logo for category "
						+ std::to_string(category->category_id) + ", category logo will not be saved", DDLLogLevel::Error);
					incomplete_download = true;
				}
			}
		}
	}

	std::map<int, std::string> topic_urls = std::map<int, std::string>();
	bool needs_url_list_download = true;

	if (config->enable_url_caching)
	{
		if (DDL::Utils::IO::IsFile(category_directory + "urlcache"))
		{
			bool url_cache_valid = true;

			std::vector<std::string> topic_url_items = DDL::Utils::IO::GetFileContentsAsLines(category_directory + "urlcache");

			for (std::string item : topic_url_items)
			{
				std::vector<std::string> components = DDL::Utils::String::Split(item, "|");

				if (components.size() != 2)
				{
					DDL::Logger::LogEvent("url cache contains an invalid entry - url cache will be ignored (contains more than 2 components)!",
						DDLLogLevel::Warning);
					url_cache_valid = false;
					break;
				}

				if (!DDL::Converters::IsStringInt(components.at(0)))
				{
					DDL::Logger::LogEvent("url cache contains an invalid entry - url cache will be ignored (topic id was invalid)!", DDLLogLevel::Warning);
					url_cache_valid = false;
					break;
				}

				topic_urls.insert(std::pair<int, std::string>(DDL::Converters::StringToInt(components.at(0)), components.at(1)));
			}

			if (url_cache_valid)
			{
				DDL::Logger::LogEvent("successfully loaded " + std::to_string(topic_urls.size()) + " topic urls from urlcache, skipping url fetching");
				needs_url_list_download = false;
			}
		}
	}

	// Build complete topic list if url cache doesnt exist/isn't enabled
	if (needs_url_list_download)
	{
		DDL::Logger::LogEvent("building topic url list for category " + std::to_string(category->category_id) + ", this may take a while...");

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

					int topic_category_id = topic_info_json["category_id"].GetInt();
					if (!config->download_subcategory_topics && topic_category_id != category->category_id)
					{
						continue;
					}

					std::string topic_info_url = topic_info_url_format;
					{
						topic_info_url = DDL::Utils::String::Replace(topic_info_url, "<BASE_URL>", config->website_url);
						topic_info_url = DDL::Utils::String::Replace(topic_info_url, "<TOPIC_ID>", std::to_string(topic_info_json["id"].GetInt()));
					}

					if (!topic_urls.contains(topic_info_json["id"].GetInt()))
					{
						topic_urls.insert(std::pair<int, std::string>(topic_info_json["id"].GetInt(), topic_info_url));
					}
					else
					{
						DDL::Logger::LogEvent("not adding topic url '" + topic_info_url + "' to topic list because the list already contains that url");
					}
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
				DDL::Logger::LogEvent("got http " + std::to_string(http_code) + ", some topics will be missed!", DDLLogLevel::Error);
				incomplete_download = true;
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
			+ ", collected " + std::to_string(topic_urls.size()) + " topics");

		bool topic_count_mismatch = false;

		if (config->strict_topic_count_checks)
		{
			if (topic_urls.size() != (*category->json_file)["topic_count"].GetInt())
			{
				topic_count_mismatch = true;
			}
		}
		else
		{
			if (topic_urls.size() < (*category->json_file)["topic_count"].GetInt())
			{
				topic_count_mismatch = true;
			}
		}

		if (topic_count_mismatch)
		{
			DDL::Logger::LogEvent("collected url count does not match topic_count from category json, some topics may be missed!", DDLLogLevel::Warning);
			DDL::Logger::LogEvent("- url count   : " + std::to_string(topic_urls.size()), DDLLogLevel::Warning);
			DDL::Logger::LogEvent("- topic_count : " + std::to_string((*category->json_file)["topic_count"].GetInt()), DDLLogLevel::Warning);
		}
	}

	// Write topic URL list to disk to avoid redownloading topic list later
	if (config->enable_url_caching)
	{
		DDL::Logger::LogEvent("saving url cache for category " + std::to_string(category->category_id) + "...");

		std::string url_cache_file_contents = "";

		std::map<int, std::string>::iterator it;

		for (it = topic_urls.begin(); it != topic_urls.end(); it++)
		{
			url_cache_file_contents += std::to_string(it->first) + "|" + it->second + "\n";
		}

		bool cache_result = DDL::Utils::IO::CreateNewFile(category_directory + "urlcache", url_cache_file_contents);

		if (cache_result)
		{
			DDL::Logger::LogEvent("url cache save finished");
		}
		else
		{
			DDL::Logger::LogEvent("failed to save url cache, topic urls will have to be redownloaded again in the case of an interrupted download",
				DDLLogLevel::Warning);
		}
	}

	download_category_topics(category, &topic_urls);

	// Write topic data to disk so we can free up memory
	if (config->enable_data_caching)
	{
		DDL::Logger::LogEvent("saving data cache for category " + std::to_string(category->category_id) + "...");

		std::string data_cache_contents = "";

		for (DiscourseTopic* topic : category->topics)
		{
			std::string cache_entry = data_cache_entry_format;
			
			cache_entry = DDL::Utils::String::Replace(cache_entry, "<URL>", topic->request_url);
			cache_entry = DDL::Utils::String::Replace(cache_entry, "<TOPIC_ID>", std::to_string(topic->topic_id));
			cache_entry = DDL::Utils::String::Replace(cache_entry, "<POST_COUNT>", std::to_string(topic->posts_count));

			std::string post_id_list = "";
			{
				for (int post_id : topic->posts)
				{
					post_id_list += std::to_string(post_id) + ",";
				}

				if (post_id_list.ends_with(","))
				{
					post_id_list = post_id_list.substr(0, post_id_list.length() - 1);
				}
			}

			cache_entry = DDL::Utils::String::Replace(cache_entry, "<POST_IDS>", post_id_list);

			data_cache_contents += cache_entry + "\n";
		}

		bool cache_result = DDL::Utils::IO::CreateNewFile(category_directory + "datacache", data_cache_contents);

		if (cache_result)
		{
			for (DiscourseTopic* topic : category->topics)
			{
				topic->posts.clear();
				delete topic;
			}

			category->topics.clear();

			DDL::Logger::LogEvent("data cache save finished");
		}
		else
		{
			DDL::Logger::LogEvent("failed to save data cache - topic data will NOT be unloaded from memory, this could result in high memory usage",
				DDLLogLevel::Warning);
		}
	}

	DDL::Logger::LogEvent("finished downloading category with id '" + std::to_string(category->category_id) + "'");

	if (incomplete_download)
	{
		DDL::Logger::LogEvent("some content was not downloaded, you should probably retry this category later", DDLLogLevel::Warning);
		return DDLResult::Error_IncompleteDownload;
	}

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

		if (config->use_category_id_filter)
		{
			bool download_category = false;

			if (config->use_filter_as_blacklist)
			{
				download_category = true;

				for (int id : config->category_id_whitelist)
				{
					if (id == category->category_id)
					{
						download_category = false;
						break;
					}
				}
			}
			else
			{
				for (int id : config->category_id_whitelist)
				{
					if (id == category->category_id)
					{
						download_category = true;
						break;
					}
				}
			}

			if (!download_category)
			{
				DDL::Logger::LogEvent("skipping category with id '" + std::to_string(category->category_id)
					+ "' because category whitelist is enabled. if this is not what you want, please edit your website.cfg");

				delete category->json_file;
				delete category;
				continue;
			}
		}

		// Get extra category info
		{
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

DDLResumeInfo get_resume_info(bool* result)
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get download resume info, configuration file was nullptr - download will NOT be resumed!", DDLLogLevel::Error);

		if (result)
		{
			*result = false;
		}

		return DDLResumeInfo();
	}

	if (!DDL::Utils::IO::IsFile(config->site_directory_root + "/resume"))
	{
		DDL::Logger::LogEvent("could not get download resume info, no resume information found - download will NOT be resumed! "
			"(note: this is normal when starting a new download)", DDLLogLevel::Warning);

		if (result)
		{
			*result = false;
		}

		return DDLResumeInfo();
	}

	std::vector<std::string> resume_info_lines = DDL::Utils::IO::GetFileContentsAsLines(config->site_directory_root + "/resume");

	DDLResumeInfo resume_info = DDLResumeInfo();

	for (std::string line : resume_info_lines)
	{
		if (line.starts_with("category_id="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "category_id=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				resume_info.category_id = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("last_saved_topic="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "last_saved_topic=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				resume_info.last_saved_topic = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("last_user_id="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "last_user_id=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				resume_info.last_user_id = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("download_step="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "download_step=", "");

			if (DDL::Utils::String::ToLower(line_value) == "topics")
			{
				resume_info.download_step = DDLResumeInfo::DownloadStep::TOPICS;
			}
			else if (DDL::Utils::String::ToLower(line_value) == "users")
			{
				resume_info.download_step = DDLResumeInfo::DownloadStep::USERS;
			}
		}
	}

	if (resume_info.download_step != DDLResumeInfo::DownloadStep::INVALID)
	{
		if (resume_info.download_step == DDLResumeInfo::DownloadStep::TOPICS)
		{
			if (resume_info.category_id != -1 && resume_info.last_saved_topic)
			{
				if (result)
				{
					*result = true;
				}

				return resume_info;
			}
		}
		else if (resume_info.download_step == DDLResumeInfo::DownloadStep::TOPICS)
		{
			if (resume_info.last_user_id != -1)
			{
				if (result)
				{
					*result = true;
				}

				return resume_info;
			}
		}
	}

	DDL::Logger::LogEvent("!!!!!! could not parse download resume information file - DOWNLOAD WILL BE RESTARTED!", DDLLogLevel::Error);
	DDL::Logger::LogEvent("!!!!!! you have 10 seconds to abort the application now if you would like to perform additional diagnosis or back up the existing download folder!", DDLLogLevel::Error);
	Sleep(10 * 1000);

	if (result)
	{
		*result = false;
	}

	return DDLResumeInfo();
}

void load_category_data_cache(DiscourseCategory* category, WebsiteConfig* config)
{
	if (config->enable_data_caching)
	{
		DDL::Logger::LogEvent("loading data from cache...");

		std::string cache_path = json_category_info_path + "datacache";
		{
			cache_path = DDL::Utils::String::Replace(cache_path, "<JSON_ROOT>", config->json_path);
			cache_path = DDL::Utils::String::Replace(cache_path, "<CAT_ID>", std::to_string(category->category_id));
		}

		if (DDL::Utils::IO::IsFile(cache_path))
		{
			std::vector<std::string> cache_entries = DDL::Utils::IO::GetFileContentsAsLines(cache_path);

			for (std::string cache_entry : cache_entries)
			{
				bool cache_entry_valid = true;

				std::vector<std::string> components = DDL::Utils::String::Split(cache_entry, "|");

				if (components.size() != 4)
				{
					cache_entry_valid = false;
					DDL::Logger::LogEvent("data cache entry has an invalid number of components (expected 4, got "
						+ std::to_string(components.size()) + ")", DDLLogLevel::Error);
					continue;
				}

				if (!DDL::Converters::IsStringInt(components.at(1)) || !DDL::Converters::IsStringInt(components.at(2)))
				{
					cache_entry_valid = false;
					DDL::Logger::LogEvent("data cache entry has an invalid topic id and/or post count", DDLLogLevel::Error);
					continue;
				}

				std::vector<int> post_ids = std::vector<int>();
				std::vector<std::string> post_ids_str = DDL::Utils::String::Split(components.at(3), ",");

				for (std::string post_id_str : post_ids_str)
				{
					if (DDL::Converters::IsStringInt(post_id_str))
					{
						post_ids.push_back(DDL::Converters::StringToInt(post_id_str));
					}
					else
					{
						cache_entry_valid = false;
						DDL::Logger::LogEvent("data cache entry has an invalid post id in post id list", DDLLogLevel::Error);
					}
				}

				std::string topic_url = components.at(0);
				int topic_id = DDL::Converters::StringToInt(components.at(1));
				int post_count = DDL::Converters::StringToInt(components.at(2));

				if (post_ids.size() != post_count)
				{
					cache_entry_valid = false;
					DDL::Logger::LogEvent("data cache entry has a post count mismatch - reported post count was "
						+ std::to_string(post_count) + ", post id count was " + std::to_string(post_ids.size()), DDLLogLevel::Error);
				}

				if (cache_entry_valid)
				{
					DiscourseTopic* topic = new DiscourseTopic();

					topic->request_url = topic_url;
					topic->topic_id = topic_id;
					topic->posts_count = post_count;
					topic->posts = post_ids;

					category->topics.push_back(topic);
				}
			}
		}
		else
		{
			DDL::Logger::LogEvent("category " + std::to_string(category->category_id) + " is missing a data cache file, "
				"category cannot be verified - this may result in an incomplete download!", DDLLogLevel::Error);
		}
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

	if (config->resume_download)
	{
		bool resume_info_result = false;
		DDLResumeInfo resume_info = get_resume_info(&resume_info_result);

		if (resume_info_result)
		{
			DDL::Logger::LogEvent("resuming previous download...");

			download_resume_info = resume_info;

			if (downloaded_categories.size() > 0)
			{
				while (downloaded_categories.at(0)->category_id != resume_info.category_id)
				{
					DDL::Logger::LogEvent("category " + std::to_string(downloaded_categories.at(0)->category_id)
						+ " will be skipped as it seems to already be downloaded");

					delete downloaded_categories.at(0);
					downloaded_categories.erase(downloaded_categories.begin() + 0);
				}
			}
		}
	}

	for (DiscourseCategory* category : downloaded_categories)
	{
		download_category(category);
	}

	if (config->sanity_check_on_finish)
	{
		DDL::Logger::LogEvent("performing sanity check on existing data...");

		// check that stored counts match up with category topic/post counts

		for (DiscourseCategory* category : downloaded_categories)
		{
			bool category_status = true;
			int errored_topics = 0;

			DDL::Logger::LogEvent("checking category " + std::to_string(category->category_id) + "...");

			load_category_data_cache(category, config);

			int saved_topic_count = category->topics.size();
			int reported_topic_count = (*category->json_file)["topic_count"].GetInt();

			for (DiscourseTopic* topic : category->topics)
			{
				int reported_post_count = topic->posts_count;
				int saved_post_count = topic->posts.size();

				if (saved_post_count != reported_post_count)
				{
					DDL::Logger::LogEvent("topic with id " + std::to_string(topic->topic_id) + " has a mismatched post count!", DDLLogLevel::Warning);
					DDL::Logger::LogEvent("- reported post count : " + std::to_string(reported_post_count), DDLLogLevel::Warning);
					DDL::Logger::LogEvent("- saved post count    : " + std::to_string(saved_post_count), DDLLogLevel::Warning);
					category_status = false;
					errored_topics++;
				}
			}

			bool topic_count_mismatch = false;

			if (config->strict_topic_count_checks)
			{
				if (saved_topic_count != reported_topic_count)
				{
					topic_count_mismatch = true;
				}
			}
			else
			{
				if (saved_topic_count < reported_topic_count)
				{
					topic_count_mismatch = true;
				}
			}

			if (topic_count_mismatch)
			{
				category_status = false;
			}

			if (!category_status)
			{
				DDL::Logger::LogEvent("finished checking category " + std::to_string(category->category_id) + ", but found errors:");

				if (errored_topics > 0)
				{
					DDL::Logger::LogEvent("- a total of " + std::to_string(errored_topics) + " topics had a mismatched post count");
				}

				if (saved_topic_count != reported_topic_count)
				{
					DDL::Logger::LogEvent("- category has a topic count mismatch:");
					DDL::Logger::LogEvent("  - reported topic count : " + std::to_string(reported_topic_count));
					DDL::Logger::LogEvent("  - saved topic count    : " + std::to_string(saved_topic_count));
				}
			}
			else
			{
				DDL::Logger::LogEvent("finished checking category " + std::to_string(category->category_id) + " and found no errors");
			}
		}

		DDL::Logger::LogEvent("sanity check finished");

		if (config->thorough_sanity_check)
		{
			DDL::Logger::LogEvent("performing in-depth sanity check, this may take a while...");

			int total_incomplete_categories = 0;
			int total_missing_categories = 0;
			int total_missing_topics = 0;
			int total_missing_posts = 0;

			std::vector<DiscourseCategory*> redownload_categories = std::vector<DiscourseCategory*>();

			// go through each topic in each category and verify that each individual post json file exists
			for (DiscourseCategory* category : downloaded_categories)
			{
				load_category_data_cache(category, config);

				std::vector<DiscourseTopic*> redownload_topics = std::vector<DiscourseTopic*>();
				bool missing_content = false;

				std::string category_root = json_category_info_path;
				{
					category_root = DDL::Utils::String::Replace(category_root, "<JSON_ROOT>", config->json_path);
					category_root = DDL::Utils::String::Replace(category_root, "<CAT_ID>", std::to_string(category->category_id));
				}

				if (!DDL::Utils::IO::IsDirectory(category_root))
				{
					DDL::Logger::LogEvent("category " + std::to_string(category->category_id)
						+ " does not appear to be downloaded, will attempt to redownload", DDLLogLevel::Warning);

					total_missing_categories++;
					redownload_categories.push_back(category);
				}
				else if (!DDL::Utils::IO::IsFile(category_root + "show.json"))
				{
					DDL::Logger::LogEvent("category " + std::to_string(category->category_id)
						+ " missing category information json, will attempt to redownload", DDLLogLevel::Warning);

					total_missing_categories++;
					redownload_categories.push_back(category);
				}

				bool topic_count_mismatch = false;

				if (config->strict_topic_count_checks)
				{
					if (category->topics.size() != (*category->json_file)["topic_count"].GetInt())
					{
						topic_count_mismatch = true;
					}
				}
				else
				{
					if (category->topics.size() < (*category->json_file)["topic_count"].GetInt())
					{
						topic_count_mismatch = true;
					}
				}

				if (topic_count_mismatch)
				{
					DDL::Logger::LogEvent("category " + std::to_string(category->category_id) + " has a topic count mismatch, category will "
						"NOT be redownloaded automatically but you may want to using the category id whitelist options in website.cfg:", DDLLogLevel::Warning);
					DDL::Logger::LogEvent("- reported topic count : " + std::to_string((*category->json_file)["topic_count"].GetInt()), DDLLogLevel::Warning);
					DDL::Logger::LogEvent("- saved topic count    : " + std::to_string(category->topics.size()), DDLLogLevel::Warning);

					missing_content = true;
				}

				for (DiscourseTopic* topic : category->topics)
				{
					bool topic_missing_content = false;

					std::string topic_root = category_root + "topics/" + std::to_string(topic->topic_id) + "/";
					std::string posts_root = topic_root + "posts/";

					if (!DDL::Utils::IO::IsDirectory(topic_root))
					{
						DDL::Logger::LogEvent("topic " + std::to_string(topic->topic_id)
							+ " directory is missing, topic will be redownloaded", DDLLogLevel::Warning);

						missing_content = true;
						topic_missing_content = true;
						total_missing_topics++;
					}
					else if (!DDL::Utils::IO::IsFile(topic_root + "topic.json"))
					{
						DDL::Logger::LogEvent("topic " + std::to_string(topic->topic_id)
							+ " information file is missing, topic will be redownloaded", DDLLogLevel::Warning);

						missing_content = true;
						topic_missing_content = true;
						total_missing_topics++;
					}

					if (!DDL::Utils::IO::IsDirectory(posts_root))
					{
						DDL::Logger::LogEvent("topic " + std::to_string(topic->topic_id)
							+ " posts directory is missing, topic will be redownloaded", DDLLogLevel::Warning);

						missing_content = true;
						topic_missing_content = true;
						total_missing_posts += topic->posts_count;
					}

					bool post_count_mismatch = false;

					if (config->strict_topic_count_checks)
					{
						if (topic->posts.size() != topic->posts_count)
						{
							post_count_mismatch = true;
						}
					}
					else
					{
						if (topic->posts.size() < topic->posts_count)
						{
							post_count_mismatch = true;
						}
					}

					if (post_count_mismatch)
					{
						DDL::Logger::LogEvent("topic " + std::to_string(topic->topic_id)
							+ " has a post count mismatch, topic will be redownloaded:", DDLLogLevel::Warning);
						DDL::Logger::LogEvent("- reported post count : " + std::to_string(topic->posts_count), DDLLogLevel::Warning);
						DDL::Logger::LogEvent("- saved post count    : " + std::to_string(topic->posts.size()), DDLLogLevel::Warning);

						missing_content = true;
						topic_missing_content = true;
					}

					for (int post_id : topic->posts)
					{
						if (!DDL::Utils::IO::IsFile(posts_root + std::to_string(post_id) + ".json"))
						{
							DDL::Logger::LogEvent("topic " + std::to_string(topic->topic_id)
								+ " is missing post " + std::to_string(post_id) + ", topic will be redownloaded", DDLLogLevel::Warning);

							missing_content = true;
							topic_missing_content = true;
							total_missing_posts++;
						}
					}

					if (topic_missing_content)
					{
						redownload_topics.push_back(topic);
					}
				}

				if (redownload_topics.size() > 0)
				{
					DDL::Logger::LogEvent("attempting redownload of " + std::to_string(redownload_topics.size()) + " missing topics in category " + std::to_string(category->category_id)
						+ ", this may take a while depending on how many topics are missing...");

					std::map<int, std::string> redownload_topic_urls = std::map<int, std::string>();

					for (DiscourseTopic* topic : redownload_topics)
					{
						redownload_topic_urls.insert(std::pair<int, std::string>(topic->topic_id, topic->request_url));
					}

					download_category_topics(category, &redownload_topic_urls);

					DDL::Logger::LogEvent("finished redownloading missing topics in category " + std::to_string(category->category_id));
				}
			}

			if (redownload_categories.size() > 0)
			{
				DDL::Logger::LogEvent("attempting redownload of " + std::to_string(redownload_categories.size())
					+ " missing categories, this may take a while...");

				for (DiscourseCategory* category : redownload_categories)
				{
					download_category(category);
				}

				DDL::Logger::LogEvent("finished redownloading missing categories");
			}

			DDL::Logger::LogEvent("in-depth sanity check finished");
		}
	}

	// Release data
	{
		for (DiscourseCategory* category : downloaded_categories)
		{
			for (DiscourseTopic* topic : category->topics)
			{
				topic->posts.clear();
				delete topic;
			}

			category->topics.clear();
			delete category->json_file;
			delete category;
		}

		downloaded_categories.clear();
	}
}