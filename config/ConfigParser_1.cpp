#include "ConfigParser.hpp"

//============================================================================
// CONSTRUCTOR Y DESTRUCTOR
//============================================================================

ConfigParser::ConfigParser() : _is_parsed(false) {}

ConfigParser::~ConfigParser() {
	for (size_t i = 0; i < _servers.size(); ++i) {
		ServerConfig* server = _servers[i];

		// Delete all LocationConfig objects first
		for (size_t j = 0; j < server->locations.size(); ++j) {
			delete server->locations[j];
		}
		server->locations.clear();

		// Then delete the ServerConfig
		delete server;
	}
	_servers.clear();
}

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

	// Cleanup old servers if any
	for (size_t i = 0; i < _servers.size(); ++i) {
		ServerConfig* server = _servers[i];
		for (size_t j = 0; j < server->locations.size(); ++j)
			delete server->locations[j];
		delete server;
	}
	_servers.clear();
	std::string line;

	while (std::getline(file, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#') continue;

		if (line.find("server") == 0 && line.find("{") != std::string::npos) {
			ServerConfig* server = new ServerConfig();
			if (parseServerBlock(file, server)) {
				_servers.push_back(server);  // push pointer
			} else {
				delete server;  // cleanup if parse failed
			}
		}
	}

	file.close();
	return finalizeConfig();
}

bool ConfigParser::finalizeConfig() {
	std::vector<ServerConfig*> validServers;
	// Validar cada servidor por separado

	for (size_t i = 0; i < _servers.size(); ++i) {
		ServerConfig* server = _servers[i];

		// ✅ APLICAR HERENCIA ANTES DE VALIDAR
		for (size_t j = 0; j < server->locations.size(); ++j) {
			LocationConfig* location = server->locations[j];

			// Si no está configurado (valor 0), heredar del servidor
			if (location->client_max_body_size == 0) {
				location->client_max_body_size = server->client_max_body_size;
			}
		}

		if (validateServer(i, server)) {
			validServers.push_back(server);
		} else {
			delete server; // cleanup invalid servers
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

bool ConfigParser::parseServerBlock(std::ifstream &file, ServerConfig* server) {
	std::string line;

	while (std::getline(file, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#') continue;

		line = removeInlineComment(line);
		if (line.empty()) continue;

		if (line == "}") return true;

		if (line.find("location") == 0) {
			LocationConfig* location = new LocationConfig();
			if (parseLocationHeader(line, *location) && parseLocationBlock(file, location)) {
				server->locations.push_back(location);
			}
			else
				delete location;
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

bool ConfigParser::parseLocationBlock(std::ifstream &file, LocationConfig* location) {
	std::string line;

	while (std::getline(file, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#') continue;

		line = removeInlineComment(line);
		if (line.empty()) continue;

		if (line == "}") return true;

		std::vector<std::string> tokens = split(line, ' ');
		// if (tokens.size() < 2) continue;
		// Permitir locations vacías (sin directivas)
		if (tokens.empty()) continue;

		if (!parseLocationDirective(tokens, location)) {
			return false;
		}
	}

	return false; // Missing closing brace
}

//============================================================================
// DISPATCHERS DE DIRECTIVAS
//============================================================================

bool ConfigParser::parseServerDirective(const std::vector<std::string>& tokens, ServerConfig* server) {
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

bool ConfigParser::parseLocationDirective(const std::vector<std::string>& tokens, LocationConfig* location) {
	const std::string& directive = tokens[0];

	if (directive == "methods") {
		return parseLocationMethodsDirective(tokens, location);
	} else if (directive == "root") {
		return parseLocationRootDirective(tokens, *location);
	} else if (directive == "index") {
		return parseLocationIndexDirective(tokens, *location);
	} else if (directive == "autoindex") {
		return parseLocationAutoindexDirective(tokens, *location);
	} else if (directive == "upload_dir") {
		return parseLocationUploadDirDirective(tokens, *location);
	} else if (directive == "cgi_extension") {
		return parseLocationCgiExtensionDirective(tokens, *location);
	} else if (directive == "cgi_path") {
		return parseLocationCgiPathDirective(tokens, *location);
	} else if (directive == "client_max_body_size") {
		return parseLocationClientMaxBodySizeDirective(tokens, *location);
	} else if (directive == "return") {
        return parseLocationReturnDirective(tokens, *location);
    }

	return true; // Directiva no reconocida - no crítico
}
