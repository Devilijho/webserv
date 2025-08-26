


#include "server.hpp"


std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

// bool isRequestComplete(const std::string& data) {
// 	size_t header_end = data.find("\r\n\r\n");
// 	if (header_end == std::string::npos)
// 		return false; // still waiting for headers

// 	// check for Content-Length
// 	size_t content_pos = data.find("Content-Length:");
// 	if (content_pos != std::string::npos) {
// 		int content_length = atoi(data.substr(content_pos + 15).c_str());
// 		size_t total_length = header_end + 4 + content_length;
// 		return data.size() >= total_length;
// 	}
// 	return true; // no body, headers are enough
// }

// std::string Server::extractOneRequest(std::string &buffer) {
// 	// Find end of headers
// 	size_t header_end = buffer.find("\r\n\r\n");
// 	if (header_end == std::string::npos) {
// 		return ""; // not complete
// 	}

// 	size_t request_end = header_end + 4;

// 	// Check for Content-Length (to know if there's a body)
// 	size_t content_pos = buffer.find("Content-Length:");
// 	if (content_pos != std::string::npos) {
// 		// Extract content length
// 		size_t line_end = buffer.find("\r\n", content_pos);
// 		std::string len_str = buffer.substr(content_pos + 15, line_end - (content_pos + 15));
// 		int content_length = atoi(len_str.c_str());

// 		request_end += content_length;
// 		if (buffer.size() < request_end) {
// 			return ""; // still incomplete body
// 		}
// 	}

// 	// Extract full request
// 	std::string full_request = buffer.substr(0, request_end);

// 	// Remove it from the buffer (leave remaining data for next request)
// 	buffer.erase(0, request_end);

// 	return full_request;
// }



bool Server::handleReadEvent(int client_fd)
{
	char buffer[4096];
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));

	if (bytes_read <= 0) {
		closeConnection(client_fd);
		perror("read");
		return false;
	}

	std::string raw_request(buffer);
	std::cout << "Pedido recibido:\n" << raw_request << std::endl;

	clientBuffers[client_fd].append(buffer, bytes_read);

	std::map<int, ServerConfig>::iterator config_it = client_to_server_config.find(client_fd);
	if (config_it == client_to_server_config.end()) {
		std::cerr << "No server config found for client fd " << client_fd << std::endl;
		closeConnection(client_fd);
		return false;
	}

	ServerConfig &serverConfig = config_it->second;
	std::string response = buildHttpResponse(raw_request, serverConfig);
	ssize_t sent = send(client_fd, response.c_str(), response.size(), 0);
	if (sent < 0) {
		perror("send");
		closeConnection(client_fd);
		return false;
	}
	closeConnection(client_fd);
	return true;
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
			std::map<int, std::string>::const_iterator error_it = srv.error_pages.find(404);
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
	data.rawRequest = raw_request;
	setData(data, const_cast<ServerConfig&>(srv));
	if (access(data.FileName.c_str(), R_OK | F_OK) != SUCCESS){
		errorHandling(data, srv, 404);
	}
	else if (data.FileContentType == "php" && (method == "GET" || method == "POST")){
		data.FileContentType = "html";
		if (handle_dynamic_request(data) != SUCCESS)
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
