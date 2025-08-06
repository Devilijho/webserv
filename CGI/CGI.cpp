#include "CGI.hpp"

void	htpp_request(void)
{
	char *args[] = {(char *)"/opt/homebrew/bin/php-cgi", (char *)"/Users/devilijho/Workplace/webserv/CGI/index.php", NULL};
	char *env[] = {
		(char *)"REQUEST_METHOD=GET",
		(char *)"SCRIPT_FILENAME=/Users/devilijho/Workplace/webserv/CGI/index.php",
		(char *)"REDIRECT_STATUS=200",
		(char *)"CONTENT_LENGTH=0",
		(char *)"HTTP_USER_AGENT=SANTI",
		(char *)"SERVER_PROTOCOL=HTTP/1.1",
		NULL};

	pid_t pid = fork();
	if (pid == 0)
		execve("/opt/homebrew/bin/php-cgi", args, env);
	else
		wait(&pid);
}
