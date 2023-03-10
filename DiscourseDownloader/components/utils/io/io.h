#include <string>
#include <vector>

/**
* Utilities relating to reading/writing to and from files.
*/
namespace DDL::Utils::IO
{
	/**
	* Checks if the specified file or directory exists.
	*
	* @param name - The path to the file or directory.
	*
	* @returns `true` if the file exists, `false` if it does not
	*/
	bool FileExists(std::string name);

	/**
	* Checks if a given path refers to a regular file.
	*
	* @param path - The path to check.
	*
	* @returns `true` if the given path refers to a regular file. If the path does not exist or does not
	*     refer to a regular file, then this will return `false`.
	*/
	bool IsFile(std::string path);

	/**
	* Checks if a given path refers to a directory.
	*
	* @param path - The path to check.
	*
	* @returns `true` if the given path refers to a directory. If the path does not exist or does not
	*     refer to a directory, then this will return `false`.
	*/
	bool IsDirectory(std::string path);

	/**
	* Creates a file with the specified contents, or overwrites an existing file if it already exists.
	*
	* @param filename - The path to the file to create.
	* @param file_contents - The contents to write to the file.
	*
	* @returns `true` if the file was created successfully, otherwise returns `false`.
	*/
	bool CreateNewFile(std::string filename, std::string file_contents);

	/**
	* Creates a file with the specified contents, or overwrites an existing file if it already exists.
	* Identical to CreateNewFile, but with the `std::ios::binary` flag set.
	*
	* @param filename - The path to the file to create.
	* @param file_contents - The contents to write to the file.
	*
	* @returns `true` if the file was created successfully, otherwise returns `false`.
	*/
	bool CreateNewFileBinaryMode(std::string filename, std::string file_contents);

	/**
	* Reads a file as raw binary data.
	*
	* @param path - The path of the file to read from.
	* @param data - Address to write data to.
	* @param size - Pointer of the size of the resource.
	*
	* @returns `true` if the file was read without error, `false` if the file could not be read.
	*/
	bool ReadBinaryFile(std::string path, void* data, int64_t* size);

	/**
	* Reads a file as a list of strings. Can be used to easily read a set of lines from a plain
	* text file.
	*
	* @param path - The path of the file to read from.
	*
	* @returns A list of strings, with each string representing a line in the file.
	*/
	std::vector<std::string> GetFileContentsAsLines(std::string path);

	/**
	* Reads a file as a string.
	*
	* @param path - The path of the file to read from.
	*
	* @returns The file contents, represented as a string.
	*/
	std::string GetFileContentsAsString(std::string path);

	/**
	* Validates that the specified path exists.
	*
	* The path can be any relative or absolute path. The function will automatically create any and all
	* required subdirectories to ensure that the path can be written to.
	*
	* @param path - The path to validate.
	*/
	void ValidatePath(std::string path);

	/**
	* Normalizes a path string.
	*
	* This function will take a file path, and ensure it is consistent prior to being interpreted by other
	* code within the application. Primarily, it will replace `\\` with `/`.
	*
	* @param path - The path to normalize.
	*
	* @returns The normalized file path.
	*/
	std::string NormalizePath(std::string path);

	/**
	* Retrieves the name of the file or folder that a given path refers to.
	*
	* For instance, using this function on `C:/Windows/explorer.exe` would return `explorer.exe`. Running
	* this function on `C:/Program Files (x86)/Microsoft Games/Halo` would return `Halo`.
	*
	* @param path - The path to retrieve the file name for.
	*
	* @returns The file or folder name.
	*/
	std::string GetFileName(std::string path);

	/**
	* Retrieves the name of the file or folder that a given path refers to, excluding the file's extension (if applicable).
	*
	* For instance, using this function on `C:/Windows/explorer.exe` would return `explorer`. Running
	* this function on `C:/Program Files (x86)/Microsoft Games/Halo` would return `Halo`.
	*
	* @param path - The path to retrieve the file name for.
	*
	* @returns The file or folder name.
	*/
	std::string GetFileNameWithoutExtension(std::string path);

	/**
	* Retrieves the extension of a file.
	*
	* For instance, using this function on `C:/Windows/explorer.exe` would return `exe`. Running this
	* function on a directory or file with no extension would return an empty string.
	*
	* @param path - The path to retrieve the extension for.
	*
	* @returns The file extension.
	*/
	std::string GetFileExtension(std::string path);
}