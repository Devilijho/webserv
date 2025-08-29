


#include "server.hpp"

bool Server::handleWriteEvent(int client_fd)
{
	RequestHandlerData* data = clientSockets[client_fd];
	if (!data || data->responseBuffer.empty()) {
		// Nothing to write
		return true;
	}

	ssize_t bytes_to_send = data->responseBuffer.size() - data->bytesSent;
	ssize_t sent = send(client_fd, data->responseBuffer.c_str() + data->bytesSent, bytes_to_send, 0);

	if (sent > 0)
		data->bytesSent += sent;
	else // Either socket temporarily not writable or connection closed
		return true; // just wait for the next POLLOUT event

	// data->bytesSent += sent;

	// If fully sent, we can close the connection (HTTP/1.0 style)
	if (data->bytesSent >= data->responseBuffer.size())
	{
		data->responseBuffer.clear();
		data->bytesSent = 0;
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

