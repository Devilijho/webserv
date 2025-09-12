#include "ConfigParser.hpp"

//============================================================================
// PARSING DE DIRECTIVAS SERVER
//============================================================================

bool ConfigParser::parseServerPortDirective(const std::vector<std::string>& tokens, ServerConfig* server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty listen directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    // ✅ USAR FUNCIÓN CENTRALIZADA
    return validateAndSetPort(value, server->port);
}

bool ConfigParser::parseServerHostDirective(const std::vector<std::string>& tokens, ServerConfig *server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty host directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    // ✅ USAR FUNCIÓN CENTRALIZADA
    return validateAndSetHost(value, server->host);
}

bool ConfigParser::parseServerRootDirective(const std::vector<std::string>& tokens, ServerConfig *server) {
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

	server->root = value;
	return true;
}

bool ConfigParser::parseServerIndexDirective(const std::vector<std::string>& tokens, ServerConfig *server) {
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

	server->index = value;
	return true;
}

bool ConfigParser::parseServerNameDirective(const std::vector<std::string>& tokens, ServerConfig *server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty server_name directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    if (value.empty()) {
        std::cerr << "Error: server_name cannot be empty" << std::endl;
        return false;
    }

    // ✅ SOLO AÑADIR VALIDACIÓN BÁSICA
    if (!isValidHostname(value)) {
        std::cerr << "Error: Invalid server_name: " << value << std::endl;
        return false;
    }

    server->server_name = value;
    std::cout << "Server name set to: " << value << std::endl;

    // ⚠️ ADVERTIR SI HAY MÁS DE UNO (PERO NO FALLAR)
    if (tokens.size() > 2) {
        std::cout << "Error: Multiple server_names detected. Using only the first one: " << value << std::endl;
		return false;
    }

    return true;
}

bool ConfigParser::parseServerClientMaxBodySizeDirective(const std::vector<std::string>& tokens, ServerConfig *server) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty client_max_body_size directive" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    // ✅ USAR FUNCIÓN CENTRALIZADA
    return validateAndSetClientMaxBodySize(value, server->client_max_body_size, " in server");
}

bool ConfigParser::parseServerErrorPageDirective(const std::vector<std::string>& tokens, ServerConfig *server) {
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

	server->error_pages[errorCode] = path;
	return true;
}

bool ConfigParser::parseServerDefaultErrorPageDirective(const std::vector<std::string>& tokens, ServerConfig* server) {
	if (tokens.size() < 2) {
		std::cerr << "Error: Empty default_error_page directive" << std::endl;
		return false;
	}

	std::string value = tokens[1];

	if (!value.empty() && value[value.length() - 1] == ';') {
		value = value.substr(0, value.length() - 1);
	}

	if (value.empty()) {
		std::cerr << "Error: default_error_page cannot be empty" << std::endl;
		return false;
	}

	server->default_error_page = value;
	return true;
}
