
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

struct RequestHandlerData;

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

		bool loadConfig(const std::string& configFile);
		void addServerSocketToPoll();
		void eventLoop();
		void handleClientConnection(size_t index);

		// Error handling
		std::string getStatusMessage(int code);
		void serveError(RequestHandlerData &data, const ServerConfig &srv, int code);

		// Config helpers
		bool isMethodAllowed(const LocationConfig &loc, const std::string &method);
		bool isBodySizeAllowed(const std::string &raw_request, size_t max_size);

		// Response building
		std::string buildHttpResponse(const std::string &raw_request);
		std::string buildResponseString(const RequestHandlerData &data);


	public:
		Server(int port);
		~Server();

		bool start();

};

