
#pragma once

#include <vector>
#include <map>
#include <string>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <errno.h>

#include "../config/ConfigParser.hpp"
#include "../config/ServerConfig.hpp"
#include "../RequestHandler/RequestHandler.hpp"

class Server
{
private:
	// Multi-server support
	std::vector<int> server_fds;							// Multiple server file descriptors
	std::vector<pollfd> poll_fds;							// Poll file descriptors
	std::map<int, const ServerConfig*> fd_to_config;		// Maps server fd to its config
	std::map<int, const ServerConfig*> client_to_server;	// Maps client fd to server config
	ConfigParser config;
		
	// Server management methods
	bool setupSockets();
	bool setupSocket(const ServerConfig& serverConfig);
	void acceptClient(int server_fd);
	void handleClient(int client_fd);
	void addServerSocketsToPoll();
	void eventLoop();
	void handleClientConnection(size_t index);
		
	// Configuration selection methods
	const ServerConfig* getServerConfigForClient(int client_fd, const std::string& raw_request);
	const ServerConfig* selectVirtualHost(const std::string& raw_request, int client_port);
		
	// Request parsing utilities
	std::string extractHost(const std::string& raw_request);
	std::string extractServerName(const std::string& raw_request);
		
	// HTTP response building
	std::string buildHttpResponse(const std::string &raw_request, const ServerConfig& serverConfig);
		
	// Utility methods
	std::string toString(int value);
	bool loadConfig(const std::string& configFile);

public:
	Server();												// Updated constructor
	~Server();
	bool start(const std::string& configFile);              // Updated start method
};