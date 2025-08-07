#pragma once

#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <ostream>
#include <iostream>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>

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
	std::vector<char *> args;
	std::vector<char *> env;
	int	fd[2];
};

int	handle_dynamic_request(CGIHandlerData &Data);
int	handle_static_request(std::string fileName);
int	setData(CGIHandlerData &Data);
int	htpp_request(void);
