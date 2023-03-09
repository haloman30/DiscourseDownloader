#pragma once

#include <string>
#include <vector>

#include "components/diagnostics/errors/errors.h"

#define CONSOLE_COLOR_RESET std::string("\033[0m")
#define CONSOLE_COLOR_RED std::string("\033[1;31m")
#define CONSOLE_COLOR_YELLOW std::string("\033[1;33m")
#define CONSOLE_COLOR_GREEN std::string("\033[1;32m")

/**
* Enumerator listing possible log levels.
*/
enum class DDLLogLevel
{
	Info,    //!< Indicates an informational message.
	Warning, //!< Indicates a message that the user should be aware of, but may not be critical.
	Error    //!< Indicates an critical error that the user should be aware of immediately.
};

/**
* Class representing a log message.
*/
class DDLLogMessage
{
public:
	std::string timestamp = ""; //!< A string containing the timestamp of this message.
	std::string message = "";   //!< The content of this message.
	DDLLogLevel log_level = DDLLogLevel::Info; //!< The log level of this message.

	/**
	* Generates a string containing a fully formatted message line, ready to be displayed to the user.
	* 
	* @returns A string representation of this log message.
	*/
	std::string GenerateLogLine();
};

/**
* Class representing a log file.
*/
class DDLLogFile
{
public:
	std::vector<DDLLogMessage> history = std::vector<DDLLogMessage>(); //!< The complete log history of this file.
	std::string filename = ""; //!< The filename/path of this log file.

	/**
	* Prepares a new log file.
	* 
	* @param _filename - The filename/path of the new log file.
	*/
	DDLLogFile(std::string _filename);

	/**
	* Adds a new message to this log file.
	* 
	* @param message - The message to append to the file.
	* 
	* @returns `Success_OK` if the message was appended successfully, otherwise returns an error code.
	*/
	DDLResult AppendMessageToFile(DDLLogMessage message);
};

/**
* Namespace containing functions for the logger.
*/
namespace DDL::Logger
{
	/**
	* Namespace containing utility functions used by the logger internally.
	*/
	namespace Utils
	{
		/**
		* Generates a prefix label for a given log level, ie: `[INFO]` or `[WARN]`.
		* 
		* @param log_level - The log level to create a prefix for.
		* 
		* @returns The appropriate log level prefix as a string.
		*/
		std::string GenerateLogLevelPrefix(DDLLogLevel log_level);

		/**
		* Generates a string containing the current timestamp, formatted as follows:
		* 
		* > [MM.DD.YY hh:mm:ss]
		* 
		* @returns A string containing the current log timestamp.
		*/
		std::string GetCurrentTimestamp();
	}

	/**
	* Initializes the logger.
	*/
	void StartLogger();

	/**
	* Shuts down the logger.
	*/
	void ShutdownLogger();

	/**
	* Logs a new message, with the `Info` log level.
	* 
	* @param message - The message to log.
	*/
	void LogEvent(std::string message);

	/**
	* Logs a new message.
	*
	* @param message - The message to log.
	* @param log_level - The log level of the new message.
	*/
	void LogEvent(std::string message, DDLLogLevel log_level);
}