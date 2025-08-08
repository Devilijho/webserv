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

#include "../config/ConfigParser.hpp"
#include "../config/ServerConfig.hpp"
#include "../RequestHandler/RequestHandler.hpp"
#include "Request.hpp"

class Server
{
	private:
		int server_fd;
		int port;
		std::vector<pollfd> poll_fds;
		ConfigParser config;

		bool setupSocket();
		void acceptClient();
		void handleClient(int client_fd);
		std::string toString(int value);
		std::string buildHttpResponse(const std::string &raw_request);

		bool loadConfig(const std::string& configFile);
		void addServerSocketToPoll();
		void eventLoop();
		void handleClientConnection(size_t index);

	public:
		Server(int port);
		~Server();

		bool start();

};
