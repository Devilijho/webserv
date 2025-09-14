#include "RequestHandler.hpp"
#include <complex>
#include <sys/poll.h>
#include <unistd.h>
#include <vector>

/*Sets data and the variables needed for the dynamic file handling*/

int	setData(RequestHandlerData &data, const ServerConfig &dataServer, const LocationConfig *loc)
{
	data.StatusLine = "HTTP/1.1 200 OK";
	setRequestBody(data);
	setQueryData(data);

	data.args_str.push_back(data.FileName);
	data.env_str.push_back("REQUEST_METHOD=" + data.requestMethod);
	data.env_str.push_back(std::string("SCRIPT_FILENAME=") + data.FileName);
	data.env_str.push_back("REDIRECT_STATUS=CGI");
	data.env_str.push_back("SERVER_PROTOCOL=HTTP/1.1");
	data.env_str.push_back("PATH_INFO=" + loc->cgi_path);
	data.env_str.push_back("QUERY_STRING=" + data.query);
	data.env_str.push_back("UPLOAD_DIR=" + loc->upload_dir);
	data.env_str.push_back("GATEWAY_INTERFACE=CGI/1.1");
	data.env_str.push_back("SERVER_NAME=" + dataServer.server_name);
	data.env_str.push_back("REMOTE_ADDR=" + dataServer.host);
	data.env_str.push_back("SERVER_PORT=" + toString(dataServer.port));
	data.env_str.push_back("CONTENT_TYPE=" + getRequestContentType(data));
	if (data.requestMethod == "POST")
		data.env_str.push_back("CONTENT_LENGTH=" + toString(data.requestBody.length()));
	else if (data.requestMethod == "GET")
		data.env_str.push_back("CONTENT_LENGTH=0");
	data.env_str.push_back("PHPRC=./www/script/config");

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

	if (getFileType(data.FileName) == DIRECTORY)
		data.FileName = (std::string(data.FileName) + std::string("index.html"));
	data.staticFile.open(data.FileName.c_str());
	if (data.staticFile.is_open() == false)
		return (ERROR);
	oss << data.staticFile.rdbuf();
	data.FileContent = oss.str();
	data.staticFile.close();
	return (SUCCESS);
}

/*Executes a script such as PHP with phpCGI, returns the output trough a pipe*/

int	handle_dynamic_request(RequestHandlerData &data, const char *path_cgi, Server *srv)
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
		close(data.fdIn[0]);
		close(data.fdOut[1]);
		for (unsigned long i = 0; i < srv->poll_fds.size() ; i++)
			close(srv->poll_fds[i].fd);
		child_status = execve(path_cgi, data.args.data(), data.env.data());
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

void errorHandling(RequestHandlerData &data,const ServerConfig *srv, int code)
{
	std::map<int, std::string>::const_iterator it = srv->error_pages.find(code);
	// std::string returnData;
	if (it != srv->error_pages.end())
		data.FileName = it->second;
	else
		data.FileName = srv->default_error_page;
	data.FileContentType = "html";
	data.StatusLine = getStatusMessage(code);
	if (access(data.FileName.c_str(), R_OK | F_OK) != 0)
		return ;
	if (handle_static_request(data) != 0)
		return ;
}

/*assembles the http response and returns it as a string */

std::string getRedirectStatusMessage(int code)
{
    switch (code) {
        case 301: return "HTTP/1.1 301 Moved Permanently";
        case 302: return "HTTP/1.1 302 Found";
        case 303: return "HTTP/1.1 303 See Other";
        case 307: return "HTTP/1.1 307 Temporary Redirect";
        case 308: return "HTTP/1.1 308 Permanent Redirect";
        default:  return "HTTP/1.1 302 Found";
    }
}

std::string http_response(RequestHandlerData &data, ServerConfig &srv)
{
    // ✅ HANDLE REDIRECTS FIRST
    if (data.is_redirect && !data.redirect_location.empty()) {
        std::string response = getRedirectStatusMessage(data.statusCode);
        response += "\r\nLocation: " + data.redirect_location;
        response += "\r\nContent-Length: 0";
        response += "\r\nConnection: keep-alive";
        response += "\r\nDate: " + getDate();
        response += "\r\nServer: " + srv.server_name;
        response += "\r\n\r\n";
        return response;
    }

    // ✅ RESPONSE NORMAL (CÓDIGO EXISTENTE)
    std::string response =
    data.StatusLine
    + "\r\nConnection: keep-alive"
    + "\r\nLast-Modified: " + getFileDate(data.FileName)
    + "\r\nDate: " + getDate()
    + "\r\nContent-Lenght: " + toString(data.FileContent.size())
    + "\r\nContent-Type: " + (data.FileContentType == "js" ? ("application/javascript") : ("text/" + data.FileContentType))
    + "\r\nAccept-Ranges: bytes"
    + "\r\nETag: " + getETag(data.FileName)
    + "\r\nServer: " + srv.server_name
    + "\r\n\r\n" + data.FileContent;
    return response;
}


/*handles delete requests */

void	handle_delete_request(RequestHandlerData &data)
{
	std::remove(data.FileName.c_str());
	data.StatusLine = "HTTP/1.1 204 No Content";
	data.FileContent = "";
}
