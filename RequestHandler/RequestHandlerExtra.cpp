#include "RequestHandler.hpp"

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
	// std::cout << "QUERY->>>>>>>" << std::endl;
	// std::cout << data.FileName << std::endl;
	// std::cout << queryData << std::endl;
	return queryData;
}
