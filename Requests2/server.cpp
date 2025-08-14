#include "server.hpp"
#include <unistd.h>

int server_fd;
		int port;
		std::vector<pollfd> poll_fds;
		ConfigParser config;


Server::Server(int _port): server_fd(-1), port(_port) {}

Server::~Server() {
	if (server_fd != -1)
		close(server_fd);
}

bool Server::start()
{
	if (!config.parseFile("default.conf") || !config.isValid()) {
		std::cerr << "Invalid configuration. Stopping." << std::endl;
		return false;
	}
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		return (perror("socket"), false);

}

std::string Server::buildHttpResponse(const std::string &raw_request)
{
	std::istringstream req_stream(raw_request);
	std::string method, path, protocol;
	req_stream >> method >> path >> protocol;
	if (method.empty() || path.empty()) {
		return "HTTP/1.1 400 Bad Request0\r\n\r\n";
	}
}
