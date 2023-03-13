#pragma once

#include <vector>
#include <string>
#include <map>

#include "components/3rdparty/rapidjson/document.h"
#include "components/diagnostics/errors/errors.h"

#define category_list_url_format std::string("<BASE_URL>/categories.json?include_subcategories=true")
#define category_info_url_format std::string("<BASE_URL>/c/<CAT_ID>/show.json")

#define topic_list_url_format std::string("<BASE_URL>/c/<CAT_SLUG>/<CAT_ID>.json?page=")
#define topic_info_url_format std::string("<BASE_URL>/t/<TOPIC_ID>.json")
#define topic_posts_url_format std::string("<BASE_URL>/t/<TOPIC_ID>/posts.json?")

#define user_list_url_format std::string("<BASE_URL>/directory_items.json?period=all&page=")

#define json_category_info_path std::string("<JSON_ROOT>/c/<CAT_ID>/")
#define data_cache_entry_format std::string("<URL>|<TOPIC_ID>|<POST_COUNT>|<POST_IDS>")

typedef std::vector<int> topic_post_chunk;

struct DDLResumeInfo
{
	enum class DownloadStep
	{
		TOPICS,
		USERS,
		INVALID
	};

	int category_id = -1;
	int last_saved_topic = -1;
	int topic_first_id = -1;
	int topic_last_id = -1;
	int topic_download_index = -1;
	int last_user_id = -1;
	DownloadStep download_step = DownloadStep::INVALID;
};

struct DiscourseTopic
{
	std::string request_url = "";

	int topic_id = -1;
	int posts_count = -1;

	std::vector<int> posts = std::vector<int>();
};

struct DiscourseCategory
{
	rapidjson::Document* json_file = nullptr;
	std::string request_url = "";

	int category_id = -1;

	std::vector<DiscourseTopic*> topics = std::vector<DiscourseTopic*>();
};

struct DDLDownloadRetryInfo
{
	std::string local_path = "";
	std::string url = "";
	int retries = 0;
};

/**
* Namespace containing functions for downloading data from a Discourse forum, and building a
* HTML archive website intended for public viewing.
*/
namespace DDL::Discourse
{
	namespace Downloader
	{
		bool LoadResumeFile();
		void SaveResumeFile();
		DDLResumeInfo* GetLastResumeInfo();
		DDLResumeInfo* GetCurrentResumeInfo();
		DDLResult DownloadTopics(DiscourseCategory* category, std::map<int, std::string>* topic_url_list);
		void DownloadCategories();
	}

	/**
	* Performs all tasks for downloading web content from the configured Discourse forum.
	* 
	* @todo explain how this works in depth
	*/
	void DownloadWebContent();

	/**
	* Builds the HTML archive website from all downloaded data obtained from the
	* configured Discourse forum.
	*
	* @todo explain how this works in depth
	*/
	void BuildArchiveWebsite();
}