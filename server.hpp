





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

class Server
{
	private:
		int server_fd;
		int port;
		std::vector<pollfd> poll_fds;

		bool setupSocket();
		void acceptClient();
		void handleClient(int client_fd);
		std::string toString(int value);
		std::string createResponse(const std::string& request);


	public:
		Server(int port);
		~Server();

		bool start();
		
};
