#include "string.h"

#include <iostream>
#include <algorithm>
#include <string.h>

bool DDL::Utils::String::StartsWith(std::string string, std::string starts_with, bool case_insensetive)
{
	if (case_insensetive)
	{
		std::transform(string.begin(), string.end(), string.begin(), ::tolower);
		std::transform(starts_with.begin(), starts_with.end(), starts_with.begin(), ::tolower);
	}

	if (string.find(starts_with) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DDL::Utils::String::EndsWith(std::string string, std::string ends_with, bool case_insensetive)
{
	if (case_insensetive)
	{
		std::transform(string.begin(), string.end(), string.begin(), ::tolower);
		std::transform(ends_with.begin(), ends_with.end(), ends_with.begin(), ::tolower);
	}

	if (string.length() >= ends_with.length())
	{
		if (string.compare(string.length() - ends_with.length(), ends_with.length(), ends_with) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

std::string DDL::Utils::String::ToLower(std::string string)
{
	std::string lower = string;
	
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

	return lower;
}

std::string DDL::Utils::String::ToUpper(std::string string)
{
	std::string upper = string;

	std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

	return upper;
}

std::string DDL::Utils::String::Replace(std::string orig, std::string to_replace, std::string replace_with)
{
	std::string replaced = orig;

	if (to_replace.empty())
	{
		return replaced;
	}

	size_t start_pos = 0;

	while ((start_pos = replaced.find(to_replace, start_pos)) != std::string::npos)
	{
		replaced.replace(start_pos, to_replace.length(), replace_with);
		start_pos += replace_with.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}

	return replaced;
}

std::vector<std::string> DDL::Utils::String::Split(std::string string, std::string splitter)
{
	std::vector<std::string> split_result;

	std::string base_str = string;

	int pos = 0;

	while ((pos = base_str.find(splitter)) != std::string::npos)
	{
		std::string str = base_str.substr(0, pos);
		split_result.push_back(str);
		base_str.erase(0, pos + splitter.length());
	}

	split_result.push_back(base_str);

	return split_result;
}

std::string DDL::Utils::String::FixDirectorySeparators(std::string string)
{
	std::string fixed_string = string;

	fixed_string = DDL::Utils::String::Replace(fixed_string, "\\", "/");

	return fixed_string;
}

bool DDL::Utils::String::ContainsChar(std::string string, char contains)
{
	bool contains_char = false;

	for (int i = 0; i < string.length(); i++)
	{
		if (string[i] == contains)
		{
			contains_char = true;
			break;
		}
	}

	return contains_char;
}

bool DDL::Utils::String::Contains(std::string string, std::string contains)
{
	if (string.find(contains) != std::string::npos)
	{
		return true;
	}

	return false;
}

bool DDL::Utils::String::MemoryStringCompare(char* address, char* to_compare, int size)
{
	if (memcmp(address, to_compare, size) == 0)
	{
		return true;
	}

	return false;
}

std::string DDL::Utils::String::ConvertBoolToString(bool value)
{
	if (value)
	{
		return "true";
	}
	
	return "false";
}

bool DDL::Utils::String::StringListContains(std::vector<std::string> list, std::string string)
{
	for (std::string list_item : list)
	{
		if (list_item == string)
		{
			return true;
		}
	}

	return false;
}

std::vector<std::string> DDL::Utils::String::RemoveDuplicateStrings(std::vector<std::string> list)
{
	std::vector<std::string> pruned_list = std::vector<std::string>();

	for (std::string list_item : list)
	{
		if (!StringListContains(pruned_list, list_item))
		{
			pruned_list.push_back(list_item);
		}
	}

	return pruned_list;
}