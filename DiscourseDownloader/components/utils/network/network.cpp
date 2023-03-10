#include "network.h"

#include <sstream>

#include "components/3rdparty/curlpp/cURLpp.hpp"
#include "components/3rdparty/curlpp/Easy.hpp"
#include "components/3rdparty/curlpp/Options.hpp"
#include "components/3rdparty/curlpp/Infos.hpp"

#include "components/settings/settings.h"
#include "components/utils/string/string.h"
#include "components/diagnostics/logger/logger.h"
#include "main.h"

std::string DDL::Utils::Network::PerformHTTPRequestWithRetries(std::string url, int* http_code)
{
	int max_retries = 60;
	{
		WebsiteConfig* config = DDL::Settings::GetSiteConfig();

		if (config)
		{
			max_retries = config->max_http_retries;
		}
	}

	std::string response = DDL::Utils::Network::PerformHTTPRequest(url, http_code);

	// If we get an invalid response, retry as many times as specified in config or until a successful response
	{
		int retries = 0;

		while (*http_code != 200)
		{
			// warn about retrying
			response = DDL::Utils::Network::PerformHTTPRequest(url, http_code);
			retries++;

			if (retries >= max_retries && max_retries != -1)
			{
				// retry limit reached, abort
				break;
			}
		}
	}

	return response;
}

std::string DDL::Utils::Network::PerformHTTPRequest(std::string url, int* http_code)
{
	return DDL::Utils::Network::PerformHTTPRequest(url, 0, 1, http_code);
}

std::string DDL::Utils::Network::PerformHTTPRequest(std::string url, float backoff_factor, int max_retries, int* http_code)
{
	std::string response = "";
	std::stringstream response_stream = std::stringstream();

	curlpp::Easy request = curlpp::Easy();
	{
		request.setOpt<cURLpp::Options::WriteStream>(&response_stream);
		request.setOpt<curlpp::options::Url>(url);
		request.setOpt<curlpp::options::UserAgent>("DiscourseDL v" + std::string(DISCOURSEDL_VERSION));
		request.setOpt<curlpp::options::SslVerifyPeer>(false);
		request.setOpt<curlpp::options::FollowLocation>(true);
	}

	float next_attempt_delay = 0.0f;
	bool use_backoff = false;

	if (backoff_factor > 0 && max_retries > 0)
	{
		use_backoff = true;
	}

	for (int retry_count = 0; retry_count < max_retries; retry_count++)
	{
		try
		{
			request.perform();

			int code = curlpp::Infos::ResponseCode::get(request);

			//std::stringstream response_stream = std::stringstream();
			//response_stream << request;

			response = response_stream.str();

			if (http_code)
			{
				*http_code = code;
			}

			break;
		}
		catch (std::exception ex)
		{
			std::cout << ex.what() << std::endl;

			if (use_backoff)
			{
				next_attempt_delay = backoff_factor * (2 * retry_count);

				if (next_attempt_delay > BACKOFF_FACTOR_MAX)
				{
					next_attempt_delay = BACKOFF_FACTOR_MAX;
				}

				if (retry_count == (max_retries - 1))
				{
					DDL::Logger::LogEvent("Maximum retries reached. No further retries will be performed.", DDLLogLevel::Error);
				}
				else
				{
					std::string message = "Backoff factor enabled, will retry in {0}s (retry {1}/{2}.";
					{
						message = DDL::Utils::String::Replace(message, "{0}", std::to_string(next_attempt_delay));
						message = DDL::Utils::String::Replace(message, "{1}", std::to_string(retry_count + 1));
						message = DDL::Utils::String::Replace(message, "{2}", std::to_string(max_retries));
					}

					DDL::Logger::LogEvent(message, DDLLogLevel::Warning);
				}
			}
			else
			{
				break;
			}
		}
	}

	return response;
}