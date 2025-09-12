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
	// ✅ SOLO VERIFICAR VALORES FINALES, NO RE-VALIDAR
	if (srv->port <= 0 || srv->port > 65535) {
		std::cerr << "Server " << serverIndex << ": Invalid port " << srv->port << std::endl;
		return false;
	}

	if (srv->host.empty()) {
		std::cerr << "Server " << serverIndex << ": Host cannot be empty" << std::endl;
		return false;
	}

	if (srv->index.empty()) {
		std::cerr << "Server " << serverIndex << ": Index file cannot be empty" << std::endl;
		return false;
	}

	// ✅ AÑADIR VALIDACIÓN DE server_name
	if (srv->server_name.empty()) {
		std::cerr << "Server " << serverIndex << ": server_name cannot be empty" << std::endl;
		return false;
	}

	// ✅ NO RE-VALIDAR client_max_body_size (ya validado en parsing)
	if (srv->client_max_body_size == 0) {
		std::cerr << "Server " << serverIndex << ": client_max_body_size not set" << std::endl;
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

	if (srv->default_error_page.empty()) {
		std::cerr << "Server " << serverIndex << ": default_error_page not configured" << std::endl;
		return false;
	}

	if (access(srv->default_error_page.c_str(), R_OK) != 0) {
		std::cerr << "Server " << serverIndex << ": Default error page not found or not readable: "
				  << srv->default_error_page << std::endl;
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

	return validateLocationFiles(serverIndex, srv, loc);
}

bool ConfigParser::validateLocationFiles(size_t serverIndex, const ServerConfig *srv,
										const LocationConfig& loc) {
	// Permitir que el root de la location no exista físicamente (nginx-like)
	// Solo validar si el string está vacío, pero no si existe en disco
	std::string effectiveRoot = loc.root.empty() ? srv->root : loc.root;
	if (effectiveRoot.empty()) {
		std::cerr << "Server " << serverIndex << " Location '" << loc.path
				  << "': Root directory not specified" << std::endl;
		return false;
	}

	// No validar existencia de directorio ni index aquí
	// Eso se gestiona en tiempo de petición

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
