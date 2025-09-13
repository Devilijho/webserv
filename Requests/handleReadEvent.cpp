#include "server.hpp"
#include <sys/poll.h>
#include <unistd.h>
#include <vector>


std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool Server::hasCompleteRequest(int client_fd)
{
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



bool Server::isMethodAllowed(const LocationConfig* loc, const std::string& method) const
{
	if (!loc->methods.empty()) {
		for (size_t i = 0; i < loc->methods.size(); ++i)
			if (loc->methods[i] == method)
				return true;
		return false;
	}
	// Si no hay métodos definidos, permite todos (nginx-like)
	return method == "GET" || method == "POST" || method == "DELETE";
}

std::string Server::getFullPath(const LocationConfig* loc, const ServerConfig* srv, const std::string& path) const
{
	std::string root = loc->root.empty() ? srv->root : loc->root;
	if (!root.empty() && root[root.length()-1] == '/')
		root = root.substr(0, root.length()-1);
	std::string fullPath = root;
	if (path[0] == '/')
		fullPath += path;
	else
		fullPath += "/" + path;
	return fullPath;
}

void Server::handleResource(RequestHandlerData& data, const LocationConfig* loc, const ServerConfig* srv, const std::string& method)
{
	if (loc->has_return) {
		data.is_redirect = true;
		data.statusCode = loc->return_code;
		data.redirect_location = loc->return_url;
		data.FileContent = "";  // No content para redirects
		return;
	}

	bool isAllowed = isMethodAllowed(loc, method);
	if (getFileType(data.FileName) == DIRECTORY) {
		std::string indexFile = data.FileName;
		if (!indexFile.empty() && indexFile[indexFile.length()-1] != '/')
			indexFile += "/";
		std::string indexName = loc->index.empty() ? srv->index : loc->index;
		indexFile += indexName;

		if (access(indexFile.c_str(), R_OK | F_OK) == SUCCESS && isAllowed && method == "GET") {
			data.FileName = indexFile;
			if (handle_static_request(data) != SUCCESS)
				errorHandling(data, srv, 500);
		}
		else if (loc->autoindex == true && isAllowed && method == "GET") {
			setCurrentDirFiles(data, *srv, loc);
		}
		else {
			errorHandling(data, srv, 403);
		}
	} else {
		handleFileRequest(data, loc, srv, method, isAllowed);
	}
}

void Server::handleFileRequest(RequestHandlerData& data, const LocationConfig* loc, const ServerConfig* srv, const std::string& method, bool isAllowed)
{
	if (!isAllowed) {
	errorHandling(data, srv, 405);  // ✅ 405, no 403
	return;
	}

	if (("." + data.FileContentType) == loc->cgi_extension && (method == "GET" || method == "POST") && isAllowed) {
		data.FileContentType = "html";
		if (handle_dynamic_request(data, loc->cgi_path.c_str(), this) != SUCCESS)
			errorHandling(data, srv, 500);
	}
	else if (method == "GET") {
		if (isAllowed) {
			if (handle_static_request(data) != SUCCESS)
				errorHandling(data, srv, 500);
		} else {
			errorHandling(data, srv, 405);
		}
	}
	else if (method == "DELETE") {
		if (isAllowed)
			handle_delete_request(data);
		else
			errorHandling(data, srv, 405);
	}
	else {
		errorHandling(data, srv, 405);
	}
}

std::string Server::buildHttpResponse(const std::string &raw_request, const ServerConfig* serverConfig)
{
	std::istringstream req_stream(raw_request);
	std::string method, path, protocol;
	req_stream >> method >> path >> protocol;

	const ServerConfig* srv = serverConfig;
	const LocationConfig *loc = srv->findLocation(path);

	RequestHandlerData data;
	data.path = path;
	data.FileName = getFullPath(loc, srv, path);
	data.requestMethod = method;
	data.rawRequest = raw_request;

	// ✅ INICIALIZAR CAMPOS DE REDIRECT
	data.is_redirect = false;
	data.statusCode = 200;
	data.redirect_location = "";

	setData(data, *srv, loc);

	if (!isMethodAllowed(loc, method)) {
    errorHandling(data, srv, 405);  // ✅ 405 Method Not Allowed
    return (http_response(data, const_cast<ServerConfig&>(*srv)));
    }

	// ✅ VERIFICAR REDIRECT ANTES DE CHECK FILE ACCESS
	if (loc->has_return) {
		data.is_redirect = true;
		data.statusCode = loc->return_code;
		data.redirect_location = loc->return_url;
		data.FileContent = "";
	}
	else if (access(data.FileName.c_str(), R_OK | F_OK) != SUCCESS) {
		errorHandling(data, srv, 404);
	}
	else {
		handleResource(data, loc, srv, method);
	}

	return (http_response(data, const_cast<ServerConfig&>(*srv)));
}
