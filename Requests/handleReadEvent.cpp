


#include "server.hpp"


std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool Server::handleReadEvent(int client_fd)
{
    char buffer[4096];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
    
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            std::cout << "[INFO] Client disconnected on fd " << client_fd << std::endl;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read");
        }
        closeConnection(client_fd);
        return false;
    }
    
    // Accumulate request data in clientBuffers
    clientBuffers[client_fd].append(buffer, bytes_read);
    
    // Check if we have a complete HTTP request (headers end with \r\n\r\n)
    std::string& raw_request = clientBuffers[client_fd];
    size_t end_of_headers = raw_request.find("\r\n\r\n");
    
    if (end_of_headers == std::string::npos) {
        // Request not complete yet, continue reading
        return true;
    }
    
    // Find the server config for this client
    std::map<int, ServerConfig>::iterator config_it = client_to_server_config.find(client_fd);
    if (config_it == client_to_server_config.end()) {
        std::cerr << "[ERROR] No server config found for client fd " << client_fd << std::endl;
        closeConnection(client_fd);
        return false;
    }
    
    // Build HTTP response using the existing method
    std::string response = buildHttpResponse(raw_request, config_it->second);
    
    if (response.empty()) {
        std::cerr << "[ERROR] Failed to build HTTP response for client fd " << client_fd << std::endl;
        closeConnection(client_fd);
        return false;
    }
    
    // Send the response immediately
    ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
    if (bytes_sent < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("send");
        }
        closeConnection(client_fd);
        return false;
    }
    
    // Close connection after sending response (HTTP/1.0 style)
    closeConnection(client_fd);
    return true;
}


std::string Server::buildHttpResponse(const std::string &raw_request, const ServerConfig& serverConfig)
{
	std::istringstream req_stream(raw_request);
	std::string method, path, protocol;
	req_stream >> method >> path >> protocol;

	const ServerConfig &srv = serverConfig;
	const LocationConfig *loc = srv.findLocation(path);
	RequestHandlerData data;

	data.FileName = srv.root + path;
	data.requestMethod = method;
	data.rawRequest = raw_request;
	setData(data, srv, loc);
	if (access(data.FileName.c_str(), R_OK | F_OK) != SUCCESS){
		errorHandling(data, srv, 404);
	}
	else if (data.FileContentType == "php" && (method == "GET" || method == "POST")){
		data.FileContentType = "html";
		if (handle_dynamic_request(data, loc->cgi_path.c_str()) != SUCCESS)
			errorHandling(data, srv, 500);
	}
	else if (method == "GET"){
		if (handle_static_request(data) != SUCCESS)
			errorHandling(data, srv, 500);
	}
	else if (method == "DELETE")
		handle_delete_request(data);
	else
		errorHandling(data, srv, 405);
	return (http_response(data, const_cast<ServerConfig&>(srv)));
}
