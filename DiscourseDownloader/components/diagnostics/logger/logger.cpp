#include "logger.h"

#include <iostream>

DDLLogFile* active_log = nullptr;

void DDL::Logger::StartLogger()
{
	active_log = new DDLLogFile("discoursedl.log");
}

void DDL::Logger::ShutdownLogger()
{
	if (active_log)
	{
		delete active_log;
	}
}

void DDL::Logger::LogEvent(std::string message)
{
	DDL::Logger::LogEvent(message, DDLLogLevel::Info);
}

void DDL::Logger::LogEvent(std::string message, DDLLogLevel log_level)
{
	DDLLogMessage log_message =
	{
		DDL::Logger::Utils::GetCurrentTimestamp(),
		message,
		log_level
	};

	if (!active_log)
	{
		DDL::Logger::StartLogger();
	}

	active_log->AppendMessageToFile(log_message);

	std::string formatted_log_message = log_message.GenerateLogLine();

	if (log_level == DDLLogLevel::Info)
	{
		DDL::Logger::PrintMessageToStdout(formatted_log_message, TerminalColor::Reset);
	}
	else if (log_level == DDLLogLevel::Warning)
	{
		DDL::Logger::PrintMessageToStdout(formatted_log_message, TerminalColor::Yellow);
	}
	else if (log_level == DDLLogLevel::Error)
	{
		DDL::Logger::PrintMessageToStdout(formatted_log_message, TerminalColor::BrightRed);
	}
	else
	{
		DDL::Logger::PrintMessageToStdout(formatted_log_message, TerminalColor::Reset);
	}
}