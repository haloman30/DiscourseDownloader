#include "components/discourse/discourse.h"

#include "components/diagnostics/logger/logger.h"
#include "components/settings/settings.h"
#include "components/utils/io/io.h"
#include "components/utils/network/network.h"
#include "components/utils/string/string.h"
#include "components/utils/json/json.h"

DDLResult DDL::Discourse::Downloader::DownloadTopics(DiscourseCategory* category, std::map<int, std::string>* topic_url_list)
{
	bool incomplete_download = false;

	DDLResumeInfo* current_resume_info = DDL::Discourse::Downloader::GetCurrentResumeInfo();
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

	current_resume_info->download_step = DDLResumeInfo::DownloadStep::TOPICS;
	current_resume_info->category_id = category->category_id;

	std::string topic_dir_base = JSON_CATEGORY_ROOT_FORMAT + "topics/";
	{
		topic_dir_base = DDL::Utils::String::Replace(topic_dir_base, "<JSON_ROOT>", config->json_path);
		topic_dir_base = DDL::Utils::String::Replace(topic_dir_base, "<CAT_ID>", std::to_string(category->category_id));
	}

	DDL::Utils::IO::ValidatePath(topic_dir_base);

	current_resume_info->topic_first_id = topic_url_list->begin()->first;
	current_resume_info->topic_last_id = topic_url_list->end()->first;

	int requests_until_next_notify = config->topic_url_collection_notify_interval;

	std::map<int, std::string>::iterator it;
	bool found_resume_starting_point = false;
	int ti = -1;

	for (it = topic_url_list->begin(); it != topic_url_list->end(); it++)
	{
		ti++;

		if (DDL::Discourse::Downloader::GetLastResumeInfo() != nullptr)
		{
			DDLResumeInfo* last_resume_info = DDL::Discourse::Downloader::GetLastResumeInfo();

			if (last_resume_info->download_step == DDLResumeInfo::DownloadStep::TOPICS)
			{
				if (!found_resume_starting_point)
				{
					if (category->category_id == last_resume_info->category_id)
					{
						if (topic_url_list->begin()->first != last_resume_info->topic_first_id ||
							topic_url_list->end()->first != last_resume_info->topic_last_id)
						{
							found_resume_starting_point = true;
							DDL::Logger::LogEvent("cannot resume topic download, first/last topic ids in status do not match actual url list", DDLLogLevel::Warning);
						}

						if (ti == last_resume_info->topic_download_index)
						{
							found_resume_starting_point = true;

							if (last_resume_info->last_saved_topic == it->first)
							{
								DDL::Logger::LogEvent("resuming topic download in category " + std::to_string(category->category_id)
									+ " at topic " + std::to_string(it->first));
							}
							else
							{
								DDL::Logger::LogEvent("cannot resume topic download, topic at last saved index does not match real topic id", DDLLogLevel::Warning);
							}
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
				std::string post_chunk_url_base = TOPIC_POSTS_URL_FORMAT;
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

			current_resume_info->topic_download_index = ti;
			current_resume_info->last_saved_topic = topic_id;
			DDL::Discourse::Downloader::SaveResumeFile();
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