#include "ConfigParser.hpp"

//============================================================================
// CONSTRUCTOR Y DESTRUCTOR
//============================================================================

ConfigParser::ConfigParser() : _is_parsed(false) {}

ConfigParser::~ConfigParser() {}

//============================================================================
// FUNCIONES PRINCIPALES DE PARSING
//============================================================================

bool ConfigParser::parseFile(const std::string &filename) {
    std::cout << "ConfigParser: Loading " << filename << std::endl;

    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << filename << std::endl;
        return false;
    }

    _servers.clear();
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line.find("server") == 0 && line.find("{") != std::string::npos) {
            ServerConfig server;
            if (parseServerBlock(file, server)) {
                _servers.push_back(server);
            }
        }
    }

    file.close();
    return finalizeConfig();
}

bool ConfigParser::finalizeConfig() {
    // ✅ 1. PRIMERO APLICAR HERENCIA SIN FILTRAR
    for (size_t i = 0; i < _servers.size(); ++i) {
        ServerConfig& server = _servers[i];

        // Aplicar herencia de valores del servidor a las locations
        for (size_t j = 0; j < server.locations.size(); ++j) {
            LocationConfig& loc = server.locations[j];

            if (loc.root.empty()) {
                loc.root = server.root;
            }
            if (loc.index.empty()) {
                loc.index = server.index;
            }
            if (loc.client_max_body_size == 0) {
                loc.client_max_body_size = server.client_max_body_size;
            }
        }
    }

    // ✅ 2. VALIDAR TODO ANTES DE FILTRAR
    if (!validateConfig()) {  // ← AQUÍ SE LLAMA validateDuplicateServers()
        return false;
    }

    // ✅ 3. DESPUÉS FILTRAR SOLO LOS VÁLIDOS (SI QUIERES)
    std::vector<ServerConfig> validServers;
    for (size_t i = 0; i < _servers.size(); ++i) {
        if (validateServer(i, _servers[i])) {
            validServers.push_back(_servers[i]);
        }
    }

    _servers = validServers;

    if (_servers.empty()) {
        std::cerr << "Error: No valid server configurations found" << std::endl;
        return false;
    }

    _is_parsed = true;
    std::cout << "ConfigParser: Successfully loaded " << _servers.size() << " server(s)" << std::endl;
    return true;
}

//============================================================================
// PARSING DE BLOQUES PRINCIPALES
//============================================================================

bool ConfigParser::parseServerBlock(std::ifstream &file, ServerConfig &server) {
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        line = removeInlineComment(line);
        if (line.empty()) continue;

        if (line == "}") return true;

        if (line.find("location") == 0) {
            LocationConfig location;
            if (parseLocationHeader(line, location) && parseLocationBlock(file, location)) {
                server.locations.push_back(location);
            }
            continue;
        }

        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.size() < 2) continue;

        if (!parseServerDirective(tokens, server)) {
            return false;
        }
    }

    return false; // Missing closing brace
}

bool ConfigParser::parseLocationBlock(std::ifstream &file, LocationConfig &location) {
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        line = removeInlineComment(line);
        if (line.empty()) continue;

        if (line == "}") return true;

        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.size() < 2) continue;

        if (!parseLocationDirective(tokens, location)) {
            return false;
        }
    }

    return false; // Missing closing brace
}

//============================================================================
// DISPATCHERS DE DIRECTIVAS
//============================================================================

bool ConfigParser::parseServerDirective(const std::vector<std::string>& tokens, ServerConfig& server) {
    const std::string& directive = tokens[0];

    if (directive == "listen") {
        return parseServerPortDirective(tokens, server);
    } else if (directive == "host") {
        return parseServerHostDirective(tokens, server);
    } else if (directive == "root") {
        return parseServerRootDirective(tokens, server);
    } else if (directive == "index") {
        return parseServerIndexDirective(tokens, server);
    } else if (directive == "server_name") {
        return parseServerNameDirective(tokens, server);
    } else if (directive == "client_max_body_size") {
        return parseServerClientMaxBodySizeDirective(tokens, server);
    } else if (directive == "error_page") {
        return parseServerErrorPageDirective(tokens, server);
    } else if (directive == "default_error_page") {
        return parseServerDefaultErrorPageDirective(tokens, server);
    }

    return true; // Directiva no reconocida - no crítico
}

bool ConfigParser::parseLocationDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
    const std::string& directive = tokens[0];

    if (directive == "methods") {
        return parseLocationMethodsDirective(tokens, location);
    } else if (directive == "root") {
        return parseLocationRootDirective(tokens, location);
    } else if (directive == "index") {
        return parseLocationIndexDirective(tokens, location);
    } else if (directive == "autoindex") {
        return parseLocationAutoindexDirective(tokens, location);
    } else if (directive == "upload_dir") {
        return parseLocationUploadDirDirective(tokens, location);
    } else if (directive == "cgi_extension") {
        return parseLocationCgiExtensionDirective(tokens, location);
    } else if (directive == "cgi_path") {
        return parseLocationCgiPathDirective(tokens, location);
    } else if (directive == "client_max_body_size") {
        return parseLocationClientMaxBodySizeDirective(tokens, location);
    }

    return true; // Directiva no reconocida - no crítico
}
