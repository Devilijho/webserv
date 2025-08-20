#include "server.hpp"

Server::Server(int _port) : server_fd(-1), port(_port) {}

Server::~Server() {
	if (server_fd != -1)
		close(server_fd);
}


bool Server::start()
{
	if (!loadConfig("default.conf"))
		return false;

	if (!setupSocket())
		return false;

	addServerSocketToPoll();

	std::cout << "Servidor escuchando en puerto " << port << std::endl;

	eventLoop();

	return true;
}

bool Server::loadConfig(const std::string& configFile) //check les methodes de Config
{
	if (!config.parseFile(configFile) || !config.isValid())
	{
		std::cerr << "Invalid configuration. Stopping." << std::endl;
		return false;
	}
	return true;
}

bool Server::setupSocket()
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
		return perror("socket"), false;

	fcntl(server_fd, F_SETFL, O_NONBLOCK); // Socket no bloqueante

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port); // host to network short

	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		return perror("bind"), false;
	if (listen(server_fd, 10) < 0)
		return perror("listen"), false;
	return true;
}

void Server::addServerSocketToPoll()
{
	struct pollfd server_pollfd;
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	poll_fds.push_back(server_pollfd);
}


void Server::handleClientConnection(size_t index)
{
	int client_fd = poll_fds[index].fd;
	handleClient(client_fd);
	close(client_fd);
	poll_fds.erase(poll_fds.begin() + index);
}

void Server::eventLoop()
{
	while (true)
	{
		int ret = poll(&poll_fds[0], poll_fds.size(), -1);
		if (ret < 0)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			if (poll_fds[i].revents & POLLIN)
			{
				if (poll_fds[i].fd == server_fd)
					acceptClient();
				else
					handleClientConnection(i);
			}
		}
	}
}

void Server::acceptClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0) {
		perror("accept");
		return;
	}
	fcntl(client_fd, F_SETFL, O_NONBLOCK);
	struct pollfd client_pollfd;
	client_pollfd.fd = client_fd;
	client_pollfd.events = POLLIN;
	poll_fds.push_back(client_pollfd);
}

void Server::handleClient(int client_fd) {
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	if (bytes_read <= 0) {
		perror("read");
		return;
	}

	std::string raw_request(buffer);
	std::cout << "Pedido recibido:\n" << raw_request << std::endl;


	std::string response = buildHttpResponse(raw_request);

	send(client_fd, response.c_str(), response.length(), 0);
}

std::string Server::toString(int value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string Server::buildHttpResponse(const std::string &raw_request)
{
	std::istringstream req_stream(raw_request);
	std::string method, path, protocol;
	req_stream >> method >> path >> protocol;
	if (method.empty() || path.empty())
		return "HTTP/1.1 400 Bad Request\r\n\r\n";

	// Find matching server + location (simplified: take first server)
	const ServerConfig &srv = config.getServers()[0];
	const LocationConfig *loc = srv.findLocation(path);
	(void)loc;

	RequestHandlerData data;

	data.FileName = srv.root + path;
	data.requestMethod = method;
	data.rawRequest = raw_request;
	data.FileName = srv.root + path;
	setData(data, const_cast<ServerConfig&>(srv));

	if (access(data.FileName.c_str(), R_OK | F_OK) != SUCCESS)
	{
		data.FileContentType = "html";
		errorHandling(data, srv, 404);
	}
	else if (data.FileContentType == "php" && (method == "GET" || method == "POST")) {
		data.FileContentType = "html";
		if (handle_dynamic_request(data) != SUCCESS)
			errorHandling(data, srv, 500);
	}
	else if (method == "GET") {
		if (handle_static_request(data) != SUCCESS)
			errorHandling(data, srv, 500);
	}
	else if (method == "DELETE")
		errorHandling(data, srv, 404);
	else
		errorHandling(data, srv, 405);
	return (http_response(data, const_cast<ServerConfig&>(srv)));
}

/*
std::string Server::buildResponseString(const RequestHandlerData &data)
{
	std::string returnData =
		data.StatusLine
		+ "\r\nConnection: keep-alive"
		+ "\r\nLast-Modified: " + getFileDate(data.FileName)
		+ "\r\nDate: " + getDate()
		+ "\r\nContent-Length: " + toString(data.FileContent.size())
		+ "\r\nContent-Type: text/" + data.FileContentType
		+ "\r\n\r\n" + data.FileContent;

	return returnData;
}

std::string Server::getStatusMessage(int code)
{
	switch (code) {
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		default:  return "Error";
	}
}

void Server::serveError(RequestHandlerData &data, const ServerConfig &srv, int code)
{
	std::map<int, std::string>::const_iterator it = srv.error_pages.find(code);
	std::string filePath;

	if (it != srv.error_pages.end())
		filePath = it->second;
	else
		filePath = "./errors/default.html";

	std::ifstream errFile(filePath.c_str());
	std::stringstream buf;
	buf << errFile.rdbuf();

	data.FileContent = buf.str();
	data.FileContentType = "html";
	data.StatusLine = "HTTP/1.1 " + toString(code) + " " + getStatusMessage(code);
}
 */


bool Server::isMethodAllowed(const LocationConfig &loc, const std::string &method)
{
	const std::vector<std::string> &allowed = loc.methods;
	return std::find(allowed.begin(), allowed.end(), method) != allowed.end();
}

bool Server::isBodySizeAllowed(const std::string &raw_request, size_t max_size)
{
	std::istringstream req_stream(raw_request);
	std::string line;
	while (std::getline(req_stream, line) && line != "\r") {
		if (line.find("Content-Length:") == 0) {
			size_t length = std::atoi(line.substr(15).c_str());
			return length <= max_size;
		}
	}
	return true;
}
