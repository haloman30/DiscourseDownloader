#include "components/discourse/discourse.h"

#include "components/diagnostics/logger/logger.h"
#include "components/settings/settings.h"
#include "components/utils/io/io.h"
#include "components/utils/string/string.h"
#include "components/utils/converters/converters.h"

bool last_resume_load_result = true;
DDLResumeInfo last_resume_info = DDLResumeInfo();
DDLResumeInfo current_resume_info = DDLResumeInfo();

bool DDL::Discourse::Downloader::LoadResumeFile()
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("could not get download resume info, configuration file was nullptr - download will NOT be resumed!", DDLLogLevel::Error);
		last_resume_load_result = false;
		return false;
	}

	if (!DDL::Utils::IO::IsFile(config->site_directory_root + "/resume"))
	{
		DDL::Logger::LogEvent("could not get download resume info, no resume information found - download will NOT be resumed! "
			"(note: this is normal when starting a new download)", DDLLogLevel::Warning);
		last_resume_load_result = false;
		return false;
	}

	std::vector<std::string> resume_info_lines = DDL::Utils::IO::GetFileContentsAsLines(config->site_directory_root + "/resume");

	last_resume_info = DDLResumeInfo();

	for (std::string line : resume_info_lines)
	{
		if (line.starts_with("category_id="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "category_id=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				last_resume_info.category_id = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("last_saved_topic="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "last_saved_topic=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				last_resume_info.last_saved_topic = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("topic_first_id="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "topic_first_id=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				last_resume_info.topic_first_id = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("topic_last_id="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "topic_last_id=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				last_resume_info.topic_last_id = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("topic_download_index="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "topic_download_index=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				last_resume_info.topic_download_index = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("last_user_id="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "last_user_id=", "");

			if (DDL::Converters::IsStringInt(line_value))
			{
				last_resume_info.last_user_id = DDL::Converters::StringToInt(line_value);
			}
		}
		else if (line.starts_with("download_step="))
		{
			std::string line_value = DDL::Utils::String::Replace(line, "download_step=", "");

			if (DDL::Utils::String::ToLower(line_value) == "topics")
			{
				last_resume_info.download_step = DDLResumeInfo::DownloadStep::TOPICS;
			}
			else if (DDL::Utils::String::ToLower(line_value) == "users")
			{
				last_resume_info.download_step = DDLResumeInfo::DownloadStep::USERS;
			}
		}
	}

	if (last_resume_info.download_step != DDLResumeInfo::DownloadStep::INVALID)
	{
		if (last_resume_info.download_step == DDLResumeInfo::DownloadStep::TOPICS)
		{
			if (last_resume_info.category_id != -1 && last_resume_info.last_saved_topic != -1)
			{
				last_resume_load_result = true;
				return true;
			}
		}
		else if (last_resume_info.download_step == DDLResumeInfo::DownloadStep::TOPICS)
		{
			if (last_resume_info.last_user_id != -1)
			{
				last_resume_load_result = true;
				return true;
			}
		}
	}

	DDL::Logger::LogEvent("!!!!!! could not parse download resume information file - DOWNLOAD WILL BE RESTARTED!", DDLLogLevel::Error);
	DDL::Logger::LogEvent("!!!!!! you have 10 seconds to abort the application now if you would like to perform additional diagnosis or back up the existing download folder!", DDLLogLevel::Error);
	Sleep(10 * 1000);
	last_resume_load_result = false;
	return false;
}

void DDL::Discourse::Downloader::SaveResumeFile()
{
	WebsiteConfig* config = DDL::Settings::GetSiteConfig();

	if (!config)
	{
		DDL::Logger::LogEvent("failed to save resume info - could not get website config!", DDLLogLevel::Error);
		return;
	}

	std::string resume_file_contents = "";
	{
		resume_file_contents += "category_id=" + std::to_string(current_resume_info.category_id) + "\n";
		resume_file_contents += "last_saved_topic=" + std::to_string(current_resume_info.last_saved_topic) + "\n";
		resume_file_contents += "topic_first_id=" + std::to_string(current_resume_info.topic_first_id) + "\n";
		resume_file_contents += "topic_last_id=" + std::to_string(current_resume_info.topic_last_id) + "\n";
		resume_file_contents += "topic_download_index=" + std::to_string(current_resume_info.topic_download_index) + "\n";
		resume_file_contents += "last_user_id=" + std::to_string(current_resume_info.last_user_id) + "\n";

		if (current_resume_info.download_step == DDLResumeInfo::DownloadStep::TOPICS)
		{
			resume_file_contents += "download_step=TOPICS";
		}
		else if (current_resume_info.download_step == DDLResumeInfo::DownloadStep::USERS)
		{
			resume_file_contents += "download_step=USERS";
		}
		else
		{
			resume_file_contents += "download_step=INVALID";
		}
	}

	DDL::Utils::IO::CreateNewFile(config->site_directory_root + "/resume", resume_file_contents);
}

DDLResumeInfo* DDL::Discourse::Downloader::GetLastResumeInfo()
{
	if (last_resume_load_result)
	{
		return &last_resume_info;
	}

	return nullptr;
}

DDLResumeInfo* DDL::Discourse::Downloader::GetCurrentResumeInfo()
{
	return &current_resume_info;
}