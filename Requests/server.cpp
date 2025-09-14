

#include "server.hpp"

Server::Server() : configs() {}

Server::~Server()
{
	for (std::map<int, ServerConfig*>::iterator it = listeningSockets.begin(); it != listeningSockets.end(); ++it)
	{
		if (it->first != -1)
			close(it->first);
	}
}

bool Server::start(const std::vector<ServerConfig*>& servers, const std::string& configFile)
{
	this->error413 = 0;
	if (!servers.empty())
		configs = servers;
	else
		if (!loadConfig(configFile))
			return false;

	for (size_t i = 0; i < configs.size(); ++i)
	{
		ServerConfig* cfg = configs[i]; // pointer instead of value

		int fd = setupSocket(cfg);  // setupSocket must accept ServerConfig*
		if (fd < 0)
		{
			std::cerr << "[ERROR] Failed to set up socket for " << cfg->host << ":" << cfg->port << std::endl;
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
	std::cout << "[INFO] Entering event loop" << std::endl;

	while (g_running) {
		int ret = poll(&poll_fds[0], poll_fds.size(), 100);
		if (ret < 0)
		{
			if (errno == EINTR)
				std::cout << "[DEBUG] Poll interrupted by signal, g_running = " << g_running << std::endl;
			perror("poll");
			break;
		}
		if (ret == 0) continue; // no events occurred within timeout, loop again

		// Process all events returned by poll (reading, writing, accepting new clients, and error events)
		processPollEvents();

		// cleanup closed fds
		for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end();)
		{
			if (it->fd == -1)
				it = poll_fds.erase(it); // remove closed fd from poll list
			else
				++it;
		}
	}
	std::cout << "[INFO] Exited event loop, running cleanup" << std::endl;
	cleanup();
}

void Server::cleanup()
{
	std::cout << "[INFO] Cleaning up before exit..." << std::endl;

	// Close all client sockets
	for (std::map<int, RequestHandlerData*>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it) {
		close(it->first);
		delete it->second;
	}
	clientSockets.clear(); // Clear the map

	// Close listening sockets
	for (std::map<int, ServerConfig*>::iterator it = listeningSockets.begin(); it != listeningSockets.end(); ++it) {
		close(it->first);
	}
	listeningSockets.clear(); // Clear the map

	poll_fds.clear();
	clientBuffers.clear();
	client_to_server_config.clear();

	std::cout << "[INFO] Cleanup complete, exiting." << std::endl;
}
