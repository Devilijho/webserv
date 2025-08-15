

#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <algorithm>

#include "../config/ConfigParser.hpp"
#include "../config/ServerConfig.hpp"
#include "../RequestHandler/RequestHandler.hpp"
#include "Request.hpp"

class Server
{
	public:
		Server();
		~Server();

		bool start(const std::vector<ServerConfig>& servers);

	private:
		// --- Configuration ---
		std::vector<ServerConfig> configs;

		// --- Sockets ---
		std::vector<int> server_fds;
		std::vector<struct pollfd> poll_fds;

		// --- Setup ---
		bool setupSocket(const ServerConfig& cfg);
		void addServerSocketToPoll(int fd);

		// --- Event loop ---
		void eventLoop();
		void handlePollEvent(struct pollfd& pfd);

		// --- Client handling ---
		void acceptClient(int server_fd);
		void handleClient(int client_fd);
};