#include "ConfigParser.hpp"

//============================================================================
// VALIDACIÓN COMPLETA
//============================================================================

bool ConfigParser::validateConfig() {
    if (_servers.empty()) {
        std::cerr << "No servers defined in configuration" << std::endl;
        return false;
    }

    for (size_t i = 0; i < _servers.size(); ++i) {
        if (!validateServer(i, _servers[i])) {
            return false;
        }
    }

    return validateDuplicateServers();
}

bool ConfigParser::validateServer(size_t serverIndex, const ServerConfig *srv) {
    return validateServerBasics(serverIndex, srv) &&
           validateServerFiles(serverIndex, srv) &&
           validateServerLocations(serverIndex, srv);
}

bool ConfigParser::validateServerBasics(size_t serverIndex, const ServerConfig *srv) {
    if (!isValidPort(srv->port)) {
        std::cerr << "Server " << serverIndex << ": Invalid port " << srv->port << std::endl;
        return false;
    }

    if (srv->host.empty() || !isValidHost(srv->host)) {
        std::cerr << "Server " << serverIndex << ": Invalid host format '" << srv->host << "'" << std::endl;
        return false;
    }

    if (srv->index.empty()) {
        std::cerr << "Server " << serverIndex << ": Index file cannot be empty" << std::endl;
        return false;
    }

    if (srv->client_max_body_size < 1024) {
        std::cerr << "Server " << serverIndex << ": client_max_body_size too small: "
                  << srv->client_max_body_size << " (minimum: 1024 bytes)" << std::endl;
        return false;
    }

    return true;
}

bool ConfigParser::validateServerFiles(size_t serverIndex, const ServerConfig *srv) {
    if (access(srv->root.c_str(), R_OK) != 0) {
        std::cerr << "Server " << serverIndex << ": Root directory not accessible: " << srv->root << std::endl;
        return false;
    }

    std::string fullIndexPath = srv->root + "/" + srv->index;
    if (access(fullIndexPath.c_str(), R_OK) != 0) {
        std::cerr << "Server " << serverIndex << ": Index file not found: " << fullIndexPath << std::endl;
        return false;
    }

    return true;
}

bool ConfigParser::validateServerLocations(size_t serverIndex, const ServerConfig *srv) {
    if (srv->locations.empty()) {
        std::cerr << "Server " << serverIndex << ": No locations defined" << std::endl;
        return false;
    }

    bool hasRootLocation = false;
    for (size_t j = 0; j < srv->locations.size(); ++j) {
        LocationConfig* loc = srv->locations[j]; // get pointer from vector

        if (loc->path == "/") {
            hasRootLocation = true;
        }

        if (!validateLocation(serverIndex, srv, j, *loc)) { // dereference pointer for reference
            return false;
        }
    }

    if (!hasRootLocation) {
        std::cerr << "Server " << serverIndex << ": Missing root location '/'" << std::endl;
        return false;
    }

    return true;
}

bool ConfigParser::validateLocation(size_t serverIndex, const ServerConfig* srv, size_t locationIndex, const LocationConfig& loc) {
    if (loc.path.empty() || loc.path[0] != '/') {
        std::cerr << "Server " << serverIndex << " Location " << locationIndex
                  << ": Invalid path '" << loc.path << "'" << std::endl;
        return false;
    }

    if (loc.methods.empty()) {
        std::cerr << "Server " << serverIndex << " Location '" << loc.path
                  << "': No methods defined" << std::endl;
        return false;
    }

    return validateLocationFiles(serverIndex, srv, loc);
}

bool ConfigParser::validateLocationFiles(size_t serverIndex, const ServerConfig *srv,
                                        const LocationConfig& loc) {
    std::string effectiveRoot = loc.root.empty() ? srv->root : loc.root;
    if (access(effectiveRoot.c_str(), R_OK) != 0) {
        std::cerr << "Server " << serverIndex << " Location '" << loc.path
                  << "': Root directory not accessible: " << effectiveRoot << std::endl;
        return false;
    }

    if (!loc.index.empty()) {
        std::string fullLocationIndexPath = effectiveRoot + "/" + loc.index;
        if (access(fullLocationIndexPath.c_str(), R_OK) != 0) {
            std::cerr << "Server " << serverIndex << " Location '" << loc.path
                      << "': Index file not found: " << fullLocationIndexPath << std::endl;
            return false;
        }
    }

    if (!loc.cgi_path.empty()) {
        if (access(loc.cgi_path.c_str(), X_OK) != 0) {
            std::cerr << "Server " << serverIndex << " Location '" << loc.path
                      << "': CGI interpreter not found: " << loc.cgi_path << std::endl;
            return false;
        }
        if (loc.cgi_extension.empty()) {
            std::cerr << "Server " << serverIndex << " Location '" << loc.path
                      << "': CGI path defined but no extension" << std::endl;
            return false;
        }
    }

    return true;
}

bool ConfigParser::validateDuplicateServers() {
    for (size_t i = 0; i < _servers.size(); ++i) {
        for (size_t j = i + 1; j < _servers.size(); ++j) {
            if (_servers[i]->port == _servers[j]->port && _servers[i]->host == _servers[j]->host) {
                std::cerr << "Duplicate server configuration: "
                          << _servers[i]->host << ":" << _servers[i]->port << std::endl;
                return false;
            }
        }
    }
    return true;
}
