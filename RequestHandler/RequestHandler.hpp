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
	std::string query;

	std::string rawRequest;
	std::string requestBody;

	int	fdOut[2];
	int fdIn[2];

	std::string requestBuffer;   // Accumulated request
	std::string responseBuffer;  // Response to send
	size_t bytesSent;
};

/*Main functions */

void errorHandling(RequestHandlerData &data, const ServerConfig &srv, int code);
void	handle_delete_request(RequestHandlerData &data);
int	handle_dynamic_request(RequestHandlerData &data, const char *path_cgi);
int	handle_static_request(RequestHandlerData &data);
int	setData(RequestHandlerData &data, const ServerConfig &dataServer, const LocationConfig *loc);
int	htpp_request(ServerConfig &dataServer);
std::string http_response(RequestHandlerData &data, ServerConfig &srv);


/*Helper functions */

void	setRequestBody(RequestHandlerData &data);
void setQueryData(RequestHandlerData &data);
std::string getRequestContentType(RequestHandlerData &data);
std::string getETag(std::string fileName);
std::string getContentType(std::string);
std::string getDate(void);
std::string getFileDate(std::string fileName);
std::string toString(int value);
std::string getStatusMessage(int code);
std::string getAbsolutePath(std::string);

/*Server functions */
int send_all(int socket, const char *buffer, size_t length, int flags);
std::string read_all(int socket);
