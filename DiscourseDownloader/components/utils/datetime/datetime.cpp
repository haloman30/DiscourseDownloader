#include "datetime.h"

#include <chrono>
#include <sstream>
#include <iomanip>

std::string DDL::Utils::DateTime::GetDateTime(const char* format)
{
	time_t current_time = std::time(nullptr);
	tm local_time;

	int result = localtime_s(&local_time, &current_time);

	if (result != 0)
	{

	}

	std::ostringstream time_ss = std::ostringstream();

	time_ss << std::put_time(&local_time, format);

	return time_ss.str();
}

uint64_t DDL::Utils::DateTime::GetCurrentEpoch()
{
	std::chrono::seconds ms = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}