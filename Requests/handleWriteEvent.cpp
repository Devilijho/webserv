


#include "server.hpp"

void Server::handleWriteEvent(int client_fd)
{
    // Since handleReadEvent already sends responses immediately,
    // handleWriteEvent is mainly for cleanup or error handling
    
    // Find the client's data (using pointers as per your header)
    std::map<int, RequestHandlerData*>::iterator client_it = clientSockets.find(client_fd);
    if (client_it == clientSockets.end()) {
        // Client not found, close connection
        closeConnection(client_fd);
        return;
    }
    
    // Remove POLLOUT from this fd's events since we don't use buffered sending
    for (size_t i = 0; i < poll_fds.size(); ++i) {
        if (poll_fds[i].fd == client_fd) {
            poll_fds[i].events &= ~POLLOUT; // Remove POLLOUT flag
            poll_fds[i].events |= POLLIN;   // Keep POLLIN for new requests
            break;
        }
    }
    
    // Since we send responses immediately in handleReadEvent,
    // if we get here it's usually because the socket is ready for writing
    // but we have nothing to write, so we can close the connection
    closeConnection(client_fd);
}