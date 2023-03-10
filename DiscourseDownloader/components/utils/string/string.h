#pragma once

#include <string>
#include <vector>

/**
* Utilities for working with and modifying strings.
*/
namespace DDL::Utils::String
{
	/**
	* Determines whether or not a string starts with another string, **case-insensetive**.
	*
	* @param string - The original string.
	* @param starts_with - The string to test for.
	* @param case_sensetive - Whether or not the check is case insensetive.
	*
	* @returns Whether or not the string starts with the specified prefix.
	*/
	bool StartsWith(std::string string, std::string starts_with, bool case_insensetive);

	/**
	* Determines whether or not a string ends with another string, **case-sensetive**.
	*
	* @param string - The original string.
	* @param ends_with - The string to test for.
	* @param case_sensetive - Whether or not the check is case insensetive.
	*
	* @returns Whether or not the string ends with the specified suffix.
	*/
	bool EndsWith(std::string string, std::string ends_with, bool case_insensetive);

	/**
	* Replaces part of a string with another string.
	*
	* @param orig - The original string.
	* @param to_replace - The substring to replace.
	* @param replace_with - The string to replace any matches of the substring with.
	*
	* @returns The original string with any instances of the substring replaced.
	*/
	std::string Replace(std::string orig, std::string to_replace, std::string replace_with);

	/**
	* Transforms a string to all-lowercase.
	*
	* @param string - The original string.
	*
	* @returns The original string with all characters changed to lowercase.
	*/
	std::string ToLower(std::string string);

	/**
	* Transforms a string to all-uppercase.
	*
	* @param string - The original string.
	*
	* @returns The original string with all characters changed to uppercase.
	*/
	std::string ToUpper(std::string string);

	/**
	* Replaces any instance of `\\` in a string with `/`. Unneccessary in most cases.
	*
	* @param string - The original string.
	*
	* @returns The original string with all path separators made uniform.
	*/
	std::string FixDirectorySeparators(std::string string);

	/**
	* Splits a string around any instance of a substring.
	*
	* If `string` is "Item1|Item2|Item3", and `splitter` is "|", then the resulting string
	* list would contain the following:
	* > ```["Item1", "Item2", "Item3"]```
	*
	* Similarly, if `string` was still "Item1|Item2|Item3", and `splitter` is set to "|Item2|",
	* the resulting string list would be the following:
	* > ```["Item1", "Item3"]```
	*
	* @param string - The original string.
	* @param splitter - The string to act as the splitter.
	*
	* @returns A list of strings resulting from the split.
	*/
	std::vector<std::string> Split(std::string string, std::string splitter);

	/**
	* Determines whether or not a string contains the specified character.
	*
	* @param string - The original string.
	* @param contains - The character to look for.
	*
	* @returns Whether or not the string contains the specified character.
	*/
	bool ContainsChar(std::string string, char contains);

	/**
	* Determines whether or not a string contains the specified substring.
	*
	* @param string - The original string.
	* @param contains - The substring to look for.
	*
	* @returns Whether or not the string contains the specified substring.
	*/
	bool Contains(std::string string, std::string contains);

	/**
	* Checks the specified address in memory to see if it matches a given string, **ignoring null terminators**.
	*
	* One might be inclined to simply check the memory normally, but all strings are normally null-terminated.
	* In certain cases, it may be useful to check for a specific set of bytes that are known to always contain text
	* of a known length, that will always be followed by more data.
	*
	* @param address - Address of the start of the byte data.
	* @param to_compare - The string to check against.
	* @param size - The number of bytes within `address` to read as a string.
	*
	* @returns True if the bytes matched the provided string (w/o null terminator), False if the data didn't match.
	*/
	bool MemoryStringCompare(char* address, char* to_compare, int size);

	std::string ConvertBoolToString(bool value);

	bool StringListContains(std::vector<std::string> list, std::string string);

	std::vector<std::string> RemoveDuplicateStrings(std::vector<std::string> list);
}