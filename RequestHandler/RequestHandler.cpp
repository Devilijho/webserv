#include "RequestHandler.hpp"
#include <filesystem>
#include <unistd.h>

/*check if its either a static request (html) or needs the cgi (phpCGI)
else return an error */

int	htpp_request(ServerConfig &dataServer)
{
	RequestHandlerData data;

	setData(data, dataServer);
	if (data.requestMethod == "POST")
		return (handle_dynamic_request(data));
	else if (data.requestMethod == "GET")
		return (handle_static_request(data));
	else if (data.requestMethod == "DELETE")
		return (ERROR); // not implemented yet
	else
		return (BAD_REQUEST);
}

/*Sets data, this is temporary since the paths and values used are hard coded :) */

int	setData(RequestHandlerData &data, ServerConfig &dataServer)
{
	data.requestMethod = "POST";
	data.staticFileName = "index.html";
	data.dynamicFileName = "test.php";
	std::ostringstream clientBodysize;

	clientBodysize << dataServer.client_max_body_size;

	data.args_str.push_back(PATH_INFO);
	data.args_str.push_back(FILES_PATH + data.dynamicFileName);
	data.env_str.push_back("REQUEST_METHOD=" + data.requestMethod);
	data.env_str.push_back(std::string("SCRIPT_FILENAME=") + FILES_PATH + data.dynamicFileName);
	data.env_str.push_back("REDIRECT_STATUS=200");
	data.env_str.push_back("CONTENT_LENGTH=0");
	data.env_str.push_back("HTTP_USER_AGENT=SANTI");
	data.env_str.push_back("SERVER_PROTOCOL=HTTP/1.1");
	data.env_str.push_back("QUERY_STRING=searchedInfo");
	data.env_str.push_back("MAX_FILE_SIZE=" + clientBodysize.str());
	// data.env_str.push_back("PHP_SELF=" + dataServer.server_name);

	for (unsigned long i = 0; i < data.args_str.size(); i++)
		data.args.push_back(const_cast<char *>(data.args_str[i].c_str()));
	for (unsigned long i = 0; i < data.env_str.size(); i++)
		data.env.push_back(const_cast<char *>(data.env_str[i].c_str()));
	data.args.push_back(NULL);
	data.env.push_back(NULL);

	return (SUCCESS);
}

/*Finds the static file (html, css, ico), reads its content at returns it trough a pipe to
the server (not yet but easy to implement)*/

int	handle_static_request(RequestHandlerData &data)
{
	std::string buffer;
	std::ostringstream oss;

	if (data.staticFileName == "./www/")
		data.staticFileName = "./www/index.html";
	data.staticFile.open(data.staticFileName);
	if (data.staticFile.is_open() == false)
	{
		std::cout << "error?" << std::endl;
		return (ERROR);
	}
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

	if (pipe(data.fd) == ERROR)
		return (ERROR);
	pid = fork();
	if (pid == -1)
		return (ERROR);
	else if (pid == 0)
	{
		close(data.fd[0]);
		dup2(data.fd[1], STDOUT_FILENO);
		child_status = execve(PATH_INFO, data.args.data(), data.env.data());
		_exit(child_status);
	}
	else
		waitpid(pid,&child_status, 0);
	close(data.fd[1]);
	while (read(data.fd[0], &buffer, 1) > 0)
		data.FileContent += buffer;
	return_value = WEXITSTATUS(child_status);
	return (return_value);
}
