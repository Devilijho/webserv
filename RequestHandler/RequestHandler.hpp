#pragma once

#include "../config/ConfigParser.hpp"
#include "../config/ServerConfig.hpp"
#include "../Requests/server.hpp"
#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <ostream>
#include <iostream>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>

#define SUCCESS 0
#define ERROR 1

//Linux
// #define PATH_INFO "/usr/bin/php-cgi"

//MacOS
#define PATH_INFO "/opt/homebrew/bin/php-cgi"


struct RequestHandlerData
{
	std::vector<std::string> args_str;
	std::vector<std::string> env_str;
	std::vector<char *> args;
	std::vector<char *> env;

	std::ifstream staticFile;
	std::string	FileName;
	std::string FileContentType;

	std::string FileContent;
	std::string HeadContent;

	std::string requestMethod;

	int	fd[2];
};

void errorHandling(RequestHandlerData &data, std::string errorFile, std::string HeadContent);
int	handle_dynamic_request(RequestHandlerData &data);
int	handle_static_request(RequestHandlerData &data);
int	setData(RequestHandlerData &data, ServerConfig &dataServer);
int	htpp_request(ServerConfig &dataServer);
std::string fileContentTypeHandler(std::string);
