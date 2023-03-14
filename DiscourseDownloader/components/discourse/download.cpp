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

void DDL::Discourse::DownloadWebContent()
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get website config - web content will NOT be downloaded!", DDLLogLevel::Error);
		return;
	}

	DDL::Discourse::Downloader::DownloadCategories();
	DDL::Discourse::Downloader::DownloadUsers();
	DDL::Discourse::Downloader::DownloadSiteInfo();
	DDL::Discourse::Downloader::DownloadTags();
}