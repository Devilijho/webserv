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

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "CGIHandler.hpp"
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


	public:
		Server(int port);
		~Server();

		bool start();

};
