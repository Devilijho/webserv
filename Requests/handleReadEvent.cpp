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
		closeConnection(client_fd);
		return false;
	}

	// Append the newly read data to the per-client buffer
	clientBuffers[client_fd].append(buffer, bytes_read);

	// Check if we have a complete HTTP request
	std::string& raw_request = clientBuffers[client_fd];
	size_t end_of_headers = raw_request.find("\r\n\r\n");
	if (end_of_headers == std::string::npos) {
		// Request not complete yet, wait for more data
		return true;
	}

	// Full request received, we can process it
	std::map<int, ServerConfig>::iterator config_it = client_to_server_config.find(client_fd);
	if (config_it == client_to_server_config.end()) {
		std::cerr << "No server config found for client fd " << client_fd << std::endl;
		closeConnection(client_fd);
		return false;
	}

	ServerConfig& serverConfig = config_it->second;
	std::string response = buildHttpResponse(raw_request, serverConfig);

	// Send the response
	ssize_t sent = send(client_fd, response.c_str(), response.size(), 0);
	if (sent < 0) {
		perror("send");
		closeConnection(client_fd);
		return false;
	}

	// Optionally, clear the buffer if the connection will be closed
	closeConnection(client_fd);
	clientBuffers.erase(client_fd);

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
