#include "CGIHandler.hpp"
#include <vector>

/*check if its either a static request (html) or needs the cgi (phpCGI)*/
	/*else return an error */

int	htpp_request(void)
{
	CGIHandlerData data;

	setData(data);
	if (1)
		return (handle_dynamic_request(data));
	else if (2)
		return (handle_static_request("index.html"));
	else
		return (BAD_REQUEST);
}

int	setData(CGIHandlerData &data)
{
	std::vector<std::string> vars = {CGI_INTERPRETER_PATH, "/Users/devilijho/Workplace/webserv/CGI/test.php", NULL};
	std::vector<std::string> env = {
		"REQUEST_METHOD=GET",
		"SCRIPT_FILENAME=/Users/devilijho/Workplace/webserv/CGI/test.php",
		"REDIRECT_STATUS=200",
		"CONTENT_LENGTH=0",
		"HTTP_USER_AGENT=SANTI",
		"SERVER_PROTOCOL=HTTP/1.1",
		"QUERY_STRING=TESTTESTTEST",
		NULL};

	for (int i = vars.size() ; i > 0; i--)
		data.args.push_back(vars[i].c_str());
	return (SUCCESS);
}

int	handle_static_request(std::string fileName)
{
	if (open(fileName.c_str(), O_RDWR) == -1)
		return (ERROR);

	return (OK);
}

int	handle_dynamic_request(CGIHandlerData &data)
{
	pid_t pid = fork();
	if (pid == -1 || pipe(data.fd) == 1)
		return (ERROR);
	else if (pid == 0)
	{

		std::cout << data.args[1] << std::endl;
		close(data.fd[0]);
		dup2(data.fd[1], STDOUT_FILENO);
		execve(CGI_INTERPRETER_PATH, data.args, data.env);
		exit(0);
	}
	else
		wait(&pid);
	dup2(STDIN_FILENO, data.fd[0]);
	return (OK);
}
