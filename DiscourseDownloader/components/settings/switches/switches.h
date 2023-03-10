#pragma once

#include <string>
#include <vector>

/**
* Structure representing a command-line switch.
*
* These are switches/flags passed to the application via the command line. A switch can either
* be a switch on its own (ie, `-console`) or it can include arguments (ie, `-vidmode w,h,r`).
* Arguments must NOT contain any spaces. For multiple arguments, commas should be used. A
* switch MUST begin with a `-` or else it will be treated as an argument for the previous
* switch, or be ignored if the previous switch already had arguments passed to it.
*
* @todo find a way to add spaces support (maybe through quotes or something idk)
*/
struct BlamCommandSwitch
{
    std::string name = "unknown";  //!< The name of the switch, excluding the `-` prefix.
    std::string value = ""; //!< The value provided to the switch. Will be an empty string if no value was provided.
};

/**
* Namespace containing things relating to command-line switches.
* 
* Command-line switches refer to arguments or values which are passed into the program at startup
* using the command line. For instance, you can start the game engine with the console enabled by
* using `blamite.exe -console`.
*/
namespace DDL::Settings::Switches
{
    /**
    * Parses command-line switches from the initial variables from `main()`.
    *
    * @param args_count - Number of command line arguments provided.
    * @param args - Array containing the command line arguments.
    */
    void ParseSwitches(int args_count, char* args[]);
    
    /**
    * Retrieves the list of loaded command-line switches.
    *
    * @returns A copy of the loaded command-line switches.
    */
    std::vector<BlamCommandSwitch> GetActiveSwitches();
    
    /**
    * Determines if a given switch has been passed to the engine.
    *
    * @param switch_name - The name of the switch, without the `-` prefix.
    *
    * @returns `true` if a switch is loaded, otherwise returns `false`.
    */
    bool IsSwitchPresent(std::string switch_name);
    
    /**
    * Retrieves the list of arguments provided for the specified switch.
    *
    * @param switch_name - The name of the switch, without the `-` prefix.
    *
    * @returns The list of arguments for the switch, or returns an empty list if the
    * specified switch has no arguments or wasn't passed. You probably want to use
    * #Blam::Switches::IsSwitchPresent first.
    */
    std::vector<std::string> GetSwitchArguments(std::string switch_name);

    /**
    * Retrieves the value of a switch.
    * 
    * @param switch_name - The name of the switch, without the `-` prefix.
    * 
    * @returns The value of the switch. If the switch has no value attached to it, then
    * an empty string is returned.
    */
    std::string GetSwitchValue(std::string switch_name);
}