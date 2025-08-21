

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
#include <netdb.h>

#include "../config/ConfigParser.hpp"
#include "../config/ServerConfig.hpp"
#include "../RequestHandler/RequestHandler.hpp"

struct RequestHandlerData;

class Server
{
	public:
		Server();
		~Server();

		bool start(	const std::vector<ServerConfig>& servers,
					const std::string& configFile = "default.conf");

	private:
		// --- Configuration ---
		std::vector<ServerConfig> configs;
		
		// std::vector<int> server_fds;						// Multiple server file descriptors
		std::map<int, ServerConfig> listeningSockets;		// FD -> config
		std::map<int, RequestHandlerData*> clientSockets;	// FD -> client state

		// --- Sockets ---
		// std::vector<int> server_fds;
		std::vector<struct pollfd> poll_fds;

		// NEW: per-client buffers to accumulate request data
		std::map<int, std::string>  clientBuffers;    // client fd -> raw request so far
		std::map<int, ServerConfig> client_to_server_config;

		// --- Setup ---
		int setupSocket(const ServerConfig& cfg);
		struct addrinfo* resolveAddress(const ServerConfig& cfg);
		void addServerSocketToPoll(int fd);
		bool loadConfig(const std::string& configFile);

		// --- Event loop ---
		void eventLoop();
		void handleWriteEvent(int fd);
		void handleError(int fd);
		bool handleReadEvent(int fd);
		//
		// --- Client handling ---
		void acceptClient(int server_fd);

		void closeConnection(int client_fd);
		std::string buildHttpResponse(const std::string &raw_request, const ServerConfig& serverConfig);
		
		int clientFdToServerFd(int client_fd);


};

struct ClientData {
    ServerConfig server_config;
    int state;
    std::string request_buffer;
    std::string response_buffer;
    size_t bytes_sent;
};

enum ClientState {
    CLIENT_READING_REQUEST,
    CLIENT_WRITING_RESPONSE
};