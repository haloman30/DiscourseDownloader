#pragma once

#include <vector>
#include <string>

#include "components/3rdparty/rapidjson/document.h"
#include "components/diagnostics/errors/errors.h"

struct DiscourseTopic
{
	rapidjson::Document* json_file = nullptr;
	std::string request_url = "";

	int topic_id = -1;
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