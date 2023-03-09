#pragma once

#include <string>
#include <optional>
#include <vector>

#include "components/3rdparty/rapidjson/document.h"

/**
* Namespace containing utility functions for working with JSON documents.
*/
namespace DDL::Utils::Json
{
	/**
	* Retrieves a value from a JSON document as a string.
	* 
	* This function attempts to locate a string value from a JSON document. It also provides
	* checking and error-catching to prevent an exception if a value is not found.
	* 
	* @param document - Pointer to the JSON document to check for the property.
	* @param key - The name of the property to retrieve.
	* 
	* @returns The string value of the specified key. If the key was not found, or the key
	* does not refer to a string, then an empty string is returned.
	*/
	std::string GetValueAsString(rapidjson::Document* document, const char* key);

	/**
	* Retrieves a value from a JSON document as a 32-bit signed integer.
	*
	* This function attempts to locate an int value from a JSON document. It also provides
	* checking and error-catching to prevent an exception if a value is not found.
	*
	* @param document - Pointer to the JSON document to check for the property.
	* @param key - The name of the property to retrieve.
	*
	* @returns The int value of the specified key. If the key was not found, or the key
	* does not refer to an int, then this will return `0`.
	*/
	int GetValueAsInt(rapidjson::Document* document, const char* key);

	/**
	* Retrieves a value from a JSON document as a boolean.
	*
	* This function attempts to locate a boolean value from a JSON document. It also provides
	* checking and error-catching to prevent an exception if a value is not found.
	*
	* @param document - Pointer to the JSON document to check for the property.
	* @param key - The name of the property to retrieve.
	*
	* @returns The boolean value of the specified key. If the key was not found, or the key
	* does not refer to a boolean, then this will return `false`.
	*/
	bool GetValueAsBool(rapidjson::Document* document, const char* key);

	/**
	* Retrieves a value from a JSON document as a 64-bit unsigned integer.
	*
	* This function attempts to locate a uint64 value from a JSON document. It also provides
	* checking and error-catching to prevent an exception if a value is not found.
	*
	* @param document - Pointer to the JSON document to check for the property.
	* @param key - The name of the property to retrieve.
	*
	* @returns The uint64 value of the specified key. If the key was not found, or the key
	* does not refer to a uint64, then this will return `0`.
	*/
	uint64_t GetValueAsUInt64(rapidjson::Document* document, const char* key);

	/**
	* Serializes a JSON document to a string.
	* 
	* @param object - The JSON document to serialize.
	* 
	* @returns - A string representation of the provided JSON document.
	*/
	std::string Serialize(rapidjson::Document* object);

	/**
	* Serializes a JSON object to a string.
	*
	* @param object - The JSON object to serialize.
	*
	* @returns - A string representation of the provided JSON object.
	*/
	std::string Serialize(rapidjson::Value* object);

	/**
	* Converts a vector into a JSON string.
	* 
	* This will not escape the strings in any way, shape, or form.
	*
	* @param list - The vector to format as a string.
	*
	* @returns - A string, representing a JSON array with the same contents as the provided list.
	*/
	std::string FormatVectorAsJSONArray(std::vector<std::string> list);

	/**
	* Parses a JSON array into a list of strings.
	*
	* @param list - The optional string from an SQL row.
	*
	* @returns - A vector containing the strings within the JSON array. If the array was empty or could not be parsed, or
	*     if the optional string had no value, then an empty list is returned.
	*/
	std::vector<std::string> ParseJSONArray(std::optional<std::string> json);
}