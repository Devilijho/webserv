


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
	std::string raw_request = read_all(client_fd);
	if (raw_request == "")
	{
		closeConnection(client_fd);
		return false;
	}

	std::cout << "Pedido recibido:\n" << raw_request << std::endl;
	std::map<int, ServerConfig>::iterator config_it = client_to_server_config.find(client_fd);
	if (config_it == client_to_server_config.end()) {
		std::cerr << "No server config found for client fd " << client_fd << std::endl;
		closeConnection(client_fd);
		return false;
	}

	ServerConfig &serverConfig = config_it->second;
	std::string response = buildHttpResponse(raw_request, serverConfig);
	if (send_all(client_fd, response.c_str(), response.size(), 0) == -1)
		return false;
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
