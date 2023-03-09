#pragma once

#define DR_FAILED(result) (result < 0) //!< Macro to check if a #DDLResult indicates a failed operation.
#define DR_SUCCEEDED(result) (result >= 0) //!< Macro to check if a #DDLResult indicates a successful operation.

/**
* Enumerator listing all possible status codes.
* 
* 0 will always indicate a successful operation.
* Anything above 0 indicates a successful operation, but with warnings.
* Anything below 0 indicates a failure.
*/
enum DDLResult
{
	Success_OK = 0, //!< Indicates a successful operation.
	Error_Generic = -1, //!< Indicates a general, unspecified failure.
	Error_BadClassConstruction = -2, //!< Indicates a class was not constructed properly.
	Error_FileNotFound = -3, //!< Indicates the specified file could not be found.
	Error_NullPointer = -4, //!< Indicates that a required object pointer was `nullptr`.
	Error_FileInvalid = -5, 
};