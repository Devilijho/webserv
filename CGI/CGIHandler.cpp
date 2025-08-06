#include "CGIHandler.hpp"

/*check if its either a static request (html) or needs the cgi (phpCGI)*/
	/*else return an error */

int	htpp_request(void)
{
	t_CGIHandlerData Data;

	setData(Data);
	if (1)
		return (handle_dynamic_request(Data));
	else if (2)
		return (handle_static_request("index.html"));
	else
		return (BAD_REQUEST);
}

int	setData(t_CGIHandlerData &Data)
{
	char *args[] = {(char *)CGI_INTERPRETER_PATH, (char *)"/Users/devilijho/Workplace/webserv/CGI/test.php", NULL};
	char *env[] = {
		(char *)"REQUEST_METHOD=GET",
		(char *)"SCRIPT_FILENAME=/Users/devilijho/Workplace/webserv/CGI/test.php",
		(char *)"REDIRECT_STATUS=200",
		(char *)"CONTENT_LENGTH=0",
		(char *)"HTTP_USER_AGENT=SANTI",
		(char *)"SERVER_PROTOCOL=HTTP/1.1",
		(char *)"QUERY_STRING=TESTTESTTEST",
		NULL};

	Data.args = args;
	Data.env = env;
	return (SUCCESS);
}

int	handle_static_request(std::string fileName)
{

	if (open(fileName.c_str(), O_RDWR) == -1)
		return (ERROR);

	return (OK);
}

int	handle_dynamic_request(t_CGIHandlerData &Data)
{
	pid_t pid = fork();
	if (pid == -1 || pipe(Data.fd) == 1)
		return (ERROR);
	else if (pid == 0)
	{
		// close(Data.fd[0]);
		// dup2(Data.fd[1], STDOUT_FILENO);
		std::cout << "" << execve(CGI_INTERPRETER_PATH, Data.args, Data.env);
		exit(0);
	}
	else
		wait(&pid);
	// dup2(STDIN_FILENO, Data.fd[0]);
	return (OK);
}
