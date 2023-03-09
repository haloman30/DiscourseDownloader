#pragma once

#include <string>

/**
* Utilities relating to date and time.
*/
namespace DDL::Utils::DateTime
{
	/**
	* Retrieves the current date/time in the specified format.
	*
	* @param format - The desired format for the date and time.
	*
	* @returns String containing the date and time in the desired format.
	*/
	std::string GetDateTime(const char* format);

	/**
	* Retrieves the current epoch time, in seconds.
	* 
	* @returns The current epoch time, in seconds.
	*/
	uint64_t GetCurrentEpoch();
}