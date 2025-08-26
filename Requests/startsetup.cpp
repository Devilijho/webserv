

#include "server.hpp"
#include <arpa/inet.h>


bool Server::loadConfig(const std::string& configFile) //check les methodes de Config
{
	ConfigParser parser;

	if (!parser.parseFile(configFile) || !parser.isValid())
	{
		std::cerr << "[ERROR] Invalid configuration in " << configFile << std::endl;
		return false;
	}
	configs = parser.getServers();
	if (configs.empty())
	{
		std::cerr << "[ERROR] Configuration file " << configFile
				  << " did not define any servers.\n";
		return false;
	}
	return true;
}

struct addrinfo* Server::getBindAddress(const ServerConfig& cfg)
{
	struct addrinfo hints, *res;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_INET;		// IPv4 only
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_flags	= AI_PASSIVE;		// For binding

	std::ostringstream port_str; // Ports in sockets are strings, not integers.
	port_str << cfg.port;

	const char* host = cfg.host.empty() ? NULL : cfg.host.c_str(); // If config has no host, we pass NULL → AI_PASSIVE makes it bind to all available interfaces.
	if (getaddrinfo(host, port_str.str().c_str(), &hints, &res) != 0) {
		perror("getaddrinfo");
		return NULL;
	}
	return res; // contains a sockaddr_in that has both IP address and port number, ready to be used in bind
}

int Server::setupSocket(const ServerConfig& cfg)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	fcntl(fd, F_SETFL, O_NONBLOCK);

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct addrinfo* res = getBindAddress(cfg);
	if (!res) {
		close(fd);
		return -1;
	}

	if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) { // associates a socket with a specific IP address and port number
		perror("bind");
		freeaddrinfo(res);
		close(fd);
		return -1;
	}
	freeaddrinfo(res);

	if (listen(fd, 10) < 0)
		return perror("listen"), close(fd), -1;

	std::cout << "[INFO] Listening on " << cfg.host << ":" << cfg.port << std::endl;
	return fd;
}


void Server::addServerSocketToPoll(int fd) {
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	poll_fds.push_back(pfd);
}