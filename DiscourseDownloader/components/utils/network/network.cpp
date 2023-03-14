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
	bool retry_backoff = true;
	int backoff_increment = 5;
	int retry_delay = 1;
	int max_retries = 60;
	bool fail_on_403 = true;
	bool fail_on_404 = false;
	int max_404s = 5;
	{
		WebsiteConfig* config = DDL::Settings::GetSiteConfig();

		if (config)
		{
			max_retries = config->max_http_retries;
			retry_delay = config->request_retry_delay;
			retry_backoff = config->http_retry_use_backoff;
			backoff_increment = config->http_backoff_increment;
			fail_on_403 = config->fail_on_403;
			fail_on_404 = config->fail_on_404;
			max_404s = config->max_404s;
		}
	}

	std::string response = DDL::Utils::Network::PerformHTTPRequest(url, http_code);

	// If we get an invalid response, retry as many times as specified in config or until a successful response
	{
		int retries = 0;
		int next_retry_backoff_delay = retry_delay;
		int remaining_404s = max_404s;

		while (*http_code != 200)
		{
			if (fail_on_403 && *http_code == 403)
			{
				DDL::Logger::LogEvent("stopping further network connection attempts due to http 403 - if you want, you "
					"can disable this in website.cfg but this could result in a permanent soft-lock of the application", DDLLogLevel::Error);
				break;
			}

			if (*http_code == 404)
			{
				if (fail_on_404)
				{
					DDL::Logger::LogEvent("stopping further network connection attempts due to http 404 - if you want, you "
						"can disable this in website.cfg but this could result in a permanent soft-lock of the application", DDLLogLevel::Error);
					break;
				}

				if (remaining_404s <= 0 && max_404s != -1)
				{
					DDL::Logger::LogEvent("stopping further retries due to reaching maximum 404 count () - if you want, you"
						"can increase this amount or set it to -1 to disable this limit in website.cfg", DDLLogLevel::Error);
					break;
				}

				remaining_404s--;
			}

			

			if (retry_backoff)
			{
				next_retry_backoff_delay = backoff_increment * (retries + 1);
			}

			DDL::Logger::LogEvent("network request returned http " + std::to_string(*http_code)
				+ ", retrying in " + std::to_string(next_retry_backoff_delay) + "s (" + std::to_string(retries) + "/" + std::to_string(max_retries) + ")", DDLLogLevel::Warning);

			Sleep(1000 * next_retry_backoff_delay);

			// warn about retrying
			response = DDL::Utils::Network::PerformHTTPRequest(url, http_code);
			retries++;

			if (retries >= max_retries && max_retries != -1)
			{
				DDL::Logger::LogEvent("retry limit reached - returning failed response", DDLLogLevel::Warning);
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

		WebsiteConfig* config = DDL::Settings::GetSiteConfig();

		if (!config)
		{
			request.setOpt<curlpp::options::UserAgent>("DiscourseDL v" + std::string(DISCOURSEDL_VERSION));
		}
		else
		{
			if (config->override_user_agent)
			{
				request.setOpt<curlpp::options::UserAgent>(config->user_agent);
			}
			else
			{
				request.setOpt<curlpp::options::UserAgent>("DiscourseDL v" + std::string(DISCOURSEDL_VERSION));
			}
		}
		
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