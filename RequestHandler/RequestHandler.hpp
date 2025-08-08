#pragma once

#include "../config/ConfigParser.hpp"
#include "../config/ServerConfig.hpp"
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

#define	OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500

#define SUCCESS 0
#define ERROR 1

//Linux
#define CGI_INTERPRETER_PATH "/usr/bin/php-cgi"
//MacOS
// #define CGI_INTERPRETER_PATH "/opt/homebrew/bin/php-cgi"

struct RequestHandlerData
{
	std::vector<std::string> args_str;
	std::vector<std::string> env_str;
	std::vector<char *> args;
	std::vector<char *> env;

	std::ifstream staticFile;
	std::string staticFileContent;
	std::string	staticFileName;

	std::string dynamicFileName;

	std::string requestMethod;

	int	fd[2];
};

int	handle_dynamic_request(RequestHandlerData &data);
int	handle_static_request(RequestHandlerData &data);
int	setData(RequestHandlerData &data, ServerConfig &dataServer);
int	htpp_request(ServerConfig &dataServer);
