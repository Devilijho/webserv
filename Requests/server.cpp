/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pde-vara <pde-vara@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 11:51:40 by pde-vara          #+#    #+#             */
/*   Updated: 2025/08/08 14:43:50 by pde-vara         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

Server::Server(int _port) : server_fd(-1), port(_port) {}

Server::~Server() {
	if (server_fd != -1)
		close(server_fd);
}


bool Server::start()
{
	if (!config.parseFile("default.conf") || !config.isValid())
    {
        std::cerr << "Invalid configuration. Stopping." << std::endl;
        return false;
    }
	
	if (!setupSocket())
		return false;

	// Agregar el socket del servidor a poll()
	struct pollfd server_pollfd;
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	poll_fds.push_back(server_pollfd);

	std::cout << "Servidor escuchando en puerto " << port << std::endl;

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
				if (poll_fds[i].fd == server_fd)
					acceptClient();
				else
				{
					handleClient(poll_fds[i].fd);
					close(poll_fds[i].fd);
					poll_fds.erase(poll_fds.begin() + i);
					--i;
				}
			}
		}
	}

	return true;
}

bool Server::setupSocket() {
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket");
		return false;
	}

	fcntl(server_fd, F_SETFL, O_NONBLOCK); // Socket no bloqueante

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return false;
	}

	if (listen(server_fd, 10) < 0) {
		perror("listen");
		return false;
	}

	return true;
}

void Server::acceptClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		perror("accept");
		return;
	}

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	struct pollfd client_pollfd;
	client_pollfd.fd = client_fd;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);
}

void Server::handleClient(int client_fd) {
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	if (bytes_read <= 0) {
		perror("read");
		return;
	}

	std::string raw_request(buffer);
	std::cout << "Pedido recibido:\n" << raw_request << std::endl;

	std::string response = buildHttpResponse(raw_request);

	send(client_fd, response.c_str(), response.length(), 0);
}

std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string Server::buildHttpResponse(const std::string &raw_request) {
    // --- Parse method and path from raw_request manually ---
    // Very simple HTTP request parser, improve as needed
    std::istringstream req_stream(raw_request);
    std::string method, path, protocol;
    req_stream >> method >> path >> protocol;
    if (method.empty() || path.empty()) {
        return "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
    }

    // 2. Match to a server config (simplified)
    const ServerConfig &srv = config.getServers()[0];

    // 3. Check if path matches a location (simplified)
    const LocationConfig *loc = srv.findLocation(path);
    if (!loc) {
        // Return 404 page
        std::ifstream errFile(srv.error_pages.at(404).c_str());
        std::stringstream errBuf;
        errBuf << errFile.rdbuf();
        std::string body = errBuf.str();
        return "HTTP/1.1 404 Not Found\r\nContent-Length: " + toString(body.size()) +
               "\r\nContent-Type: text/html\r\n\r\n" + body;
    }

    // 4. Prepare RequestHandlerData
    RequestHandlerData data;
    data.requestMethod = method;
    data.staticFileName = srv.root + path; // Example, adjust for your config

    // 5. Set environment and args for CGI / static
    setData(data, const_cast<ServerConfig&>(srv));  // may want to adjust setData to not hardcode paths

    int status = 0;
    std::string body;

    if (path.find(".php") != std::string::npos && (method == "GET" || method == "POST")) {
        status = handle_dynamic_request(data);
        if (status != 0) {
            body = "CGI Error";
            return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: " + toString(body.size()) +
                   "\r\nContent-Type: text/plain\r\n\r\n" + body;
        }
        // After fix: handle_dynamic_request should fill data.staticFileContent with CGI output
        body = data.staticFileContent;
        return "HTTP/1.1 200 OK\r\nContent-Length: " + toString(body.size()) +
               "\r\nContent-Type: text/html\r\n\r\n" + body;
    } else if (method == "GET") {
        status = handle_static_request(data);
        if (status != 0) {
            return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        }
        body = data.staticFileContent;
        return "HTTP/1.1 200 OK\r\nContent-Length: " + toString(body.size()) +
               "\r\nContent-Type: text/html\r\n\r\n" + body;
    } else if (method == "DELETE") {
        return "HTTP/1.1 501 Not Implemented\r\nContent-Length: 0\r\n\r\n";
    } else {
        return "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
    }
}