#pragma once

#include <vector>
#include <string>
#include <map>

#include "components/3rdparty/rapidjson/document.h"
#include "components/diagnostics/errors/errors.h"

#define JSON_CATEGORY_ROOT_FORMAT std::string("<JSON_ROOT>/c/<CAT_ID>/")
#define DATA_CACHE_ENTRY_FORMAT std::string("<URL>|<TOPIC_ID>|<POST_COUNT>|<POST_IDS>")
#define CATEGORY_LIST_URL_FORMAT std::string("<BASE_URL>/categories.json?include_subcategories=true")
#define CATEGORY_INFO_URL_FORMAT std::string("<BASE_URL>/c/<CAT_ID>/show.json")
#define TOPIC_LIST_URL_FORMAT std::string("<BASE_URL>/c/<CAT_SLUG>/<CAT_ID>.json?page=")
#define TOPIC_INFO_URL_FORMAT std::string("<BASE_URL>/t/<TOPIC_ID>.json")
#define TOPIC_POSTS_URL_FORMAT std::string("<BASE_URL>/t/<TOPIC_ID>/posts.json?")

#define JSON_DIRECTORY_ROOT_FORMAT std::string("<JSON_ROOT>/directory/")
#define DIRECTORY_LIST_URL_FORMAT std::string("<BASE_URL>/directory_items.json?period=all&page=")

#define JSON_USER_ROOT_FORMAT std::string("<JSON_ROOT>/u/<USER_ID>/")
#define USER_INFO_URL_FORMAT std::string("<BASE_URL>/u/<USERNAME>.json")
#define USER_BADGES_INFO_URL_FORMAT std::string("<BASE_URL>/user-badges/<USERNAME>.json")
#define USER_ACTIONS_INFO_URL_FORMAT std::string("<BASE_URL>/user_actions.json?username=<USERNAME>&offset=")
#define USER_PMS_URL_FORMAT std::string("<BASE_URL>/topics/private-messages/<USERNAME>.json")
#define USER_PMS_SENT_URL_FORMAT std::string("<BASE_URL>/topics/private-messages-sent/<USERNAME>.json")

#define JSON_GROUPS_ROOT_FORMAT std::string("<JSON_ROOT>/groups/")
#define GROUPS_LIST_URL_FORMAT std::string("<BASE_URL>/groups.json?page=")
#define GROUPS_INFO_URL_FORMAT std::string("<BASE_URL>/groups/<NAME>.json")

#define JSON_TAGS_ROOT_FORMAT std::string("<JSON_ROOT>/tags/")
#define TAG_LIST_URL_FORMAT std::string("<BASE_URL>/tags.json")
#define TAG_INFO_URL_FORMAT std::string("<BASE_URL>/tag/<NAME>.json?page=")
#define TAG_GROUP_LIST_URL_FORMAT std::string("<BASE_URL>/tag_groups.json")
#define TAG_GROUP_INFO_URL_FORMAT std::string("<BASE_URL>/tag_groups/<GROUP_ID>.json")

#define JSON_SITE_ROOT_FORMAT std::string("<JSON_ROOT>/")
#define SITE_INFO_URL_FORMAT std::string("<BASE_URL>/site.json")

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
		void DownloadUsers();
		void DownloadSiteInfo();
		void DownloadTags();
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