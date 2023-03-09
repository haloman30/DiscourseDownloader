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

#include <sys/stat.h>
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