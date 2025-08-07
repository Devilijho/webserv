#include "CGIHandler.hpp"
#include <unistd.h>

/*check if its either a static request (html) or needs the cgi (phpCGI)*/
	/*else return an error */

int	htpp_request(void)
{
	CGIHandlerData data;

	setData(data);
	if (1)
		return (handle_static_request(data));
	else if (2)
		return (handle_dynamic_request(data));
	else
		return (BAD_REQUEST);
}

/*Sets data, this is temporary since the paths and values used are hard coded :) */

int	setData(CGIHandlerData &data)
{
	data.args_str.push_back(CGI_INTERPRETER_PATH);
	data.args_str.push_back("/Users/devilijho/Workplace/webserv/CGI/test.php");
	data.env_str.push_back("REQUEST_METHOD=GET");
	data.env_str.push_back("SCRIPT_FILENAME=/Users/devilijho/Workplace/webserv/CGI/test.php");
	data.env_str.push_back("REDIRECT_STATUS=200");
	data.env_str.push_back("CONTENT_LENGTH=0");
	data.env_str.push_back("HTTP_USER_AGENT=SANTI");
	data.env_str.push_back("SERVER_PROTOCOL=HTTP/1.1");
	data.env_str.push_back("QUERY_STRING=TESTTESTTEST");
	for (unsigned long i = 0; i < data.args_str.size(); i++)
		data.args.push_back(const_cast<char *>(data.args_str[i].c_str()));
	for (unsigned long i = 0; i < data.env_str.size(); i++)
		data.env.push_back(const_cast<char *>(data.env_str[i].c_str()));
	data.args.push_back(nullptr);
	data.env.push_back(nullptr);

	data.staticFileName = "CGI/index.html";
	return (SUCCESS);
}

int	handle_static_request(CGIHandlerData &data)
{
	std::string buffer;
	std::ostringstream oss;

	data.staticFile.open(data.staticFileName);
	if (data.staticFile.is_open() == false)
		return (ERROR);
	oss << data.staticFile.rdbuf();
	data.staticFileContent = oss.str();
	std::cout << data.staticFileContent << std::endl;
	return (OK);
}

int	handle_dynamic_request(CGIHandlerData &data)
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
		exit(child_status);
	}
	else
		waitpid(pid,&child_status, 0);
	return_value = WEXITSTATUS(child_status);
	return (return_value);
}
