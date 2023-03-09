#include "logger.h"

std::string DDLLogMessage::GenerateLogLine()
{
	std::string line = timestamp + " " + DDL::Logger::Utils::GenerateLogLevelPrefix(log_level) + " " + message;
	return line;
}