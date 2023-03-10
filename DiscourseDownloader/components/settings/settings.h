#pragma once

#include <string>
#include <stdint.h>

#include "components/diagnostics/errors/errors.h"
#include "components/settings/config/config.h"

struct WebsiteConfig
{
    std::string website_url = "https://my-discourse";
    int max_get_more_topics = -1;
    bool download_users = true;
    bool perform_html_build = false;
    bool skip_download = false;
    int max_http_retries = 60;
    int max_posts_per_request = 20;
    int topic_url_collection_notify_interval = 15;

    std::string html_path = "export/";
    std::string json_path = "json/";
    std::string site_directory_root = "./my-discourse/";

    std::string cookie_name = "_t";
    std::string cookie = "";
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