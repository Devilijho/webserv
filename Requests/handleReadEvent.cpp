


#include "server.hpp"


std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool Server::handleReadEvent(int client_fd)
{
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	if (bytes_read <= 0) {
		closeConnection(client_fd);
		perror("read");
		return false;
	}

	std::string raw_request(buffer);
	std::cout << "Pedido recibido:\n" << raw_request << std::endl;

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
	
	std::ostringstream oss_len;
	oss_len << data.FileContent.size();
	std::string len_str = oss_len.str();

	returnData =
		data.StatusLine
		+ "\r\nConnection: keep-alive"
		+ "\r\nLast-Modified: " + getFileDate(data.FileName)
		+ "\r\nDate: " + getDate()
		+ "\r\nContent-Length: " + toString(data.FileContent.size())
		+ "\r\nContent-Type: text/" + data.FileContentType
		+ "\r\n\r\n" + data.FileContent;
		
	return returnData;
}
