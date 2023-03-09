#include "main.h"

#include <iostream>
#include <string>

#include "components/settings/settings.h"
#include "components/diagnostics/logger/logger.h"
#include "components/utils/network/network.h"
#include "components/utils/io/io.h"

int main(int args_count, char** args)
{
	// init
	{
		DDL::Logger::StartLogger();
		DDL::Settings::LoadAllConfigurations();
	}

	int http_response = -1;

	std::string response = DDL::Utils::Network::PerformHTTPRequest("https://haloman30.com/", &http_response);

	if (http_response == 200)
	{
		DDL::Utils::IO::CreateNewFile("./response.html", response);
	}
	else
	{
		DDL::Logger::LogEvent("got invalid http response '" + std::to_string(http_response) + "'");
	}

	// shutdown
	{
		DDL::Settings::CleanupConfigurations();
		DDL::Logger::ShutdownLogger();
	}
}