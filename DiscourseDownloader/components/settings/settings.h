#pragma once

#include <string>
#include <stdint.h>
#include <vector>

#include "components/diagnostics/errors/errors.h"
#include "components/settings/config/config.h"

struct WebsiteConfig
{
    // website_config
    std::string website_url = "https://my-discourse";
    std::string site_directory_root = "./my-discourse/";
    bool skip_download = false;
    bool download_users = true;
    bool perform_html_build = false;

    // networking
    int max_http_retries = 60;
    bool http_retry_use_backoff = true;
    int http_backoff_increment = 5;
    bool override_user_agent = false;
    std::string user_agent = "";
    int request_retry_delay = 1;

    // download
    bool resume_download = true;
    bool enable_url_caching = true;
    bool enable_data_caching = true;
    bool delete_caches_on_finish = false;
    bool redownload_if_missing_cache = false;
    bool sanity_check_on_finish = true;
    bool thorough_sanity_check = true;
    bool download_skip_existing_categories = false;
    bool download_skip_existing_topics = false;
    bool download_skip_existing_posts = true;

    // forums
    int max_get_more_topics = -1;
    int max_posts_per_request = 20;
    int topic_url_collection_notify_interval = 15;
    bool download_subcategory_topics = false;
    bool use_category_id_filter = false;
    std::string category_id_filter = "";
    std::vector<int> category_id_whitelist = std::vector<int>();
    bool use_filter_as_blacklist = false;
    bool strict_topic_count_checks = false;
    
    // paths
    std::string html_path = "export/";
    std::string json_path = "json/";

    // cookies
    std::string cookie_name = "_t";
    std::string cookie = "";

    // misc
    bool disable_long_finish_message = false;
    bool log_level_debug = false;
};

/**
* Namespace containing functions for working with configuration files.
*/
namespace DDL::Settings
{
    /**
    * Retrieves the loaded website download configuration, loaded from `website.cfg` by default.
    * 
    * @returns The parsed website configuration.
    */
    WebsiteConfig* GetSiteConfig();

    /**
    * Attempts to load all required configuration files.
    * 
    * @returns `true` if all configuration files loaded successfully, otherwise returns `false`. Initialization
    * should likely be aborted if this function fails.
    */
	bool LoadAllConfigurations();

    /**
    * Cleans up all configuration file data.
    */
    void CleanupConfigurations();
}