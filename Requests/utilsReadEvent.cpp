
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
    RequestHandlerData* data = clientSockets[client_fd];
    if (!data) return false;

    std::string &buf = data->requestBuffer;

    // 1. Fin de headers correcto
    size_t hdr_end = buf.find("\r\n\r\n");
    if (hdr_end == std::string::npos)
        return false;

    // 2. Primera línea
    size_t line_end = buf.find("\r\n");
    if (line_end == std::string::npos)
        return false;

    std::string request_line = buf.substr(0, line_end);
    std::istringstream rl(request_line);
    std::string method, path, proto;
    if (!(rl >> method >> path >> proto)) {
        data->statusCode = 413;
        return true;
    }

    // 3. Localización y límite
    ServerConfig *srv = client_to_server_config[client_fd];
    const LocationConfig *loc = (srv ? srv->findLocation(path) : NULL);

    size_t max_size = (srv ? srv->client_max_body_size : 0);
    if (loc && loc->client_max_body_size > 0)
        max_size = loc->client_max_body_size;

    // 4. Content-Length (si existe)
    size_t content_length = 0;
    size_t cl_pos = buf.find("Content-Length:");
    if (cl_pos != std::string::npos && cl_pos < hdr_end) {
        size_t cl_line_end = buf.find("\r\n", cl_pos);
        if (cl_line_end == std::string::npos || cl_line_end > hdr_end)
            cl_line_end = hdr_end;
        std::string cl_line = buf.substr(cl_pos + 15, cl_line_end - (cl_pos + 15));
        // trim
        size_t a = cl_line.find_first_not_of(" \t");
        if (a != std::string::npos) {
            size_t b = cl_line.find_last_not_of(" \t");
            cl_line = cl_line.substr(a, b - a + 1);
            std::istringstream iss(cl_line);
            long long tmp;
            if (iss >> tmp && tmp >= 0)
                content_length = static_cast<size_t>(tmp);
            else {
                data->statusCode = 413;
                return true;
            }
        }
    } else {
        // No hay body -> request completa
        return true;
    }

    // 5. Chequeo de límite (solo body)
    if (max_size > 0 && content_length > max_size) {
        data->statusCode = 413;
        return true;
    }

    // 6. ¿Ya recibimos todo?
    size_t total_needed = hdr_end + 4 + content_length;
    return buf.size() >= total_needed;
}

