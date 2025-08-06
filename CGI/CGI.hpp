#pragma once

#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <ostream>
#include <iostream>
#include <sys/wait.h>

#define	OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500

// #define CGI_INTERPRETER_PATH "/usr/bin/php-cgi"
#define CGI_INTERPRETER_PATH "/opt/homebrew/bin/php-cgi"

int		handle_phpCGI(void);
int		htpp_request(void);
