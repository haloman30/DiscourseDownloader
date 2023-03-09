#pragma once

#include <map>
#include <string>
#include <Windows.h>

#include "components/diagnostics/errors/errors.h"
#include "components/settings/config/BlamColor.h"

#define CONFIG_DUMMY_SECTION_NAME "#___default"

/**
* Enumerator defining all possible configuration setting types.
*/
enum class BlamConfigurationSettingType
{
	String,  //!< Indicates the setting stores a string.
	Boolean, //!< Indicates the setting stores a bool.
	Float,   //!< Indicates the setting stores a float.
	Int,     //!< Indicates the setting stores an integer.
	Color,   //!< Indicates the setting stores a color.
	Comment, //!< Used to indicate a comment within the file.
	Invalid  //!< Indicates the setting type was invalid.
};

/**
* Class representing a configuration setting.
*/
class BlamConfigurationSetting
{
private:
	std::string filename = "";    //!< The name of the configuration file this setting belongs to.
	std::string comment_delimeter = ""; //!< The comment delimeter used when parsing the line.
	int line_number = 0;                //!< The line number that this setting appeared on within the file.

	std::string value = "";       //!< The string value of the setting, or comment text if the setting is storing a comment.
	bool value_bool = false;      //!< The boolean value of the setting.
	float value_float = 0.0f;     //!< The float value of the setting.
	int value_int = 0;            //!< The int value of the setting.
	BlamColor value_color = BlamColor(255, 255, 255); //!< The color value of the setting.

	bool valid = false; //!< Whether or not the setting was able to be interpreted.
public:
	std::string id = "unspecified"; //!< The name of the setting.
	BlamConfigurationSettingType type = BlamConfigurationSettingType::Invalid; //!< The type of setting that's being stored.

	/**
	* Prepares a new configuration setting.
	* 
	* This method will create new setting data, as well as convert the setting value to the appropriate type.
	* 
	* @param line - The line in the file to interpret as a configuration setting.
	* @param _line_number - The line number that this setting appeared on within the file.
	* @param _comment_delimeter - The prefix to use when checking if the line is a comment.
	* @param _filename - The name of the configuration file this setting belongs to.
	*/
	BlamConfigurationSetting(std::string line, int _line_number, std::string _comment_delimeter, std::string _filename);

	/**
	* Parses the raw setting value depending on the setting type.
	*/
	void ParseValue();

	/**
	* Whether or not the provided file line was able to be interpreted as a valid setting.
	* 
	* @returns `true` if the setting is valid, otherwise returns `false`.
	*/
	bool IsValid();

	/**
	* Retrieves the setting value as a string.
	* 
	* @returns Pointer to the setting value as a string.
	*/
	std::string* AsString();

	/**
	* Retrieves the setting value as a bool.
	*
	* @returns Pointer to the setting value as a bool. If the value could not be converted to a bool, then
	* `nullptr` will be returned instead.
	*/
	bool* AsBool();

	/**
	* Retrieves the setting value as a float.
	*
	* @returns Pointer to the setting value as a float. If the value could not be converted to a float, then
	* `nullptr` will be returned instead.
	*/
	float* AsFloat();

	/**
	* Retrieves the setting value as an int.
	*
	* @returns Pointer to the setting value as an int. If the value could not be converted to an int, then
	* `nullptr` will be returned instead.
	*/
	int* AsInt();

	/**
	* Retrieves the setting value as a color.
	*
	* @returns Pointer to the setting value as a color. If the value could not be converted to a color, then
	* `nullptr` will be returned instead.
	*/
	BlamColor* AsColor();

	/**
	* Converts the raw setting value to a string, with no placeholders expanded.
	* 
	* @returns The configuration setting as a string.
	*/
	std::string ConvertRawValueToString();

	/**
	* Updates the configuration setting with a new value.
	*/
	void UpdateValue(std::string new_value);

	/**
	* Retrieves the raw value of the setting, with no placeholders expanded.
	*/
	std::string* GetRawValue();

	/**
	* Creates a new copy of this setting. Used when creating a new setting from a base default setting.
	* 
	* @returns A pointer to the new copy of the setting.
	*/
	BlamConfigurationSetting* Copy();

	/**
	* Generates a string containing the setting as a config line, ready to be written to a config file.
	* 
	* @returns The configuration setting as a config line string.
	*/
	std::string CreateConfigFileLine();
};

/**
* Structure representing a configuration section.
* 
* Unlike the old configuration file setup, configuration files are broken up into sections, similar
* to an INI file. These can be used to group similar settings 
*/
class BlamConfigurationSection
{
private:
	std::string filename; //!< The filename that this configuration setting belongs to.

public:
	std::map<std::string, BlamConfigurationSetting*> settings;         //!< The list of configuration settings within this section.
	std::map<std::string, BlamConfigurationSetting*> default_settings; //!< The list of default settings within this section.
	std::string name; //!< The name of this configuration section.

	/**
	* Prepares a new configuration section.
	* 
	* @param _name - The name of the configuration section.
	* @param _filename - The filename that this section belongs to.
	*/
	BlamConfigurationSection(std::string _name, std::string _filename);

	/**
	* Destructor. Deletes any and all settings stored within the section.
	*/
	~BlamConfigurationSection();

	/**
	* Adds a new configuration setting to the section.
	* 
	* @param new_setting - Pointer to the new setting to add to this section.
	*/
	void AddNewSetting(BlamConfigurationSetting* new_setting);

	/**
	* Adds a new default configuration setting to the section.
	*
	* @param new_setting - Pointer to the new default setting to add to this section.
	*/
	void AddNewDefaultSetting(BlamConfigurationSetting* new_setting);

	/**
	* Checks whether or not a given configuration setting exists in this section.
	* 
	* @param option - The setting to look for.
	* 
	* @returns `true` if the option exists, otherwise returns `false`.
	*/
	bool HasOption(std::string option);

	/**
	* Retrieves a configuration setting as a string.
	* 
	* @param option - The option to retrieve.
	* 
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but is not listed as a
	*     string, then will also return `nullptr`.
	*/
	std::string* GetString(std::string option);

	/**
	* Retrieves a configuration setting as a bool.
	*
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but cannot be
	*     interpreted as a bool, then will also return `nullptr`.
	*/
	bool* GetBool(std::string option);

	/**
	* Retrieves a configuration setting as a float.
	*
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but cannot be
	*     interpreted as a float, then will also return `nullptr`.
	*/
	float* GetFloat(std::string option);

	/**
	* Retrieves a configuration setting as an int.
	*
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but cannot be
	*     interpreted as an int, then will also return `nullptr`.
	*/
	int* GetInt(std::string option);

	/**
	* Retrieves a configuration setting as a color.
	*
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but cannot be
	*     interpreted as a color, then will also return `nullptr`.
	*/
	BlamColor* GetColor(std::string option);

	/**
	* Restores all settings to their default values.
	*/
	void RestoreDefaultSettings();
};

class BlamConfigurationFile
{
private:
	bool loaded = false; //!< Whether or not the file has been fully loaded.

	/**
	* Releases any data used by configuration sections/settings.
	*/
	void ClearSections();

public:
	std::map<std::string, BlamConfigurationSection*> sections; //!< The list of configuration sections contained within the file.

	std::string filename = "";          //!< The file path of the configuration file.
	std::string defaults_filename = ""; //!< The file path of the configuration file used to store default settings.
	std::string comment_delimeter = ""; //!< The character or string used to indicate a comment within the file.

	/**
	* Prepares a new configuration file to be loaded.
	* 
	* @param _filename - The path to the configuration file.
	* @param _comment_delimeter - The prefix to use when checking if a line is a comment.
	* 
	* @deprecated Use the new constructor.
	*/
	BlamConfigurationFile(std::string _filename, std::string _comment_delimeter);

	/**
	* Destructor. Deletes any configuration sections within the file.
	*/
	~BlamConfigurationFile();

	/**
	* Load the list of default sections.
	*
	* @param _filename - The path to the configuration file containing default settings.
	*/
	DDLResult LoadDefaults(std::string _filename);

	/**
	* Loads the configuration file from disk.
	* 
	* @returns `#BlamResult::Success_OK` if the file was loaded successfully, otherwise returns an error code.
	*/
	DDLResult Load();

	/**
	* Loads settings from a secondary file into this file. Used for merging multiple configuration files together at runtime.
	*
	* @param _filename - The file to load additional settings from.
	* 
	* @returns `#BlamResult::Success_OK` if the file was loaded successfully, otherwise returns an error code.
	*/
	DDLResult Load(std::string _filename);

	/**
	* Cleans up old data and reloads the configuration file from disk.
	*
	* @returns `#BlamResult::Success_OK` if the file was loaded successfully, otherwise returns an error code.
	*/
	DDLResult Reload();

	/**
	* Saves the configuration file back to disk.
	*
	* @returns `#BlamResult::Success_OK` if the file was saved successfully, otherwise returns an error code.
	*/
	void Save();

	/**
	* Whether or not this configuration file has been loaded.
	* 
	* @returns Whether or not this configuration file has been loaded.
	*/
	bool IsLoaded();

	/**
	* Adds a new configuration section to the file.
	*/
	void AddNewSection(BlamConfigurationSection* section);

	/**
	* Checks whether or not a given configuration section exists in the file.
	* 
	* @param section_name - The name of the section to look for.
	* 
	* @returns `true` if the section exists, otherwise returns `false`.
	*/
	bool HasConfigurationSection(std::string section_name);

	/**
	* Retrieves a configuration section from the configuration file.
	* 
	* @returns The specified configuration section if it exists, otherwise returns `NULL`.
	*/
	BlamConfigurationSection* GetConfigurationSection(std::string section_name);

	/**
	* Resets all configuration values to default.
	*/
	void RestoreDefaultSettings();

	/**
	* Retrieves a configuration setting as a string.
	*
	* @param section_name - The section to retrieve the setting from.
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but is not listed as a
	*     string, then will also return `nullptr`.
	*/
	std::string* GetString(std::string section_name, std::string option);

	/**
	* Retrieves a configuration setting as a bool.
	*
	* @param section_name - The section to retrieve the setting from.
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but cannot be
	*     interpreted as a bool, then will also return `nullptr`.
	*/
	bool* GetBool(std::string section_name, std::string option);

	/**
	* Retrieves a configuration setting as a float.
	*
	* @param section_name - The section to retrieve the setting from.
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but cannot be
	*     interpreted as a float, then will also return `nullptr`.
	*/
	float* GetFloat(std::string section_name, std::string option);

	/**
	* Retrieves a configuration setting as an int.
	*
	* @param section_name - The section to retrieve the setting from.
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but cannot be
	*     interpreted as an int, then will also return `nullptr`.
	*/
	int* GetInt(std::string section_name, std::string option);

	/**
	* Retrieves a configuration setting as a color.
	*
	* @param section_name - The section to retrieve the setting from.
	* @param option - The option to retrieve.
	*
	* @returns Pointer to the setting value if it exists, otherwise returns `nullptr`. If the setting exists but cannot be
	*     interpreted as a color, then will also return `nullptr`.
	*/
	BlamColor* GetColor(std::string section_name, std::string option);
};

/**
* Namespace for anything related to configuration files.
*/
namespace DDL::Settings::Config
{
	/**
	* Loads a new configuration file from disk.
	*
	* @param filename - The name/path to the configuration file to load.
	* @param comment_delimeter - The string or character used to indicate a comment within the file.
	* @param file - Pointer to the pointer to set to the loaded configuration data. Will only be set if the file is loaded properly.
	*
	* @returns #BlamResult::Success_OK if the file was loaded successfully, otherwise returns an error code.
	*/
	DDLResult LoadConfiguration(std::string filename, std::string comment_delimeter, OUT BlamConfigurationFile** file);

	/**
	* Loads a new configuration file from disk.
	*
	* @param filename - The name/path to the configuration file to load.
	* @param comment_delimeter - The string or character used to indicate a comment within the file.
	* @param file - Pointer to the pointer to set to the loaded configuration data. Will only be set if the file is loaded properly.
	*
	* @returns #BlamResult::Success_OK if the file was loaded successfully, otherwise returns an error code.
	*/
	DDLResult LoadConfiguration(std::string filename, std::string defaults_filename, std::string comment_delimeter, OUT BlamConfigurationFile** file);
}