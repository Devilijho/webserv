#include "RequestHandler.hpp"
#include <unistd.h>

/*get the extension of a file */

std::string getContentType(std::string name)
{
	size_t pos;
	pos = name.find_last_of(".");

	if (pos == std::string::npos || pos == 0)
		return "html";
	else
		return name.substr(pos + 1);
}

/*Gets the current time and returns it as a string*/

std::string getDate(void)
{
	time_t now = time(0);
	std::string time(ctime(&now));
	return (time.substr(0, time.length() - 1));
}

/*Gets the last time the file was modified and returns it as a string */
std::string getFileDate(std::string fileName)
{
	struct stat file_stat;

	memset(&file_stat, 0, sizeof(file_stat));
    if (stat(fileName.c_str(), &file_stat) != 0)
    {
		std::string time(ctime(&file_stat.st_mtime));
		return (time.substr(0, time.length() - 1));
	}
	return ("Thu, 01 Jan 1970 00:00:00 GMT");
}

/*gets the variables sent by the POST method */

void setQueryData(RequestHandlerData &data)
{
	size_t pos;
	std::string cutFileName;
	std::string queryData = "";

	pos = data.FileName.find_last_of("?");
	queryData = data.FileName.substr(pos + 1);
	cutFileName = data.FileName.substr(0, pos);
	data.FileName = cutFileName;
	data.query = queryData;
	if (pos == std::string::npos)
		data.query = "";
}

/*returns an random generate ETag based on a filename and time of last modified */

std::string getETag(std::string fileName)
{
	std::string FileDate(getFileDate(fileName));
	std::ostringstream EtagStream;
	long	ETagNum;
	size_t	pos = 0;

	FileDate.erase(std::remove(FileDate.begin(), FileDate.end(), ' '), FileDate.end());
	FileDate.erase(std::remove(FileDate.begin(), FileDate.end(), ':'), FileDate.end());
	for (size_t i = 0; i < FileDate.length(); i++){
		if (std::isdigit(FileDate[i]) != 0)
		{ pos = i; break; } }
	ETagNum = std::strtol((FileDate.substr(pos, FileDate.length())).c_str(), NULL, 10);
	for (size_t i = 0; i < fileName.length(); i++)
		ETagNum += fileName[i];
	srand(ETagNum);
	EtagStream << rand();
	return "\"" + EtagStream.str() + "\"";
}

/*set the request body from the http raw request */

void	setRequestBody(RequestHandlerData &data)
{
	size_t posStart;

	posStart = data.rawRequest.find("\r\n\r\n");
	if (posStart == std::string::npos)
		data.requestBody = "";
	else
		data.requestBody = data.rawRequest.substr(posStart + 4);
}

/*converts an int to an string and returns it */

std::string toString(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

/*Returns the contentType of the http raw request on a string */

std::string getRequestContentType(RequestHandlerData &data)
{
	size_t posStart = data.rawRequest.find("Content-Type: ");
	if (posStart != std::string::npos)
	{
		size_t lineEnd = data.rawRequest.find("\r\n", posStart);
		if (lineEnd != std::string::npos) {
			return data.rawRequest.substr(posStart + 14, lineEnd - posStart - 14);
		}
	}
	return "";
}

/*return an string of the status message based on the parameter code)*/

std::string getStatusMessage(int code)
{
	switch (code) {
		case 400: return "HTTP/1.1 400 Bad Request";
		case 403: return "HTTP/1.1 403 Forbidden";
		case 404: return "HTTP/1.1 404 Not Found";
		case 405: return "HTTP/1.1 405 Method Not Allowed";
		case 508: return "HTTP/1.1 508 Loop Detected";
		case 413: return "HTTP/1.1 413 Payload Too Large";
		case 500: return "HTTP/1.1 500 Internal Server Error";
		default:  return "HTTP/1.1 501 Not Implemented";
	}
}

/*return a type of file taking as parameter the filename (relative or absolute path) */

int	getFileType(std::string filename)
{
	struct stat s;

	if( stat(filename.c_str(),&s) == 0 )
	{
		if( s.st_mode & S_IFDIR )
			return (DIRECTORY);
		else if( s.st_mode & S_IFREG )
			return (FILE);
		else
			return (ERROR);
	}
	else
		return ERROR;
}

/*returns true or false depending if all the methods located in the current directory are allowed or not */

bool	isAllowedMethod(std::string method, const LocationConfig *loc)
{
	std::vector<std::string>::const_iterator it;

	for (it = loc->methods.begin(); it != loc->methods.end(); it++)
	{
		if (*it == method)
		{
			return true;
		}
	}
	return false;
}

/* fills data.FileContent with the files that are currently on the directory */

void	setCurrentDirFiles(RequestHandlerData &data, const ServerConfig &srv, const LocationConfig *loc)
{
	std::string files;
	DIR *directory;
	struct dirent *directoryIt;

	directory = opendir(data.FileName.c_str());
	if (directory == NULL)
		return ;
	directoryIt = readdir(directory); // this is to get rid of the "." directory
	if (directoryIt == NULL)
		return ;
	(void)loc;
	while (1)
	{
		directoryIt = readdir(directory);
		std::cout << data.path[data.path.length() - 1];
		if (directoryIt == NULL)
			break ;
		if (data.path[data.path.length() - 1] == '/')
			files = files + "<a href=http://localhost:" + toString(srv.port) + data.path  +  directoryIt->d_name + ">" + directoryIt->d_name + "</a>\n";
		else
			files = files + "<a href=http://localhost:" + toString(srv.port) + data.path  + "/" +  directoryIt->d_name + ">" + directoryIt->d_name + "</a>\n";
	}
	data.FileContent = files;
	closedir(directory);
}
