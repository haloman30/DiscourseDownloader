#include "io.h"

// Windows does not define the S_ISREG and S_ISDIR macros in stat.h, so we do.
// We have to define _CRT_INTERNAL_NONSTDC_NAMES 1 before #including sys/stat.h
// in order for Microsoft's stat.h to define names like S_IFMT, S_IFREG, and S_IFDIR,
// rather than just defining  _S_IFMT, _S_IFREG, and _S_IFDIR as it normally does.
#define _CRT_INTERNAL_NONSTDC_NAMES 1
#include <sys/stat.h>
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#include <direct.h>
#include <fstream>
#include <filesystem>

#include "components/utils/string/string.h"

bool DDL::Utils::IO::FileExists(std::string name)
{
	struct stat file_stat;

	int file_status = stat(name.c_str(), &file_stat);

	if (file_status == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DDL::Utils::IO::IsFile(std::string path)
{
	if (FileExists(path))
	{
		struct stat file_stat;

		stat(path.c_str(), &file_stat);

		if (S_ISREG(file_stat.st_mode))
		{
			return true;
		}
	}

	return false;
}

bool DDL::Utils::IO::IsDirectory(std::string path)
{
	if (FileExists(path))
	{
		struct stat file_stat;

		stat(path.c_str(), &file_stat);

		if (S_ISDIR(file_stat.st_mode))
		{
			return true;
		}
	}

	return false;
}

bool DDL::Utils::IO::CreateNewFile(std::string filename, std::string file_contents)
{
	std::ofstream file = std::ofstream(filename, std::ios::out | std::ios::trunc);

	if (!file.bad())
	{
		file << file_contents;
		file.close();

		return true;
	}
	else
	{
		file.close();
		return false;
	}
}

bool DDL::Utils::IO::CreateNewFileBinaryMode(std::string filename, std::string file_contents)
{
	std::ofstream file = std::ofstream(filename, std::ios::out | std::ios::trunc | std::ios::binary);

	if (!file.bad())
	{
		file << file_contents;
		file.close();

		return true;
	}
	else
	{
		file.close();
		return false;
	}
}

std::vector<std::string> DDL::Utils::IO::GetFileContentsAsLines(std::string path)
{
	std::vector<std::string> file_lines;

	std::ifstream file_stream(path);

	std::string line;

	while (getline(file_stream, line))
	{
		file_lines.push_back(line);
	}

	return file_lines;
}

std::string DDL::Utils::IO::GetFileContentsAsString(std::string path)
{
	std::string file_contents = "";

	std::ifstream file_stream(path);

	std::string line;

	while (getline(file_stream, line))
	{
		file_contents += line + "\n";
	}

	return file_contents;
}

bool DDL::Utils::IO::ReadBinaryFile(std::string path, void* data, int64_t* size)
{
	std::ifstream file;

	file.open(path, std::ios::in | std::ios::binary);

	if (file.good())
	{
		file.seekg(0, std::ios::beg);
		*size = file.tellg();

		data = malloc(*size);
		file.read((char*)data, *size);

		file.close();
		return true;
	}
	else
	{
		file.close();
		return false;
	}
}

void DDL::Utils::IO::ValidatePath(std::string path)
{
	std::vector<std::string> path_elements = DDL::Utils::String::Split(NormalizePath(path), "/");

	std::string root = "";

	for (int i = 0; i < path_elements.size(); i++)
	{
		std::string path = root + path_elements.at(i);
		_mkdir(path.c_str());
		root = path + "/";
	}
}

std::string DDL::Utils::IO::NormalizePath(std::string path)
{
	std::string path_normalized = path;

	path_normalized = DDL::Utils::String::Replace(path_normalized, "\\", "/");
	path_normalized = DDL::Utils::String::Replace(path_normalized, "../", "@@@_UP_ONE_LEVEL_@@@");
	path_normalized = DDL::Utils::String::Replace(path_normalized, "./", "/");
	path_normalized = DDL::Utils::String::Replace(path_normalized, "@@@_UP_ONE_LEVEL_@@@", "../");

	if (path_normalized.starts_with("/"))
	{
		path_normalized = "." + path_normalized;
	}

	return path_normalized;
}

std::string DDL::Utils::IO::GetFileName(std::string path)
{
	std::string filename = "";

	std::string file_path = DDL::Utils::IO::NormalizePath(path);

	if (file_path.ends_with("/"))
	{
		file_path.erase(file_path.begin() + file_path.find_last_of('/'));
	}

	if (DDL::Utils::String::Contains(file_path, "/"))
	{
		filename = file_path.substr(file_path.find_last_of('/') + 1);
	}
	else
	{
		filename = file_path;
	}

	return filename;
}

std::string DDL::Utils::IO::GetFileNameWithoutExtension(std::string path)
{
	std::string filename = GetFileName(path);

	if (IsFile(path) || !FileExists(path))
	{
		filename = filename.substr(0, filename.find_last_of('.'));
	}

	return filename;
}

std::string DDL::Utils::IO::GetFileExtension(std::string path)
{
	std::string extension = "";

	if (IsFile(path))
	{
		path = NormalizePath(path);

		std::string filename = path.substr(path.find_last_of('/'));

		if (DDL::Utils::String::ContainsChar(filename, '.'))
		{
			extension = filename.substr(filename.find_last_of('.'));
		}
	}

	return extension;
}