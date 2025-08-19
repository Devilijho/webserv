#include "RequestHandler.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>

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
	struct stat info;
	stat(fileName.c_str(), &info);
	std::string time(ctime(&info.st_mtime));
	return (time.substr(0, time.length() - 1));
}

/*gets the variables sent by the POST method */

void setQueryData(RequestHandlerData &data)
{
	size_t pos;
	std::string cutFileName;
	std::string queryData = "";

	pos = data.FileName.find_last_of("?");
	if (pos == std::string::npos)
		data.query = "";
	queryData = data.FileName.substr(pos + 1);
	cutFileName = data.FileName.substr(0, pos);
	data.FileName = cutFileName;
	data.query = queryData;
}

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
	ETagNum = std::stol(FileDate.substr(pos, FileDate.length()));
	for (size_t i = 0; i < fileName.length(); i++)
		ETagNum += fileName[i];
	srand(ETagNum);
	EtagStream << rand();
	return "\"" + EtagStream.str() + "\"";
}

void	setRequestBody(RequestHandlerData &data)
{
	size_t posStart;

	std::cout << data.rawRequest;
	posStart = data.rawRequest.find("\r\n\r\n");
	if (posStart == std::string::npos)
		data.requestBody = "";
	else
		data.requestBody = data.rawRequest.substr(posStart + 4);
}

std::string toString(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string getRequestContentType(RequestHandlerData &data)
{
	size_t posStart = data.rawRequest.find("Content-Type: ");
	size_t posEnd = data.rawRequest.find("Upgrade-Insecure-Requests: ");
	if (posStart != std::string::npos && posEnd != std::string::npos)
		return data.rawRequest.substr(posStart + 14, posEnd - posStart - 16);
	else
		return "";
}
