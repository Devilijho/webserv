#include "ConfigParser.hpp"

ConfigParser::ConfigParser() : _is_parsed(false) {}

ConfigParser::~ConfigParser() {}

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

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find server blocks
        if (line.find("server") == 0 && line.find("{") != std::string::npos) {
            ServerConfig server;
            if (parseServerBlock(file, server)) {
                _servers.push_back(server);
            }
        }
    }

    file.close();

    if (_servers.empty()) {
        std::cerr << "Error: No valid server configurations found" << std::endl;
        return false;
    }

    _is_parsed = true;

    if (!validateConfig()) {
        std::cerr << "Configuration validation failed" << std::endl;
        _is_parsed = false;
        return false;
    }

    std::cout << "ConfigParser: Successfully loaded " << _servers.size() << " server(s)" << std::endl;
    return true;
}

const std::vector<ServerConfig> &ConfigParser::getServers() const
{
	return _servers;
}

bool ConfigParser::isValid() const
{
	return _is_parsed && !_servers.empty();
}

void ConfigParser::printConfig() const
{
	std::cout << "\n=== WEBSERV CONFIGURATION ===" << std::endl;
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		const ServerConfig &srv = _servers[i];
		std::cout << "Server " << i << ":" << std::endl;
		std::cout << "  Listen: " << srv.host << ":" << srv.port << std::endl;
		std::cout << "  Root: " << srv.root << std::endl;
		std::cout << "  Index: " << srv.index << std::endl;
		std::cout << "  Locations: " << srv.locations.size() << std::endl;
	}
	std::cout << "================================\n"
			  << std::endl;
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

bool ConfigParser::isValidMethod(const std::string& method) {
    return (method == "GET" || method == "POST" || method == "DELETE");
}

bool ConfigParser::isValidPort(int port) {
    return (port > 0 && port <= 65535);
}

std::string ConfigParser::removeInlineComment(const std::string& str) {
    size_t commentPos = str.find('#');
    if (commentPos != std::string::npos) {
        return trim(str.substr(0, commentPos));
    }
    return str;
}

bool ConfigParser::parseServerBlock(std::ifstream &file, ServerConfig &server) {
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') continue;

        line = removeInlineComment(line);
        if (line.empty()) continue;

        // End of server block
        if (line == "}") {
            return true;
        }

        // Parse location blocks
        if (line.find("location") == 0) {
            LocationConfig location;

            // Extract location path: "location /upload {" -> "/upload"
            size_t start = line.find_first_of(" \t") + 1;
            size_t end = line.find_first_of(" \t{", start);
            location.path = line.substr(start, end - start);

            if (parseLocationBlock(file, location)) {
                server.locations.push_back(location);
            }
            continue;
        }

        // Parse server directives
        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.size() < 2) continue;

        std::string directive = tokens[0];
        std::string value = tokens[1];

        // Remove semicolon from value
        if (!value.empty() && value[value.length() - 1] == ';') {
            value = value.substr(0, value.length() - 1);
        }

        if (directive == "listen") {
            server.port = atoi(value.c_str());
        } else if (directive == "host") {
            server.host = value;
        } else if (directive == "root") {
            server.root = value;
        } else if (directive == "index") {
            server.index = parseIndexValue(tokens);    // ← DEBE USAR LA FUNCIÓN
        } else if (directive == "server_name") {
            server.server_name = value;
        } else if (directive == "client_max_body_size") {
            server.client_max_body_size = parseClientMaxBodySize(tokens);  // ← DEBE USAR LA FUNCIÓN
        } else if (directive == "error_page" && tokens.size() >= 3) {
            int errorCode = atoi(value.c_str());
            std::string errorPage = tokens[2];
            if (!errorPage.empty() && errorPage[errorPage.length() - 1] == ';') {
                errorPage = errorPage.substr(0, errorPage.length() - 1);
            }
            server.error_pages[errorCode] = errorPage;
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

        // End of location block
        if (line == "}") {
            return true;
        }

        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.size() < 2) continue;

        std::string directive = tokens[0];

        if (directive == "methods") {
            // Parse: "methods GET POST DELETE;"
            for (size_t i = 1; i < tokens.size(); ++i) {
                std::string method = tokens[i];
                if (!method.empty() && method[method.length() - 1] == ';') {
                    method = method.substr(0, method.length() - 1);
                }
                if (isValidMethod(method)) {
                    location.methods.push_back(method);
                }
            }
        } else if (directive == "root") {
            std::string value = tokens[1];
            if (!value.empty() && value[value.length() - 1] == ';') {
                value = value.substr(0, value.length() - 1);
            }
            location.root = value;
        } else if (directive == "index") {
            std::string value = tokens[1];
            if (!value.empty() && value[value.length() - 1] == ';') {
                value = value.substr(0, value.length() - 1);
            }
            location.index = value;
        } else if (directive == "autoindex") {
            std::string value = tokens[1];
            if (!value.empty() && value[value.length() - 1] == ';') {
                value = value.substr(0, value.length() - 1);
            }
            location.autoindex = (value == "on");
        } else if (directive == "upload_dir") {
            std::string value = tokens[1];
            if (!value.empty() && value[value.length() - 1] == ';') {
                value = value.substr(0, value.length() - 1);
            }
            location.upload_dir = value;
        } else if (directive == "cgi_extension") {
            std::string value = tokens[1];
            if (!value.empty() && value[value.length() - 1] == ';') {
                value = value.substr(0, value.length() - 1);
            }
            location.cgi_extension = value;
        } else if (directive == "cgi_path") {
            std::string value = tokens[1];
            if (!value.empty() && value[value.length() - 1] == ';') {
                value = value.substr(0, value.length() - 1);
            }
            location.cgi_path = value;
        }
    }

    return false; // Missing closing brace
}

// Validar configuración después de analizar
bool ConfigParser::validateConfig() {
    if (_servers.empty()) {
        std::cerr << "No servers defined in configuration" << std::endl;
        return false;
    }

    // Validar cada servidor
    for (size_t i = 0; i < _servers.size(); ++i) {
        const ServerConfig& srv = _servers[i];

        // Validar puerto
        if (!isValidPort(srv.port)) {
            std::cerr << "Server " << i << ": Invalid port " << srv.port << std::endl;
            return false;
        }

        // VALIDAR HOST - AÑADIR ESTA SECCIÓN:
        if (srv.host.empty()) {
            std::cerr << "Server " << i << ": Host cannot be empty" << std::endl;
            return false;
        }

        if (!isValidHost(srv.host)) {
            std::cerr << "Server " << i << ": Invalid host format '" << srv.host
                      << "'. Expected valid IP address (e.g., 0.0.0.0, 127.0.0.1)" << std::endl;
            return false;
        }

        // AÑADIR VALIDACIÓN CRÍTICA DE INDEX
        if (srv.index.empty()) {
            std::cerr << "Server " << i << ": Index file cannot be empty" << std::endl;
            return false;  // ← CRÍTICO: falla si index está vacío
        }

        // VALIDAR QUE EL ARCHIVO INDEX EXISTE
        std::string fullIndexPath = srv.root + "/" + srv.index;
        if (access(fullIndexPath.c_str(), R_OK) != 0) {
            std::cerr << "Server " << i << ": Index file not found or not readable: "
                      << fullIndexPath << std::endl;
            return false;  // ← CRÍTICO: servidor no arranca si index no existe
        }

        // AÑADIR VALIDACIÓN CRÍTICA DE CLIENT_MAX_BODY_SIZE
        if (srv.client_max_body_size == 0) {
            std::cerr << "Server " << i << ": client_max_body_size cannot be zero" << std::endl;
            return false;  // ← CRÍTICO: falla si es 0
        }

        if (!isValidClientMaxBodySize(srv.client_max_body_size)) {
            std::cerr << "Server " << i << ": Invalid client_max_body_size: "
                      << srv.client_max_body_size << std::endl;
            return false;  // ← CRÍTICO: ya existe
        }

        // VALIDAR CLIENT_MAX_BODY_SIZE MÍNIMO
        if (srv.client_max_body_size < 1024) {  // Mínimo 1KB
            std::cerr << "Server " << i << ": client_max_body_size too small: "
                      << srv.client_max_body_size << " (minimum: 1024 bytes)" << std::endl;
            return false;  // ← CRÍTICO: tamaño demasiado pequeño
        }

        // Validar que root existe
        if (access(srv.root.c_str(), R_OK) != 0) {
            std::cerr << "Server " << i << ": Root directory not accessible: " << srv.root << std::endl;
            return false;
        }

        // Validar error pages
        for (std::map<int, std::string>::const_iterator it = srv.error_pages.begin();
             it != srv.error_pages.end(); ++it) {
            if (access(it->second.c_str(), R_OK) != 0) {
                std::cerr << "Server " << i << ": Error page not accessible: " << it->second
                          << " (for error " << it->first << ")" << std::endl;
                return false;
            }
        }

        // Validar que tiene locations
        if (srv.locations.empty()) {
            std::cerr << "Server " << i << ": No locations defined" << std::endl;
            return false;
        }

        // Verificar que existe location root "/"
        bool hasRootLocation = false;
        for (size_t j = 0; j < srv.locations.size(); ++j) {
            if (srv.locations[j].path == "/") {
                hasRootLocation = true;
                break;
            }
        }
        if (!hasRootLocation) {
            std::cerr << "Server " << i << ": Missing root location '/'" << std::endl;
            return false;
        }

        // Validar locations
        for (size_t j = 0; j < srv.locations.size(); ++j) {
            const LocationConfig& loc = srv.locations[j];

            // Validar path
            if (loc.path.empty() || loc.path[0] != '/') {
                std::cerr << "Server " << i << " Location " << j << ": Invalid path '" << loc.path << "'" << std::endl;
                return false;
            }

            // Validar methods
            if (loc.methods.empty()) {
                std::cerr << "Server " << i << " Location '" << loc.path << "': No methods defined" << std::endl;
                return false;
            }

            // Validar location root si está definido
            std::string effectiveRoot = loc.root.empty() ? srv.root : loc.root;
            if (access(effectiveRoot.c_str(), R_OK) != 0) {
                std::cerr << "Server " << i << " Location '" << loc.path << "': Root directory not accessible: "
                          << effectiveRoot << std::endl;
                return false;
            }

            // Validar upload_dir si está definido
            if (!loc.upload_dir.empty() && access(loc.upload_dir.c_str(), W_OK) != 0) {
                std::cerr << "Server " << i << " Location '" << loc.path << "': Upload directory not accessible: "
                          << loc.upload_dir << std::endl;
                return false;
            }

            // Si tiene CGI, validar que el intérprete existe
            if (!loc.cgi_path.empty()) {
                if (access(loc.cgi_path.c_str(), X_OK) != 0) {
                    std::cerr << "Server " << i << " Location '" << loc.path << "': CGI interpreter not found: "
                              << loc.cgi_path << std::endl;
                    return false;
                }

                // Validar que CGI extension está definida
                if (loc.cgi_extension.empty()) {
                    std::cerr << "Server " << i << " Location '" << loc.path << "': CGI path defined but no extension" << std::endl;
                    return false;
                }
            }

            // AÑADIR VALIDACIÓN DE INDEX EN LOCATION
            if (!loc.index.empty()) {
                std::string locationRoot = loc.root.empty() ? srv.root : loc.root;
                std::string fullLocationIndexPath = locationRoot + "/" + loc.index;
                if (access(fullLocationIndexPath.c_str(), R_OK) != 0) {
                    std::cerr << "Server " << i << " Location '" << loc.path
                              << "': Index file not found: " << fullLocationIndexPath << std::endl;
                    return false;  // ← CRÍTICO: location index no existe
                }
            }
        }
    }

    // Verificar puertos duplicados entre servidores
    for (size_t i = 0; i < _servers.size(); ++i) {
        for (size_t j = i + 1; j < _servers.size(); ++j) {
            if (_servers[i].port == _servers[j].port && _servers[i].host == _servers[j].host) {
                std::cerr << "Duplicate server configuration: "
                          << _servers[i].host << ":" << _servers[i].port << std::endl;
                return false;
            }
        }
    }

    return true;
}

// AÑADIR ESTAS FUNCIONES AL FINAL DEL ARCHIVO:

bool ConfigParser::isValidIPAddress(const std::string& ip) {
    if (ip.empty()) return false;

    // Casos especiales válidos
    if (ip == "0.0.0.0" || ip == "localhost" || ip == "127.0.0.1") {
        return true;
    }

    // Tu validación preferida:
    std::string temp = "";
    std::istringstream iss(ip);
    int count = 0;

    while (std::getline(iss, temp, '.')) {
        count++;
        if (count > 4) return false;  // Máximo 4 octetos

        if (temp.empty()) return false;  // Octeto vacío

        // Solo dígitos permitidos
        for (int j = 0; j < (int)temp.size(); j++) {
            if (!isdigit(temp[j])) return false;
        }

        // Verificar rango 0-255
        int value = atoi(temp.c_str());
        if (value < 0 || value > 255) return false;
    }

    return (count == 4);  // Debe tener exactamente 4 octetos
}

bool ConfigParser::isValidHost(const std::string& host) {
    // Por ahora, solo validar IPs
    return isValidIPAddress(host);
}

// FUNCIÓN PARA PARSEAR INDEX
std::string ConfigParser::parseIndexValue(const std::vector<std::string>& tokens) {
    // Verificar que hay suficientes tokens
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty index directive" << std::endl;  // ← CAMBIAR A ERROR
        return "";  // ← RETORNAR VACÍO PARA QUE FALLE VALIDACIÓN
    }

    std::string value = tokens[1];

    // Remover semicolon si existe
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    // Verificar que no esté vacío después de limpiar
    if (value.empty()) {
        std::cerr << "Error: Empty index value" << std::endl;  // ← CAMBIAR A ERROR
        return "";  // ← RETORNAR VACÍO PARA QUE FALLE VALIDACIÓN
    }

    // CAMBIAR: HACER QUE FALLE SI NO TIENE EXTENSIÓN
    if (value.find('.') == std::string::npos) {
        std::cerr << "Error: Index file '" << value << "' has no extension (required)" << std::endl;  // ← ERROR CRÍTICO
        return "";  // ← RETORNAR VACÍO PARA QUE FALLE VALIDACIÓN
    }

    return value;
}

// FUNCIÓN PARA PARSEAR CLIENT_MAX_BODY_SIZE
size_t ConfigParser::parseClientMaxBodySize(const std::vector<std::string>& tokens) {
    // Verificar que hay suficientes tokens
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty client_max_body_size directive" << std::endl;  // ← ERROR
        return 0;  // ← RETORNAR 0 PARA QUE FALLE VALIDACIÓN
    }

    std::string value = tokens[1];

    // Remover semicolon si existe
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    // Verificar que no esté vacío
    if (value.empty()) {
        std::cerr << "Error: Empty client_max_body_size value" << std::endl;  // ← ERROR
        return 0;  // ← RETORNAR 0 PARA QUE FALLE VALIDACIÓN
    }

    // Convertir a entero
    long size = atol(value.c_str());

    // HACER QUE VALORES INVÁLIDOS SEAN ERRORES CRÍTICOS
    if (size <= 0) {
        std::cerr << "Error: Invalid client_max_body_size '" << value
                  << "' (must be positive)" << std::endl;  // ← ERROR CRÍTICO
        return 0;  // ← RETORNAR 0 PARA QUE FALLE VALIDACIÓN
    }

    // Verificar límite superior (1GB)
    if (size > 1073741824) {
        std::cerr << "Error: client_max_body_size '" << value
                  << "' too large (max 1GB)" << std::endl;  // ← ERROR CRÍTICO
        return 0;  // ← RETORNAR 0 PARA QUE FALLE VALIDACIÓN
    }

    return static_cast<size_t>(size);
}

// FUNCIÓN PARA VALIDAR CLIENT_MAX_BODY_SIZE
bool ConfigParser::isValidClientMaxBodySize(size_t size) {
    if (size <= 0) {
        std::cerr << "Error: client_max_body_size must be positive" << std::endl;
        return false;
    }

    if (size > 1073741824) { // 1GB
        std::cerr << "Error: client_max_body_size cannot exceed 1GB" << std::endl;
        return false;
    }

    return true;
}

// bool ConfigParser::parseFile(const std::string &filename)
// {
//     std::cout << "ConfigParser: Loading " << filename << std::endl;

//     // Use constructor defaults and override only what's needed for development
//     ServerConfig defaultServer;  // Uses constructor: port=8080, host="localhost", etc.

//     // Override ONLY for development (accept external connections)
//     defaultServer.host = "0.0.0.0";

//     // Add configurations NOT in constructor
//     defaultServer.error_pages[404] = "./errors/404.html";
//     defaultServer.error_pages[500] = "./errors/500.html";

//     // Create default root location
//     LocationConfig rootLocation;  // Uses constructor: autoindex=false
//     rootLocation.path = "/";
//     rootLocation.methods.push_back("GET");
//     rootLocation.methods.push_back("POST");
//     rootLocation.autoindex = true;  // Override for root location

//     defaultServer.locations.push_back(rootLocation);

//     _servers.clear();
//     _servers.push_back(defaultServer);
//     _is_parsed = true;

//     std::cout << "ConfigParser: Default config loaded (real parsing TODO)" << std::endl;
//     return true;
// }

