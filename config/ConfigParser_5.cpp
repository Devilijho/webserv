#include "ConfigParser.hpp"

//============================================================================
// FUNCIONES UTILITARIAS Y VALIDACIONES
//============================================================================

const std::vector<ServerConfig> &ConfigParser::getServers() const {
    return _servers;
}

bool ConfigParser::isValid() const {
    return _is_parsed && !_servers.empty();
}

void ConfigParser::printConfig() const {
    std::cout << "\n=== WEBSERV CONFIGURATION ===" << std::endl;
    for (size_t i = 0; i < _servers.size(); ++i) {
        const ServerConfig &srv = _servers[i];
        std::cout << "Server " << i << ":" << std::endl;
        std::cout << "  Listen: " << srv.host << ":" << srv.port << std::endl;
        std::cout << "  Root: " << srv.root << std::endl;
        std::cout << "  Index: " << srv.index << std::endl;
        std::cout << "  Locations: " << srv.locations.size() << std::endl;
    }
    std::cout << "================================\n" << std::endl;
}

std::string ConfigParser::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> ConfigParser::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == delimiter) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += str[i];
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string ConfigParser::removeInlineComment(const std::string& str) {
    size_t commentPos = str.find('#');
    if (commentPos != std::string::npos) {
        return trim(str.substr(0, commentPos));
    }
    return str;
}

bool ConfigParser::isValidMethod(const std::string& method) {
    return (method == "GET" || method == "POST" || method == "DELETE");
}

bool ConfigParser::isValidPort(int port) {
    return (port > 0 && port <= 65535);
}

bool ConfigParser::isValidIPAddress(const std::string& ip) {
    if (ip.empty()) return false;

    if (ip == "0.0.0.0" || ip == "localhost" || ip == "127.0.0.1") {
        return true;
    }

    std::string temp;
    std::istringstream iss(ip);
    int count = 0;

    while (std::getline(iss, temp, '.')) {
        count++;
        if (count > 4 || temp.empty()) return false;

        for (size_t j = 0; j < temp.size(); j++) {
            if (!isdigit(temp[j])) return false;
        }

        int value = atoi(temp.c_str());
        if (value < 0 || value > 255) return false;
    }

    return (count == 4);
}

bool ConfigParser::isValidHost(const std::string& host) {
    return isValidIPAddress(host);
}



