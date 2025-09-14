

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
#include <csignal>
#include <sys/poll.h>
// #include <arpa/inet.h>


// #include "../config/ConfigParser.hpp"
// #include "../config/ServerConfig.hpp"
#include "../config/ConfigParser.hpp"
#include "../config/ServerConfig2.hpp"
#include "../RequestHandler/RequestHandler.hpp"

static volatile sig_atomic_t g_running = 1;

struct RequestHandlerData;

class Server
{
	public:
		Server();
		~Server();

		bool start(	const std::vector<ServerConfig*>& servers,
					const std::string& configFile = "default.conf");

		std::vector<struct pollfd> poll_fds;				// poll() structures for event loop
		std::map<int, RequestHandlerData*> clientSockets;	// Client FD -> state/data
		int error413;
	private:
		// --- Server Configuration ---
		std::vector<ServerConfig*> configs;					// List of server configs
		std::map<int, ServerConfig*> listeningSockets;		// Server socket FD -> config
		std::map<int, ServerConfig*> client_to_server_config;// Client FD -> selected server config

		// --- Client Connections ---
		std::map<int, std::string> clientBuffers;			// Client FD -> accumulated request buffer

		// --- Sockets & Polling ---

		// --- Setup / Initialization ---
		bool loadConfig(const std::string& configFile);
		int setupSocket(const ServerConfig* cfg);
		struct addrinfo* getBindAddress(const ServerConfig* cfg);
		void addServerSocketToPoll(int fd);

		// --- Event loop ---
		void eventLoop();
		void processPollEvents();
		bool handleWriteEvent(int fd);
		void handleError(int fd);
		bool handleReadEvent(int fd);
		//
		// --- Client handling ---
		void acceptClient(int server_fd);
		void closeConnection(int client_fd);

		bool isMethodAllowed(const LocationConfig* loc, const std::string& method) const;
		std::string getFullPath(const LocationConfig* loc, const ServerConfig* srv, const std::string& path) const;
		void handleResource(RequestHandlerData& data, const LocationConfig* loc, const ServerConfig* srv, const std::string& method);
		void handleFileRequest(RequestHandlerData& data, const LocationConfig* loc, const ServerConfig* srv, const std::string& method, bool isAllowed);

		std::string buildHttpResponse(const std::string &raw_request, const ServerConfig* serverConfig);

		std::string toString(int value);
		bool accumulateRequest(int client_fd, char* buffer, ssize_t bytes_read);
		std::string processRequest(int client_fd);
		bool sendResponse(int client_fd, const std::string& response);
		bool hasCompleteRequest(int client_fd);
		bool isBodyComplete(const std::string &buffer, size_t headers_end);

		void cleanup();
};

struct ClientData {
	ServerConfig server_config;
	int state;
	std::string request_buffer;
	std::string response_buffer;
	size_t bytes_sent;
};
