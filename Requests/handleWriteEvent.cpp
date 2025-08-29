#include "server.hpp"

bool Server::handleWriteEvent(int client_fd) {
	RequestHandlerData* data = clientSockets[client_fd];
	if (!data || data->responseBuffer.empty()) {
		// Nothing to write
		return true;
	}

	ssize_t bytes_to_send = data->responseBuffer.size() - data->bytesSent;
	ssize_t sent = send(client_fd, data->responseBuffer.c_str() + data->bytesSent, bytes_to_send, 0);

	if (sent < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("send");
			closeConnection(client_fd);
			return false;
		}
		// Socket not ready, try again later
		return true;
	}

	data->bytesSent += sent;

	// If fully sent, we can close the connection (HTTP/1.0 style)
	if (data->bytesSent >= data->responseBuffer.size()) {
		closeConnection(client_fd);
		return false;
	}

	return true;
}
