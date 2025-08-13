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
#include <sys/stat.h>

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
	std::string StatusLine;

	std::string requestMethod;

	std::string rawRequest;
	std::string requestBody;

	int	fdOut[2];
	int fdIn[2];
};

/*Main functions */

void errorHandling(RequestHandlerData &data, std::string errorFile, std::string HeadContent);
int	handle_dynamic_request(RequestHandlerData &data);
int	handle_static_request(RequestHandlerData &data);
int	setData(RequestHandlerData &data, ServerConfig &dataServer);
int	htpp_request(ServerConfig &dataServer);
void	setRequestBody(RequestHandlerData &data);

/*Helper functions */

std::string getContentType(std::string);
std::string getDate(void);
std::string getFileDate(std::string fileName);
std::string getQueryData(RequestHandlerData &data);
