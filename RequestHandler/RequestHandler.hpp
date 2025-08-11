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

#define	OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500

#define SUCCESS 0
#define ERROR 1

//Linux
// #define PATH_INFO "/usr/bin/php-cgi"
// #define FILES_PATH "/home/safuente/Documents/webserv/RequestHandler/"

//MacOS
#define PATH_INFO "/opt/homebrew/bin/php-cgi"
#define FILES_PATH "./www/"
#define ERROR_PATH "./www/error/"


struct RequestHandlerData
{
	std::vector<std::string> args_str;
	std::vector<std::string> env_str;
	std::vector<char *> args;
	std::vector<char *> env;

	std::ifstream staticFile;
	std::string	FileName;

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
