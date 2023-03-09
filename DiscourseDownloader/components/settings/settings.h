#pragma once

#include <string>
#include <stdint.h>

#include "components/diagnostics/errors/errors.h"
#include "components/settings/config/config.h"

struct WebsiteConfig
{
    std::string website_url = "";
    int max_get_more_topics = -1;
    bool download_users = true;

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