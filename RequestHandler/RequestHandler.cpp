#include "RequestHandler.hpp"

/*Sets data, this is temporary since the paths and values used are hard coded :) */

int	setData(RequestHandlerData &data, ServerConfig &dataServer)
{
	data.FileContentType = getContentType(data.FileName);
	data.StatusLine = "HTTP/1.1 200 OK";
	std::string query = getQueryData(data);
	setRequestBody(data);

	data.args_str.push_back(PATH_INFO);
	data.args_str.push_back(data.FileName);
	data.env_str.push_back("REQUEST_METHOD=" + data.requestMethod);
	data.env_str.push_back(std::string("SCRIPT_FILENAME=") + data.FileName);
	data.env_str.push_back("REDIRECT_STATUS=200");
	if (data.requestMethod == "POST")
		data.env_str.push_back("CONTENT_LENGTH=" + toString(data.requestBody.length()));
	else if (data.requestMethod == "GET")
		data.env_str.push_back("CONTENT_LENGTH=0");
	data.env_str.push_back("HTTP_USER_AGENT=SANTI");
	data.env_str.push_back("SERVER_PROTOCOL=HTTP/1.1");
	data.env_str.push_back("QUERY_STRING=" + query);
	data.env_str.push_back("MAX_FILE_SIZE=" + toString(dataServer.client_max_body_size));
	data.env_str.push_back("GATEWAY_INTERFACE=CGI/1.1");
	data.env_str.push_back("SERVER_NAME=localhost");
	data.env_str.push_back("SERVER_PORT=8080");
	// data.env_str.push_back("CONTENT_TYPE=" + getRequestContentType(data));
	data.env_str.push_back("CONTENT_TYPE=application/x-www-form-urlencoded");

	for (unsigned long i = 0; i < data.args_str.size(); i++)
		data.args.push_back(const_cast<char *>(data.args_str[i].c_str()));
	for (unsigned long i = 0; i < data.env_str.size(); i++)
		data.env.push_back(const_cast<char *>(data.env_str[i].c_str()));
	data.args.push_back(NULL);
	data.env.push_back(NULL);
	data.FileContentType = getContentType(data.FileName);
	return (SUCCESS);
}

/*Finds the static file (html, css, ico), reads its content at returns it trough a pipe to
the server (not yet but easy to implement)*/


int	handle_static_request(RequestHandlerData &data)
{
	std::string buffer;
	std::ostringstream oss;

	if (data.FileName == "./www/")
		data.FileName = "./www/index.html";
	data.staticFile.open(data.FileName.c_str());
	if (data.staticFile.is_open() == false)
		return (ERROR);
	oss << data.staticFile.rdbuf();
	data.FileContent = oss.str();
	data.staticFile.close();
	return (SUCCESS);
}

/*Executes a script such as PHP with phpCGI, returns the output trough a pipe*/

int	handle_dynamic_request(RequestHandlerData &data)
{
	pid_t pid;
	int		return_value;
	char	buffer;
	int		child_status = SUCCESS;

	if (pipe(data.fdOut) == ERROR || pipe(data.fdIn) == ERROR)
		return (ERROR);
	pid = fork();
	if (pid == -1)
		return (ERROR);
	else if (pid == 0)
	{
		close(data.fdOut[0]);
		close(data.fdIn[1]);
		dup2(data.fdOut[1], STDOUT_FILENO);
		dup2(data.fdIn[0], STDIN_FILENO);
		child_status = execve(PATH_INFO, data.args.data(), data.env.data());
		_exit(child_status);
	}
	else
	{
		close(data.fdOut[1]);
		close(data.fdIn[0]);
		if (!data.requestBody.empty())
			write(data.fdIn[1], data.requestBody.c_str(), data.requestBody.size());
		close(data.fdIn[1]);
		while (read(data.fdOut[0], &buffer, 1) > 0)
			data.FileContent += buffer;
		close(data.fdOut[0]);
		waitpid(pid, &child_status, 0);
		return_value = WEXITSTATUS(child_status);
	}
	return (return_value);
}

/*fills some variables and returns a error page */

void errorHandling(RequestHandlerData &data, std::string errorFile, std::string HeadContent)
{
	std::string returnData;
	data.FileName = errorFile;
	data.StatusLine = HeadContent;
	if (access(errorFile.c_str(), R_OK | F_OK) != 0)
		return ;
	if (handle_static_request(data) != 0)
		return ;
}

std::string http_response(RequestHandlerData &data, ServerConfig &srv)
{
	// std::cout << "Server name: " << srv.server_name << std::endl;
	std::string response =
	data.StatusLine
	+ "\r\nConnection: keep-alive"
	+ "\r\nLast-Modified: " + getFileDate(data.FileName)
	+ "\r\nDate: " + getDate()
	+ "\r\nContent-Lenght: " + toString(data.FileContent.size())
	+ "\r\nContent-Type: text/" + data.FileContentType
	// + "\r\nAge: " + "24"
	// + "\r\nLocation :/index.html"
	// + "\r\nRetry-After: "
	+ "\r\nAccept-Ranges: bytes"
	+ "\r\nETag: " + getETag(data.FileName)
	+ "\r\nProxy-Authenticate: Basic realm=Dev"
	+ "\r\nServer: " + srv.server_name
	// + "\r\nVary: "
	+ "\r\nWWW-Authenticate: Basic realm=User Visible Realm"
	+ "\r\n\r\n" + data.FileContent;
	return response;
}
