#include "logger.h"

#include <iomanip>
#include <chrono>
#include <sstream>

#include "components/utils/string/string.h"
#include "components/utils/datetime/datetime.h"

std::string DDL::Logger::Utils::GetCurrentTimestamp()
{
	return "[" + DDL::Utils::DateTime::GetDateTime("%m.%d.%y %H:%M:%S") + "]";
}

std::string DDL::Logger::Utils::GenerateLogLevelPrefix(DDLLogLevel log_level)
{
	std::string format = "[%s]";

	if (log_level == DDLLogLevel::Info)
	{
		format = DDL::Utils::String::Replace(format, "%s", "INFO");
	}
	else if (log_level == DDLLogLevel::Warning)
	{
		format = DDL::Utils::String::Replace(format, "%s", "WARN");
	}
	else if (log_level == DDLLogLevel::Error)
	{
		format = DDL::Utils::String::Replace(format, "%s", "ERROR");
	}
	else
	{
		format = DDL::Utils::String::Replace(format, "%s", "INFO");
	}

	return format;
}