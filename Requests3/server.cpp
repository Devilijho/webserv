

#include "server.hpp"
#include <arpa/inet.h>

Server::Server() : configs() {}

Server::~Server()
{
	for (size_t i = 0; i < server_fds.size(); ++i)
	{
		if (server_fds[i] != -1)
			close(server_fds[i]);
	}
}

bool Server::start(const std::vector<ServerConfig>& servers)
{
	configs = servers;

	for (size_t i = 0; i < configs.size(); ++i)
	{
		if (!setupSocket(configs[i]))
		{
			std::cerr << "[ERROR] Failed to set up socket for "
					  << configs[i].host << ":" << configs[i].port << std::endl;
			return false;
		}
	}

	std::cout << "[INFO] All sockets ready, entering event loop..." << std::endl;
	eventLoop();
	return true;
}


bool Server::setupSocket(const ServerConfig& cfg)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return perror("socket"), false;

	fcntl(fd, F_SETFL, O_NONBLOCK);

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(cfg.port);
	addr.sin_addr.s_addr = inet_addr(cfg.host.c_str());

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		return perror("bind"), close(fd), false;

	if (listen(fd, 10) < 0)
		return perror("listen"), close(fd), false;

	server_fds.push_back(fd);
	addServerSocketToPoll(fd);

	std::cout << "[INFO] Listening on " << cfg.host << ":" << cfg.port << std::endl;
	return true;
}

void Server::addServerSocketToPoll(int fd) {
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	poll_fds.push_back(pfd);
}

void Server::eventLoop() {
	while (true) {
		int ret = poll(&poll_fds[0], poll_fds.size(), -1);
		if (ret < 0) {
			perror("poll");
			break;
		}

		for (size_t i = 0; i < poll_fds.size(); ++i) {
			if (poll_fds[i].revents & POLLIN) {
				handlePollEvent(poll_fds[i]);
			}
		}

		// cleanup closed fds
		poll_fds.erase(std::remove_if(poll_fds.begin(), poll_fds.end(),
						[](const struct pollfd& p){ return p.fd == -1; }),
						poll_fds.end());
	}
}


void Server::handlePollEvent(struct pollfd& pfd) {
	// Is it a server socket?
	if (std::find(server_fds.begin(), server_fds.end(), pfd.fd) != server_fds.end()) {
		acceptClient(pfd.fd);
	} else {
		handleClient(pfd.fd);
		pfd.fd = -1; // mark for cleanup
	}
}


void Server::acceptClient(int server_fd) {
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		perror("accept");
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	struct pollfd client_pfd;
	client_pfd.fd = client_fd;
	client_pfd.events = POLLIN;
	poll_fds.push_back(client_pfd);

	std::cout << "[INFO] Accepted client fd=" << client_fd << std::endl;
}


void Server::handleClient(int client_fd) {
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t bytes = read(client_fd, buffer, sizeof(buffer) - 1);

	if (bytes <= 0) {
		close(client_fd);
		return;
	}

	std::string request(buffer);
	std::cout << "[DEBUG] Request:\n" << request << std::endl;

	std::string body = "<html><body><h1>Hello from Webserv!</h1></body></html>";
	std::ostringstream resp;
	resp << "HTTP/1.1 200 OK\r\n"
		 << "Content-Type: text/html\r\n"
		 << "Content-Length: " << body.size() << "\r\n"
		 << "Connection: close\r\n\r\n"
		 << body;

	send(client_fd, resp.str().c_str(), resp.str().size(), 0);
	close(client_fd);
}