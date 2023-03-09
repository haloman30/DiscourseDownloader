#pragma once

#include <string>

#define BACKOFF_FACTOR_MAX 60.0f

/**
* Namespace containing functions for interacting with network-based services.
*/
namespace DDL::Utils::Network
{
	std::string PerformHTTPRequestWithRetries(std::string url, int* http_code = nullptr);

	/**
	* Performs an HTTP request and stores the output.
	* 
	* @param url - The URL to send the request to.
	* @param http_code - An optional pointer to an integer. This will be set to the HTTP response code if specified.
	* 
	* @returns A string containing the response text. If the request failed for any reason, an empty string is returned.
	*/
	std::string PerformHTTPRequest(std::string url, int* http_code = nullptr);

	std::string PerformHTTPRequest(std::string url, float backoff_factor, int max_retries, int* http_code);
}