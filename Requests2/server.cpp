/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pde-vara <pde-vara@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 15:43:31 by pde-vara          #+#    #+#             */
/*   Updated: 2025/08/13 18:11:29 by pde-vara         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <sstream>
#include <iostream>
#include <cstdlib>

// Updated constructor - no longer needs specific port
Server::Server() : config() {}

Server::~Server() {
	// Close all server sockets
	for (size_t i = 0; i < server_fds.size(); ++i) {
		if (server_fds[i] != -1) {
			close(server_fds[i]);
		}
	}
}

bool Server::start(const std::string& configFile)
{
	if (!loadConfig(configFile))
		return false;

	if (!setupSockets())
		return false;

	addServerSocketsToPoll();

	std::cout << "Servidor iniciado con " << server_fds.size() << " configuraciones" << std::endl;

	eventLoop();

	return true;
}

bool Server::loadConfig(const std::string& configFile)
{
	if (!config.parseFile(configFile) || !config.isValid())
	{
		std::cerr << "Invalid configuration. Stopping." << std::endl;
		return false;
	}
	return true;
}

bool Server::setupSockets()
{
	const std::vector<ServerConfig>& servers = config.getServers();
		
	for (size_t i = 0; i < servers.size(); ++i) {
		if (!setupSocket(servers[i])) {
			std::cerr << "Failed to setup socket for server " << i << std::endl;
			return false;
		}
	}
	return true;
}

bool Server::setupSocket(const ServerConfig& serverConfig)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) 
		return perror("socket"), false;

	fcntl(server_fd, F_SETFL, O_NONBLOCK); // Socket no bloqueante

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
		
	// Use server's host configuration
	if (serverConfig.host.empty() || serverConfig.host == "0.0.0.0") {
		addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		if (inet_pton(AF_INET, serverConfig.host.c_str(), &addr.sin_addr) <= 0) {
			std::cerr << "Invalid address: " << serverConfig.host << std::endl;
			close(server_fd);
			return false;
		}
	}
		
	addr.sin_port = htons(serverConfig.port); // host to network short

	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(server_fd);
		return false;
	}
	if (listen(server_fd, 10) < 0) {
		perror("listen");
		close(server_fd);
		return false;
	}
	// Store server fd and its configuration mapping
	server_fds.push_back(server_fd);
	fd_to_config[server_fd] = &serverConfig;
		
	std::cout << "Servidor escuchando en " << serverConfig.host << ":" << serverConfig.port << std::endl;
	return true;
}

void Server::addServerSocketsToPoll()
{
	for (size_t i = 0; i < server_fds.size(); ++i) {
		struct pollfd server_pollfd;
		server_pollfd.fd = server_fds[i];
		server_pollfd.events = POLLIN;
		poll_fds.push_back(server_pollfd);
	}
}

void Server::handleClientConnection(size_t index)
{
	int client_fd = poll_fds[index].fd;
	handleClient(client_fd);
	close(client_fd);
		
	// Remove client from server mapping
	client_to_server.erase(client_fd);
		
	poll_fds.erase(poll_fds.begin() + index);
}

void Server::eventLoop()
{
	while (true)
	{
		int ret = poll(&poll_fds[0], poll_fds.size(), -1);
		if (ret < 0)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			if (poll_fds[i].revents & POLLIN)
			{
				// Check if this is a server socket
				bool is_server_socket = false;
				for (size_t j = 0; j < server_fds.size(); ++j) {
					if (poll_fds[i].fd == server_fds[j]) {
						acceptClient(server_fds[j]);
						is_server_socket = true;
						break;
					}
				}
				
				if (!is_server_socket) {
					handleClientConnection(i);
					// Adjust index after removal
					if (i > 0) --i;
				}
			}
		}
	}
}

void Server::acceptClient(int server_fd)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("accept");
		}
		return;
	}
		
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
		
	// Map client to server configuration
	auto config_it = fd_to_config.find(server_fd);
	if (config_it != fd_to_config.end()) {
		client_to_server[client_fd] = config_it->second;
	}
		
	struct pollfd client_pollfd;
	client_pollfd.fd = client_fd;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);
		
	std::cout << "Cliente conectado en fd " << client_fd << std::endl;
}

void Server::handleClient(int client_fd) {
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	if (bytes_read <= 0) {
		if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("read");
		}
		return;
	}

	std::string raw_request(buffer);
	std::cout << "Pedido recibido:\n" << raw_request << std::endl;

	// Get the server configuration for this client
	const ServerConfig* serverConfig = getServerConfigForClient(client_fd, raw_request);
	if (!serverConfig) {
		std::cerr << "No server configuration found for client " << client_fd << std::endl;
		return;
	}

	std::string response = buildHttpResponse(raw_request, *serverConfig);
	send(client_fd, response.c_str(), response.length(), 0);
}

const ServerConfig* Server::getServerConfigForClient(int client_fd, const std::string& raw_request)
{
	// First, try to get from client mapping
	auto it = client_to_server.find(client_fd);
	if (it != client_to_server.end()) {
		// If we have multiple servers, do virtual host selection
		if (server_fds.size() > 1) {
			return selectVirtualHost(raw_request, it->second->port);
		}
		return it->second;
	}
		
	// Fallback: use first server configuration
	const std::vector<ServerConfig>& servers = config.getServers();
	return servers.empty() ? nullptr : &servers[0];
}

const ServerConfig* Server::selectVirtualHost(const std::string& raw_request, int client_port)
{
	std::string host = extractHost(raw_request);
	std::string server_name = extractServerName(raw_request);
		
	const std::vector<ServerConfig>& servers = config.getServers();
	const ServerConfig* default_server = nullptr;
		
	// Find matching server configuration
	for (size_t i = 0; i < servers.size(); ++i) {
		const ServerConfig& srv = servers[i];
		
		// Port must match
		if (srv.port != client_port) {
			continue;
		}
		
		// First server on this port becomes default
		if (!default_server) {
			default_server = &srv;
		}
		
		// Check server_name match
		if (!server_name.empty()) {
			for (size_t j = 0; j < srv.server_names.size(); ++j) {
				if (srv.server_names[j] == server_name) {
					return &srv; // Exact server name match
				}
			}
		}
		
		// Check host match
		if (!host.empty() && !srv.host.empty()) {
			if (srv.host == host || srv.host == "0.0.0.0") {
				return &srv; // Host match
			}
		}
	}
		
	// Return default server for this port
	return default_server;
}

std::string Server::extractHost(const std::string& raw_request)
{
	size_t host_pos = raw_request.find("Host: ");
	if (host_pos == std::string::npos) {
		return "";
	}
		
	host_pos += 6; // Skip "Host: "
	size_t end_pos = raw_request.find("\r\n", host_pos);
	if (end_pos == std::string::npos) {
		end_pos = raw_request.find("\n", host_pos);
	}
		
	if (end_pos == std::string::npos) {
		return "";
	}
		
	std::string host = raw_request.substr(host_pos, end_pos - host_pos);
		
	// Remove port from host if present for host matching
	size_t colon_pos = host.find(':');
	if (colon_pos != std::string::npos) {
		return host.substr(0, colon_pos);
	}
		
	return host;
}

std::string Server::extractServerName(const std::string& raw_request)
{
	// For HTTP/1.1, server name is typically the same as host
	return extractHost(raw_request);
}

std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string Server::buildHttpResponse(const std::string &raw_request, const ServerConfig& serverConfig)
{
	// --- Parse method and path from raw_request manually ---
	std::istringstream req_stream(raw_request);
	std::string method, path, protocol;
	req_stream >> method >> path >> protocol;
	if (method.empty() || path.empty()) {
		return "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n"; // Fixed typo
	}

	// Use the selected server configuration instead of hardcoded [0]
	const ServerConfig &srv = serverConfig;

	// Check if path matches a location
	const LocationConfig *loc = srv.findLocation(path);
	if (!loc) {
		// Handle 404 with proper error page lookup
		auto error_it = srv.error_pages.find(404);
		if (error_it != srv.error_pages.end()) {
			std::ifstream errFile(error_it->second.c_str());
			if (errFile.is_open()) {
				std::stringstream errBuf;
				errBuf << errFile.rdbuf();
				std::string body = errBuf.str();
				return "HTTP/1.1 404 Not Found\r\nContent-Length: " + toString(body.size()) +
					   "\r\nContent-Type: text/html\r\n\r\n" + body;
			}
		}
		// Fallback 404 response
		return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
	}

	RequestHandlerData data;
	data.FileName = srv.root + path;
	data.requestMethod = method;
	std::string returnData;

	setData(data, const_cast<ServerConfig&>(srv));
	data.FileContentType = getContentType(data.FileName);
		
	if (access(data.FileName.c_str(), R_OK | F_OK) != SUCCESS)
	{
		data.FileContentType = "html";
		errorHandling(data, "./www/error/404.html", "HTTP/1.1 404 Not Found");
	}
	else if (data.FileContentType == "php" && (method == "GET" || method == "POST"))
	{
		data.FileContentType = "html";
		if (handle_dynamic_request(data) != SUCCESS)
			errorHandling(data, "./www/error/500.html", "HTTP/1.1 500 Internal Server Error");
	}
	else if (method == "GET")
	{
		if (handle_static_request(data) != SUCCESS)
			std::cout << "File: " + data.FileName + " failed to be handled" << std::endl;
	}
	else if (method == "DELETE")
		errorHandling(data, "./www/error/404.html", "HTTP/1.1 404 Not Found");
	else
		errorHandling(data, "./www/error/405.html", "HTTP/1.1 405 Method Not Allowed");
		
	returnData =
		data.StatusLine
		+ "\r\nConnection: keep-alive"
		+ "\r\nLast-Modified: " + getFileDate(data.FileName)
		+ "\r\nDate: " + getDate()
		+ "\r\nContent-Length: " + toString(data.FileContent.size()) // Fixed typo: "Lenght" -> "Length"
		+ "\r\nContent-Type: text/" + data.FileContentType
		+ "\r\n\r\n" + data.FileContent;
		
	return returnData;
}
