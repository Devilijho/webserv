#include "CGI.hpp"
#include <unistd.h>

/*check if its either a static request (html) or needs the cgi (phpCGI)*/
	/*else return an error */

int	htpp_request(void)
{
	if (1)
	{
		return (handle_phpCGI());
	}
	else
		return (BAD_REQUEST);
}

int	handle_phpCGI(void)
{
		char *args[] = {(char *)CGI_INTERPRETER_PATH, (char *)"/Users/devilijho/Workplace/webserv/CGI/test.php", NULL};
		char *env[] = {
			(char *)"REQUEST_METHOD=GET",
			(char *)"SCRIPT_FILENAME=/Users/devilijho/Workplace/webserv/CGI/test.php",
			(char *)"REDIRECT_STATUS=200",
			(char *)"CONTENT_LENGTH=0",
			(char *)"HTTP_USER_AGENT=SANTI",
			(char *)"SERVER_PROTOCOL=HTTP/1.1",
			NULL};

		pid_t pid = fork();
		int fd[2];
		if (pid == -1 || pipe(fd) == 1)
			return (INTERNAL_SERVER_ERROR);
		else if (pid == 0)
		{
			close(fd[0]);
			dup2(STDIN_FILENO, fd[1]);
			execve(CGI_INTERPRETER_PATH, args, env);
			exit(0);
		}
		else
			wait(&pid);
		// close(fd[1]);
		dup2(STDOUT_FILENO, fd[0]);
		return (OK);
}
