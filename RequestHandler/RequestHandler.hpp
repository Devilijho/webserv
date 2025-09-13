#pragma once

#include "../config/ConfigParser.hpp"
#include "../config/ServerConfig2.hpp"
#include "../Requests/server.hpp"
#include <fstream>
#include <sys/poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include <sys/stat.h>
#include <fstream>
#include <dirent.h>

#define SUCCESS 0
#define ERROR 1

#define DIRECTORY 0
#define FILE 2

class Server;

struct RequestHandlerData
{
	std::vector<std::string> args_str;
	std::vector<std::string> env_str;
	std::vector<char *> args;
	std::vector<char *> env;

	std::ifstream staticFile;
	std::string	FileName;
	std::string FileContentType;
	std::string path;

	std::string FileContent;
	std::string StatusLine;

	std::string requestMethod;
	std::string query;

	std::string rawRequest;
	std::string requestBody;

	int	fdOut[2];
	int fdIn[2];

	std::string requestBuffer;
	std::string responseBuffer;
	size_t bytesSent;

	bool is_redirect;
	std::string redirect_location;
	int statusCode;
};

/*Main functions */

void errorHandling(RequestHandlerData &data, const ServerConfig *srv, int code);
void	handle_delete_request(RequestHandlerData &data);
int	handle_dynamic_request(RequestHandlerData &data, const char *path_cgi, Server *srv);
int	handle_static_request(RequestHandlerData &data);
int	setData(RequestHandlerData &data, const ServerConfig &dataServer, const LocationConfig *loc);
std::string http_response(RequestHandlerData &data, ServerConfig &srv);


/*Helper functions */

int	getFileType(std::string filename);
void	setRequestBody(RequestHandlerData &data);
void setQueryData(RequestHandlerData &data);
bool	isAllowedMethod(std::string method, const LocationConfig *loc);
void	setCurrentDirFiles(RequestHandlerData &data, const ServerConfig &srv, const LocationConfig *loc);
std::string getRequestContentType(RequestHandlerData &data);
std::string getETag(std::string fileName);
std::string getContentType(std::string);
std::string getDate(void);
std::string getFileDate(std::string fileName);
std::string toString(int value);
std::string getStatusMessage(int code);
std::string getAbsolutePath(std::string);
