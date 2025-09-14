#include "server.hpp"


std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool Server::isMethodAllowed(const LocationConfig* loc, const std::string& method) const
{
    // Si no hay location coincidente, permitir un set básico
    if (!loc)
        return method == "GET" || method == "POST" || method == "DELETE";
    if (!loc->methods.empty()) {
        for (size_t i = 0; i < loc->methods.size(); ++i)
            if (loc->methods[i] == method)
                return true;
        return false;
    }
    return method == "GET" || method == "POST" || method == "DELETE";
}

std::string Server::getFullPath(const LocationConfig* loc, const ServerConfig* srv, const std::string& path) const
{
    if (!srv) return path;
    std::string root = srv->root;
    if (loc && !loc->root.empty())
        root = loc->root;
    if (!root.empty() && root[root.length()-1] == '/')
        root.erase(root.length()-1);
    std::string rel = path;
    if (rel.empty()) rel = "/";
    if (!rel.empty() && rel[0] == '/')
        return root + rel;
    return root + "/" + rel;
}


bool Server::handleReadEvent(int client_fd)
{
    char tmp[4096];
    ssize_t n = read(client_fd, tmp, sizeof(tmp));
    if (n <= 0) {
        if (n == 0)
            std::cout << "[INFO] Client disconnected on fd " << client_fd << std::endl;
        else
            perror("read");
        closeConnection(client_fd);
        return false;
    }

    RequestHandlerData *d = clientSockets[client_fd];
    d->requestBuffer.append(tmp, n);

    if (!hasCompleteRequest(client_fd))
        return true;

    ServerConfig *srv = client_to_server_config[client_fd];

    // Error temprano (413 / 400 etc)
    if (d->statusCode == 413) {
        errorHandling(*d, srv, d->statusCode);
        d->responseBuffer = http_response(*d, *srv);
        d->bytesSent = 0;
        for (size_t i=0;i<poll_fds.size();++i)
            if (poll_fds[i].fd == client_fd)
                poll_fds[i].events |= POLLOUT;
        d->requestBuffer.clear();
        return true;
    }

    std::cout << "DONE READING" << std::endl;
    std::string response = buildHttpResponse(d->requestBuffer, srv);
    d->responseBuffer = response;
    d->bytesSent = 0;

    for (size_t i = 0; i < poll_fds.size(); ++i) {
        if (poll_fds[i].fd == client_fd) {
            poll_fds[i].events |= POLLOUT;
            break;
        }
    }
    d->requestBuffer.clear();
    return true;
}

void Server::handleResource(RequestHandlerData& data, const LocationConfig* loc,
                            const ServerConfig* srv, const std::string& method)
{
    // Redirect
    if (loc && loc->has_return) {
        data.is_redirect = true;
        data.statusCode = loc->return_code;
        data.redirect_location = loc->return_url;
        data.FileContent.clear();
        return;
    }

    bool allowed = isMethodAllowed(loc, method);
    // Path puede no tener location válida
    if (getFileType(data.FileName) == DIRECTORY) {
        std::string indexFile = data.FileName;
        if (!indexFile.empty() && indexFile[indexFile.length()-1] != '/')
            indexFile += "/";
        std::string indexName = (loc && !loc->index.empty()) ? loc->index : srv->index;
        indexFile += indexName;

        if (access(indexFile.c_str(), R_OK | F_OK) == SUCCESS && allowed && method == "GET") {
            data.FileName = indexFile;
            if (handle_static_request(data) != SUCCESS)
                errorHandling(data, srv, 500);
        }
        else if (loc && loc->autoindex && allowed && method == "GET") {
            setCurrentDirFiles(data, *srv, loc);
        }
        else {
            errorHandling(data, srv, 403);
        }
        return;
    }
    handleFileRequest(data, loc, srv, method, allowed);
}

void Server::handleFileRequest(RequestHandlerData& data, const LocationConfig* loc,
                               const ServerConfig* srv, const std::string& method, bool allowed)
{
    if (!allowed) {
        errorHandling(data, srv, 405);
        return;
    }
    // CGI
    if (loc && ("." + data.FileContentType) == loc->cgi_extension
        && (method == "GET" || method == "POST")) {
        data.FileContentType = "html";
        if (handle_dynamic_request(data, loc->cgi_path.c_str(), this) != SUCCESS)
            errorHandling(data, srv, 500);
        return;
    }
    if (method == "GET") {
        if (handle_static_request(data) != SUCCESS)
            errorHandling(data, srv, 500);
    }
    else if (method == "DELETE") {
        handle_delete_request(data);
    }
    else {
        errorHandling(data, srv, 405);
    }
}

std::string Server::buildHttpResponse(const std::string &raw, const ServerConfig* srv)
{
    size_t line_end = raw.find("\r\n");
    if (line_end == std::string::npos) {
        RequestHandlerData d; d.statusCode = 413;
        return http_response(d, *const_cast<ServerConfig*>(srv));
    }
    std::string line = raw.substr(0, line_end);
    std::istringstream iss(line);
    std::string method, path, proto;
    if (!(iss >> method >> path >> proto)) {
        RequestHandlerData d; d.statusCode = 413;
        return http_response(d, *const_cast<ServerConfig*>(srv));
    }
    const LocationConfig *loc = srv ? srv->findLocation(path) : NULL;

    RequestHandlerData data;
    data.path = path;
    data.requestMethod = method;
    data.FileName = getFullPath(loc, srv, path);
    data.rawRequest = raw;
    data.statusCode = 200;
    data.is_redirect = false;
    data.redirect_location.clear();

    setData(data, *srv, loc);

    if (!isMethodAllowed(loc, method)) {
        errorHandling(data, srv, 405);
        return http_response(data, *const_cast<ServerConfig*>(srv));
    }
    if (loc && loc->has_return) {
        data.is_redirect = true;
        data.statusCode = loc->return_code;
        data.redirect_location = loc->return_url;
        data.FileContent.clear();
        return http_response(data, *const_cast<ServerConfig*>(srv));
    }
    if (access(data.FileName.c_str(), R_OK | F_OK) != SUCCESS) {
        errorHandling(data, srv, 404);
        return http_response(data, *const_cast<ServerConfig*>(srv));
    }
    handleResource(data, loc, srv, method);
    return http_response(data, *const_cast<ServerConfig*>(srv));
}
