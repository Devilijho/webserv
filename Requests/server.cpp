

#include "server.hpp"
#include <arpa/inet.h>

Server::Server() : configs() {}

Server::~Server()
{
	for (std::map<int, ServerConfig>::iterator it = listeningSockets.begin(); it != listeningSockets.end(); ++it)
	{
		if (it->first != -1)
			close(it->first);
	}
}

bool Server::start(const std::vector<ServerConfig>& servers, const std::string& configFile)
{
	if (!servers.empty())
		configs = servers;
	else
	{
		if (!loadConfig(configFile))
			return false;
	}

	for (size_t i = 0; i < configs.size(); ++i)
	{
		int fd = setupSocket(configs[i]);
		if (fd < 0)
		{
			std::cerr << "[ERROR] Failed to set up socket for "
					  << configs[i].host << ":" << configs[i].port << std::endl;
			return false;
		}
		listeningSockets[fd] = configs[i]; //server_fds.push_back(fd);
		addServerSocketToPoll(fd);
	}

	std::cout << "Listening on " << listeningSockets.size() << " sockets..." << std::endl;
	eventLoop();
	return true;
}

void Server::eventLoop()
{
	while (true)
	{
		int ret = poll(&poll_fds[0], poll_fds.size(), -1);
		if (ret < 0)
		{
			if (errno == EINTR) continue;
			perror("poll");
			break;
		}

		for (int i = static_cast<int>(poll_fds.size()) - 1; i >= 0; --i)
		{
			struct pollfd& pfd = poll_fds[i];

			if (pfd.revents == 0) continue;

			if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
				handleError(pfd.fd);
				continue;
			}

			if (listeningSockets.count(pfd.fd)) { // If fd is a listening socket (server socket):
				acceptClient(pfd.fd);
				continue;
			}

			if (pfd.revents & POLLIN)
				if (!handleReadEvent(pfd.fd))
					continue;	// Connection closed or error

			// if (pfd.revents & POLLOUT)
			// 	handleWriteEvent(pfd.fd);
		}

		// cleanup closed fds
		for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end();)
		{
			if (it->fd == -1)
				it = poll_fds.erase(it);
			else
				++it;
		}
	}
}

void Server::handleError(int fd) {
	std::cerr << "[ERROR] Socket error on fd " << fd << std::endl;
	closeConnection(fd);
}

void Server::acceptClient(int server_fd)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// Accept the new client
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
		return;

	// Set client socket to non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// Add client socket to poll list
	struct pollfd client_pollfd;
	client_pollfd.fd = client_fd;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);

	clientSockets[client_fd] = new RequestHandlerData();

	// Initialize client	closeConnection(client_fd); buffer
	clientBuffers[client_fd] = "";

	// Map the client to the server config
	std::map<int, ServerConfig>::iterator server_it = listeningSockets.find(server_fd);
	if (server_it != listeningSockets.end()) {
		client_to_server_config[client_fd] = server_it->second;
	}

	std::cout << "Client connected on fd " << client_fd << std::endl;
}

void Server::closeConnection(int client_fd)
{
	// Remove from poll_fds by marking fd as -1
	for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it) {
		if (it->fd == client_fd) {
			it->fd = -1; // Mark for cleanup instead of erasing immediately
			break;
		}
	}
	// Clean up client data - no pointers, just erase directly
	clientSockets.erase(client_fd);
	clientBuffers.erase(client_fd);
	client_to_server_config.erase(client_fd);

	// Close the socket
	close(client_fd);
	std::cout << "[INFO] Closed connection on fd " << client_fd << std::endl;
}
