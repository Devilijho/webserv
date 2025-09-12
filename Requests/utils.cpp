

#include "server.hpp"


void Server::processPollEvents()
{
	for (int i = static_cast<int>(poll_fds.size()) - 1; i >= 0; --i)
	{
		struct pollfd& pfd = poll_fds[i];

		if (pfd.revents == 0) continue;  // If no events occurred, skip this fd

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

		if (pfd.revents & POLLOUT)
			handleWriteEvent(pfd.fd);
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
	std::map<int, ServerConfig*>::iterator server_it = listeningSockets.find(server_fd);
	if (server_it != listeningSockets.end()) {
		client_to_server_config[client_fd] = server_it->second;
	}

	std::cout << "Client connected on fd " << client_fd << std::endl;
}