#include "switches.h"

#include "components/utils/string/string.h"
#include "components/diagnostics/logger/logger.h"

std::vector<BlamCommandSwitch> active_switches = std::vector<BlamCommandSwitch>();

void DDL::Settings::Switches::ParseSwitches(int args_count, char* args[])
{
	bool found_switch = false;

	bool quoted_switch_value = false;

	BlamCommandSwitch new_switch = BlamCommandSwitch();

	// Skip first item in array, as that's just the name of the executable
	for (int i = 1; i < args_count; i++)
	{
		std::string arg = std::string(args[i]);

		if (DDL::Utils::String::StartsWith(arg, "-", true))
		{
			if (found_switch)
			{
				active_switches.push_back(new_switch);
			}

			found_switch = true;
			new_switch.name = arg.substr(1, arg.length() - 1);

			if (i == args_count - 1)
			{
				active_switches.push_back(new_switch);
			}
		}
		else
		{
			if (found_switch)
			{
				if (arg.starts_with("\""))
				{
					quoted_switch_value = true;
				}

				new_switch.value += arg;

				if (arg.ends_with("\""))
				{
					quoted_switch_value = false;
					new_switch.value = new_switch.value.substr(1, new_switch.value.length() - 2);
				}
			}
			else
			{
				DDL::Logger::LogEvent("unexpected item found in command line switches: '" + arg + "'", DDLLogLevel::Warning);
			}
		}
	}

	DDL::Logger::LogEvent("interpreted " + std::to_string(active_switches.size()) + " command line switches");
}

std::vector<BlamCommandSwitch> DDL::Settings::Switches::GetActiveSwitches()
{
	return active_switches;
}

bool DDL::Settings::Switches::IsSwitchPresent(std::string switch_name)
{
	std::string switch_to_check = DDL::Utils::String::ToLower(switch_name);

	for (int i = 0; i < active_switches.size(); i++)
	{
		std::string current_switch = DDL::Utils::String::ToLower(active_switches.at(i).name);

		if (current_switch == switch_to_check)
		{
			return true;
		}
	}

	return false;
}

std::vector<std::string> DDL::Settings::Switches::GetSwitchArguments(std::string switch_name)
{
	std::string switch_to_check = DDL::Utils::String::ToLower(switch_name);

	for (int i = 0; i < active_switches.size(); i++)
	{
		std::string current_switch = DDL::Utils::String::ToLower(active_switches.at(i).name);

		if (current_switch == switch_to_check)
		{
			std::vector<std::string> options = DDL::Utils::String::Split(active_switches.at(i).value, ",");

			return options;
		}
	}

	// Return empty list if we can't find a matching switch
	std::vector<std::string> empty_list;
	return empty_list;
}

std::string DDL::Settings::Switches::GetSwitchValue(std::string switch_name)
{
	std::string switch_to_check = DDL::Utils::String::ToLower(switch_name);

	for (int i = 0; i < active_switches.size(); i++)
	{
		std::string current_switch = DDL::Utils::String::ToLower(active_switches.at(i).name);

		if (current_switch == switch_to_check)
		{
			return active_switches.at(i).value;
		}
	}

	return "";
}