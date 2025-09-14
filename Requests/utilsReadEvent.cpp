
#include "server.hpp"


bool Server::isBodyComplete(const std::string &buffer, size_t headers_end)
{
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

bool Server::hasCompleteRequest(int client_fd)
{
	std::string method, path, protocol;
	size_t max_size;
	const LocationConfig *loc;
	RequestHandlerData* data = clientSockets[client_fd];
	if (!data)
		return false;
	std::string& buffer = data->requestBuffer;

	// Look for end of headers
	size_t headers_end = buffer.find("\r\n");
	std::string header = buffer.substr(0, headers_end);
	std::istringstream req_stream(header);
	req_stream >> method >> path >> protocol;
	if (client_to_server_config[client_fd] != NULL)
		loc = client_to_server_config[client_fd]->findLocation(path);
	else
		loc = NULL;
	if (loc != NULL)
		max_size = loc->client_max_body_size;
	else
		max_size = client_to_server_config[client_fd]->client_max_body_size;
	// std::cout << data->requestBuffer.size() << "---" << max_size << std::endl;
	if (data->requestBuffer.size() > max_size)
	{
		data->requestBuffer = header;
		this->error413 = ERROR;
		return true;
	}
	if (headers_end == std::string::npos)
		return false; // headers not complete yet

	return isBodyComplete(buffer, headers_end);
}

