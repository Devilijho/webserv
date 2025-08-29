#include "server.hpp"


std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool Server::hasCompleteRequest(int client_fd) {
	RequestHandlerData* data = clientSockets[client_fd];
	if (!data)
		return false;

	std::string& buffer = data->requestBuffer;

	// Look for end of headers
	size_t headers_end = buffer.find("\r\n\r\n");
	if (headers_end == std::string::npos)
		return false; // headers not complete yet

	// Check for Content-Length
	size_t content_length_pos = buffer.find("Content-Length:");
	if (content_length_pos == std::string::npos)
		return true; // no body, headers complete = full request

	// Extract content length value
	std::istringstream iss(buffer.substr(content_length_pos + 15));
	int content_length = 0;
	iss >> content_length;

	// Total request size = headers + 4 (\r\n\r\n) + content_length
	size_t total_size = headers_end + 4 + content_length;

	return buffer.size() >= total_size;
}


bool Server::handleReadEvent(int client_fd)
{
	std::cout << "handlereadevent !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	char buffer[4096];
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
		
	if (bytes_read <= 0) {
		if (bytes_read == 0)
			std::cout << "[INFO] Client disconnected on fd " << client_fd << std::endl;
		else
			perror("read");

		closeConnection(client_fd);
		return false;
	}

	// Append to per-client buffer
	clientSockets[client_fd]->requestBuffer.append(buffer, bytes_read);

	// Check if full request has been received
	if (!hasCompleteRequest(client_fd))
		return true; // wait for more data

	// Process request and prepare response
	std::string response = buildHttpResponse(clientSockets[client_fd]->requestBuffer,
											 client_to_server_config[client_fd]);

	RequestHandlerData* data = clientSockets[client_fd];
	data->responseBuffer = response;
	data->bytesSent = 0;

	// Enable POLLOUT for this fd so we can send the response
	for (size_t i = 0; i < poll_fds.size(); ++i) {
		if (poll_fds[i].fd == client_fd) {
			poll_fds[i].events |= POLLOUT;
			break;
		}
	}

	// Clear request buffer after processing
	data->requestBuffer.clear();
	return true;
}


std::string Server::buildHttpResponse(const std::string &raw_request, const ServerConfig* serverConfig)
{
	std::istringstream req_stream(raw_request);
	std::string method, path, protocol;
	req_stream >> method >> path >> protocol;

	const ServerConfig* srv = serverConfig;
	const LocationConfig *loc = srv->findLocation(path);

	RequestHandlerData data;
	data.FileName = srv->root + path;
	data.requestMethod = method;
	data.rawRequest = raw_request;

	setData(data, *srv, loc);

	if (access(data.FileName.c_str(), R_OK | F_OK) != SUCCESS){
		errorHandling(data, *srv, 404);
	}
	else if (data.FileContentType == "php" && (method == "GET" || method == "POST")){
		data.FileContentType = "html";
		if (handle_dynamic_request(data, loc->cgi_path.c_str()) != SUCCESS)
			errorHandling(data, *srv, 500);
	}
	else if (method == "GET"){
		if (handle_static_request(data, *srv) != SUCCESS)
			errorHandling(data, *srv, 500);
	}
	else if (method == "DELETE")
		handle_delete_request(data);
	else
		errorHandling(data, *srv, 405);

	return (http_response(data, const_cast<ServerConfig&>(*srv)));
}
