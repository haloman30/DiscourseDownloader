#include "logger.h"

#include <fstream>

#include "components/utils/io/io.h"

DDLLogFile::DDLLogFile(std::string _filename)
{
	filename = _filename;

	if (!DDL::Utils::IO::FileExists(filename))
	{
		if (!DDL::Utils::IO::CreateNewFile(filename, ""))
		{
			DDL::Logger::LogEvent("Could not create new log file!", DDLLogLevel::Error);
		}
	}
}

DDLResult DDLLogFile::AppendMessageToFile(DDLLogMessage message)
{
	history.push_back(message);

	std::ofstream file = std::ofstream(filename, std::ios::out | std::ios::app);

	if (file.fail())
	{
		return DDLResult::Error_Generic;
	}

	file << message.GenerateLogLine() << std::endl;
	file.close();

	return DDLResult::Success_OK;
}