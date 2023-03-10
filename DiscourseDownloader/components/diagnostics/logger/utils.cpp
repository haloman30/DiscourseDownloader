#include "logger.h"

#include <iomanip>
#include <chrono>
#include <sstream>

#include "components/3rdparty/termcolor/termcolor.hpp"
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

TerminalColor DDL::Logger::TranslateLogLevelAsColor(DDLLogLevel log_level)
{
	if (log_level == DDLLogLevel::Warning)
	{
		return TerminalColor::Yellow;
	}
	else if (log_level == DDLLogLevel::Error)
	{
		return TerminalColor::BrightRed;
	}
	else
	{
		return TerminalColor::Reset;
	}
}

void DDL::Logger::PrintMessageToStdout(std::string message, TerminalColor color)
{
	switch (color)
	{
	case TerminalColor::Black:
	{
		std::cout << termcolor::grey;
		break;
	}
	case TerminalColor::Blue:
	{
		std::cout << termcolor::blue;
		break;
	}
	case TerminalColor::Green:
	{
		std::cout << termcolor::green;
		break;
	}
	case TerminalColor::Cyan:
	{
		std::cout << termcolor::cyan;
		break;
	}
	case TerminalColor::Red:
	{
		std::cout << termcolor::red;
		break;
	}
	case TerminalColor::Magenta:
	{
		std::cout << termcolor::magenta;
		break;
	}
	case TerminalColor::Gold:
	{
		std::cout << termcolor::yellow;
		break;
	}
	case TerminalColor::Gray:
	{
		std::cout << termcolor::white;
		break;
	}
	case TerminalColor::DarkGray:
	{
		std::cout << termcolor::bright_grey;
		break;
	}
	case TerminalColor::BrightBlue:
	{
		std::cout << termcolor::bright_blue;
		break;
	}
	case TerminalColor::BrightGreen:
	{
		std::cout << termcolor::bright_green;
		break;
	}
	case TerminalColor::BrightCyan:
	{
		std::cout << termcolor::bright_cyan;
		break;
	}
	case TerminalColor::BrightRed:
	{
		std::cout << termcolor::bright_red;
		break;
	}
	case TerminalColor::BrightMagenta:
	{
		std::cout << termcolor::bright_magenta;
		break;
	}
	case TerminalColor::Yellow:
	{
		std::cout << termcolor::bright_yellow;
		break;
	}
	case TerminalColor::White:
	{
		std::cout << termcolor::bright_white;
		break;
	}
	case TerminalColor::Reset:
	{
		std::cout << termcolor::reset;
		break;
	}
	default:
	{
		std::cout << termcolor::reset;
		break;
	}
	}

	std::cout << message << termcolor::reset << std::endl;
}