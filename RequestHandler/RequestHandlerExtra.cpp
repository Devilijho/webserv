#include "RequestHandler.hpp"
#include <algorithm>
#include <cctype>

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

std::string getQueryData(RequestHandlerData &data)
{
	size_t pos;
	std::string cutFileName;
	std::string queryData = "";

	pos = data.FileName.find_last_of("?");
	if (pos == std::string::npos)
		return "";
	queryData = data.FileName.substr(pos + 1);
	cutFileName = data.FileName.substr(0, pos);
	data.FileName = cutFileName;
	return queryData;
}

std::string getETag(std::string fileName)
{
	std::string ETag(getFileDate(fileName));
	int	pos = 0;

	ETag.erase(std::remove(ETag.begin(), ETag.end(), ' '), ETag.end());
	for (int i = 0; i < ETag.length(); i++)
	{
		if (std::isdigit(ETag[i]) != 0)
		{
			pos = i;
			break ;
		}
	}
	return ETag.substr(0, );
}

void	setRequestBody(RequestHandlerData &data)
{
	size_t posStart;

	posStart = data.rawRequest.find("\r\n\r\n");
	if (posStart == std::string::npos)
		data.requestBody = "";
	else
		data.requestBody = data.rawRequest.substr(posStart + 4);
}
