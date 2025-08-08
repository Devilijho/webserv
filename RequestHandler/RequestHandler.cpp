#include "RequestHandler.hpp"

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
	data.requestMethod = "GET";
	data.staticFileName = "RequestHandler/index.html";
	data.dynamicFileName = "RequestHandler/test.php";
	std::ostringstream clientBodysize;

	clientBodysize << dataServer.client_max_body_size;

	data.args_str.push_back(CGI_INTERPRETER_PATH);
	data.args_str.push_back("/Users/devilijho/Workplace/webserv/" + data.dynamicFileName);
	data.env_str.push_back("REQUEST_METHOD=" + data.requestMethod);
	data.env_str.push_back("SCRIPT_FILENAME=/Users/devilijho/Workplace/webserv/" + data.dynamicFileName);
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

	data.staticFile.open(data.staticFileName.c_str());
	if (data.staticFile.is_open() == false)
		return (ERROR);
	oss << data.staticFile.rdbuf();
	data.staticFileContent = oss.str();
	data.staticFile.close();
	return (SUCCESS);
}

/*Executes a script such as PHP with phpCGI, returns the output trough a pipe*/

int	handle_dynamic_request(RequestHandlerData &data)
{
	pid_t pid = fork();
	int		child_status = SUCCESS;
	int		return_value;

	if (pid == -1 || pipe(data.fd) == ERROR)
		return (ERROR);
	else if (pid == 0)
	{
		close(data.fd[0]);
		dup2(data.fd[1], STDOUT_FILENO);
		child_status = execve(CGI_INTERPRETER_PATH, data.args.data(), data.env.data());
		_exit(child_status);
	}
	else
		waitpid(pid,&child_status, 0);
	return_value = WEXITSTATUS(child_status);
	return (return_value);
}
