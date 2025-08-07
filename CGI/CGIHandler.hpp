#pragma once

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

// #define CGI_INTERPRETER_PATH "/usr/bin/php-cgi"
#define CGI_INTERPRETER_PATH "/opt/homebrew/bin/php-cgi"

struct CGIHandlerData
{
	std::vector<std::string> args_str;
	std::vector<std::string> env_str;
	std::vector<char *> args;
	std::vector<char *> env;

	std::ifstream staticFile;
	std::string staticFileContent;
	std::string	staticFileName;

	int	fd[2];
};

int	handle_dynamic_request(CGIHandlerData &data);
int	handle_static_request(CGIHandlerData &data);
int	setData(CGIHandlerData &data);
int	htpp_request(void);
