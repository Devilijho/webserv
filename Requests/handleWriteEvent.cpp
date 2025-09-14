#include "server.hpp"

bool Server::handleWriteEvent(int client_fd)
{
	RequestHandlerData* data = clientSockets[client_fd];
	if (!data || data->responseBuffer.empty())
		return true; // Nothing to write

	ssize_t bytes_to_send = data->responseBuffer.size() - data->bytesSent;
	ssize_t sent = send(client_fd, data->responseBuffer.c_str() + data->bytesSent, bytes_to_send, 0);

	if (sent > 0)
		data->bytesSent += sent;
	else if (sent == 0) // Unusual: peer closed connection while writing
	{
		std::cout << "[INFO] Client closed connection on fd " << client_fd << std::endl;
		closeConnection(client_fd);
		return false;
	}
	else  //Some error occurred
    {
        std::cout << "[ERROR] send() failed on fd " << client_fd << std::endl;
        closeConnection(client_fd);
        return false;
    }

	if (data->bytesSent >= data->responseBuffer.size())
	{
		data->responseBuffer.clear();
		data->bytesSent = 0;
		// Since we have nothing left to send; disable POLLOUT for this socket in poll
		for (size_t i = 0; i < poll_fds.size(); ++i) {
				if (poll_fds[i].fd == client_fd) {
						poll_fds[i].events &= ~POLLOUT;
						break;
				}
		}
		closeConnection(client_fd); // optional for HTTP/1.0
	}
	return true;
}

void Server::closeConnection(int client_fd)
{
	// Remove from poll_fds by marking fd as -1
	for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it) {
		if (it->fd == client_fd) {
			it->fd = -1; // Mark for cleanup instead of erasing immediately
			break;
		}
	}

	// 2. Clean up client data (delete before erasing pointer from map)
	std::map<int, RequestHandlerData*>::iterator it = clientSockets.find(client_fd);
	if (it != clientSockets.end()) {
		delete it->second;			  // free the allocated RequestHandlerData
		clientSockets.erase(it);		// remove from map
	}

	clientBuffers.erase(client_fd);
	client_to_server_config.erase(client_fd);

	// Close the socket
	close(client_fd);
	std::cout << "[INFO] Closed connection on fd " << client_fd << std::endl;
}
