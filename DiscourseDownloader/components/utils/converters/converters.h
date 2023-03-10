#pragma once

#include <string>

/**
* Namespace containing functions to convert between various types of variables.
*/
namespace DDL::Converters
{
	/**
	* Converts a string to a boolean.
	*
	* The following will be interpreted as `true`:
	* > true, t, 1, on, yes, y
	*
	* The following will be interpreted as `false`:
	* > false, f, 0, off, no, n
	*
	* Any other value will return `false` and add a message to console.
	*
	* @param string - The string to convert.
	*
	* @returns The string evaluated to a `bool`.
	*/
	bool StringToBool(std::string string);

	/**
	* Converts a hexadecimal code to its respective character.
	*
	* This is used to evaluate a hex code representing a byte to its equivalent `char`.
	*
	* @param hex - The string containing the hexadecimal representation of a character.
	* @param character - Pointer to the `char` to set to the evaluated character.
	*
	* @returns `true` if the character was converted successfully, `false` if an error occurred.
	*/
	bool HexStringToChar(std::string hex, char* character);

	/**
	* Converts a string representation of a float to a `float`.
	*
	* @param string - The string containing the float value representation.
	*
	* @returns The evaluated float value, or `0.0f` if the value failed to convert.
	*/
	float StringToFloat(std::string string);

	/**
	* Converts a string representation of an integer to an `int`.
	*
	* @param string - The string containing the integer value representation.
	*
	* @returns The evaluated integer value, or `0` if the value failed to convert.
	*/
	int StringToInt(std::string string);

	/**
	* Converts a string representation of an integer to an `int64_t`.
	*
	* @param string - The string containing the integer value representation.
	*
	* @returns The evaluated integer value, or `0` if the value failed to convert.
	*/
	int64_t StringToInt64(std::string string);

	/**
	* Converts a string representation of a double to a `double`.
	*
	* @param string - The string containing the double value representation.
	*
	* @returns The evaluated double value, or `0` if the value failed to convert.
	*/
	double StringToDouble(std::string string);

	/**
	* Checks whether or not a string is a valid representation of an integer.
	*
	* @param string - The string to check.
	*
	* @returns `true` if the string represents a valid integer, otherwise returns `false.
	*/
	bool IsStringInt(std::string string);

	/**
	* Checks whether or not a string is a valid representation of a float.
	*
	* @param string - The string to check.
	*
	* @returns `true` if the string represents a valid float, otherwise returns `false.
	*/
	bool IsStringFloat(std::string string);

	/**
	* Converts a boolean value to a string.
	*
	* @param value - The boolean value to convert.
	*
	* @returns The boolean value represented as a string. Will always be lowercase `true` or `false`.
	*/
	std::string BoolToString(bool value);

	/**
	* Converts a floating-point number to a string.
	*
	* @param value - The value to convert.
	* @param max_decimals - The amount of decimal places to display. Anything below 0 will show all decimal places,
	*	and if the amount is larger than the actual decimal value, then zeroes will be added to the end of the string
	*	to match the provided value.
	*
	* @returns The floating-point number converted to a string.
	*/
	std::string FloatToString(float value, int max_decimals = -1);
}