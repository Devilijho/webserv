#include "ConfigParser.hpp"

//============================================================================
// PARSING DE DIRECTIVAS SERVER
//============================================================================

bool ConfigParser::parseServerPortDirective(const std::vector<std::string>& tokens, ServerConfig& server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty listen directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    int port = atoi(value.c_str());
    if (port <= 0 || port > 65535) {
        std::cerr << "Error: Invalid port '" << value << "' (must be 1-65535)" << std::endl;
        return false;
    }

    server.port = port;
    return true;
}

bool ConfigParser::parseServerHostDirective(const std::vector<std::string>& tokens, ServerConfig& server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty host directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    if (value.empty()) {
        std::cerr << "Error: Host cannot be empty" << std::endl;
        return false;
    }

    if (!isValidHost(value)) {
        std::cerr << "Error: Invalid host format '" << value << "'" << std::endl;
        return false;
    }

    server.host = value;
    return true;
}

bool ConfigParser::parseServerRootDirective(const std::vector<std::string>& tokens, ServerConfig& server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty root directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    if (value.empty()) {
        std::cerr << "Error: Root directory cannot be empty" << std::endl;
        return false;
    }

    server.root = value;
    return true;
}

bool ConfigParser::parseServerIndexDirective(const std::vector<std::string>& tokens, ServerConfig& server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty index directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    if (value.empty()) {
        std::cerr << "Error: Index file cannot be empty" << std::endl;
        return false;
    }

    if (value.find('.') == std::string::npos) {
        std::cerr << "Error: Index file '" << value << "' must have extension" << std::endl;
        return false;
    }

    server.index = value;
    return true;
}

bool ConfigParser::parseServerNameDirective(const std::vector<std::string>& tokens, ServerConfig& server) {
    if (tokens.size() < 2) {
        std::cerr << "Warning: Empty server_name directive" << std::endl;
        return true;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    if (value.empty()) {
        std::cerr << "Warning: server_name is empty" << std::endl;
        return true;
    }

    server.server_name = value;
    return true;
}

bool ConfigParser::parseServerClientMaxBodySizeDirective(const std::vector<std::string>& tokens, ServerConfig& server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty client_max_body_size directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    if (value.empty()) {
        std::cerr << "Error: client_max_body_size cannot be empty" << std::endl;
        return false;
    }

    long size = atol(value.c_str());
    if (size < 1024) {
        std::cerr << "Error: client_max_body_size too small: " << size << " (minimum: 1024 bytes)" << std::endl;
        return false;
    }

    if (size > 1073741824) {
        std::cerr << "Error: client_max_body_size too large: " << size << " (maximum: 1GB)" << std::endl;
        return false;
    }

    server.client_max_body_size = static_cast<size_t>(size);
    return true;
}

bool ConfigParser::parseServerErrorPageDirective(const std::vector<std::string>& tokens, ServerConfig& server) {
    if (tokens.size() < 3) {
        std::cerr << "Error: error_page requires code and path" << std::endl;
        return false;
    }

    std::string codeStr = tokens[1];
    std::string path = tokens[2];

    if (!path.empty() && path[path.length() - 1] == ';') {
        path = path.substr(0, path.length() - 1);
    }

    int errorCode = atoi(codeStr.c_str());
    if (errorCode < 400 || errorCode > 599) {
        std::cerr << "Error: Invalid HTTP error code: " << errorCode << std::endl;
        return false;
    }

    if (path.empty()) {
        std::cerr << "Error: Error page path cannot be empty" << std::endl;
        return false;
    }

    server.error_pages[errorCode] = path;
    return true;
}
