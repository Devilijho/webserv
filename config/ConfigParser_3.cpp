#include "ConfigParser.hpp"

//============================================================================
// PARSING DE DIRECTIVAS LOCATION
//============================================================================

bool ConfigParser::parseLocationHeader(const std::string& line, LocationConfig& location) {
	size_t locationPos = line.find("location");
	if (locationPos == std::string::npos) {
		std::cerr << "Error: Invalid location header format" << std::endl;
		return false;
	}

	size_t pathStart = line.find_first_not_of(" \t", locationPos + 8);
	size_t pathEnd = line.find_first_of(" \t{", pathStart);

	if (pathStart == std::string::npos || pathEnd == std::string::npos) {
		std::cerr << "Error: Missing location path" << std::endl;
		return false;
	}

	std::string path = line.substr(pathStart, pathEnd - pathStart);
	if (path.empty() || path[0] != '/') {
		std::cerr << "Error: Location path must start with '/': " << path << std::endl;
		return false;
	}

	location.path = path;
	return true;
}

bool ConfigParser::parseLocationMethodsDirective(const std::vector<std::string>& tokens, LocationConfig *location) {
	// if (tokens.size() < 2) {
	//	 std::cerr << "Error: Empty methods directive in location" << std::endl;
	//	 return false;
	// }

	location->methods.clear();

	for (size_t i = 1; i < tokens.size(); ++i) {
		std::string method = tokens[i];

		if (!method.empty() && method[method.length() - 1] == ';') {
			method = method.substr(0, method.length() - 1);
		}

		if (method.empty()) continue;

		if (!isValidMethod(method)) {
			std::cerr << "Error: Invalid HTTP method: " << method << std::endl;
			return false;
		}

		location->methods.push_back(method);
	}

	// if (location->methods.empty()) {
	//	 std::cerr << "Error: No valid methods found in location" << std::endl;
	//	 return false;
	// }

	return true;
}

bool ConfigParser::parseLocationRootDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
	if (tokens.size() < 2) {
		std::cerr << "Error: Empty root directive in location" << std::endl;
		return false;
	}

	std::string value = tokens[1];
	if (!value.empty() && value[value.length() - 1] == ';') {
		value = value.substr(0, value.length() - 1);
	}

	if (value.empty()) {
		std::cerr << "Error: Location root directory cannot be empty" << std::endl;
		return false;
	}

	location.root = value;
	return true;
}

bool ConfigParser::parseLocationIndexDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
	if (tokens.size() < 2) {
		std::cerr << "Error: Empty index directive in location" << std::endl;
		return false;
	}

	std::string value = tokens[1];
	if (!value.empty() && value[value.length() - 1] == ';') {
		value = value.substr(0, value.length() - 1);
	}

	if (value.empty()) {
		std::cerr << "Error: Location index file cannot be empty" << std::endl;
		return false;
	}

	if (value.find('.') == std::string::npos) {
		std::cerr << "Error: Location index file '" << value << "' must have extension" << std::endl;
		return false;
	}

	location.index = value;
	return true;
}

bool ConfigParser::parseLocationAutoindexDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
	if (tokens.size() < 2) {
		std::cerr << "Error: Empty autoindex directive" << std::endl;
		return false;
	}

	std::string value = tokens[1];
	if (!value.empty() && value[value.length() - 1] == ';') {
		value = value.substr(0, value.length() - 1);
	}

	if (value != "on" && value != "off") {
		std::cerr << "Error: autoindex must be 'on' or 'off', got: " << value << std::endl;
		return false;
	}

	location.autoindex = (value == "on");
	return true;
}

bool ConfigParser::parseLocationUploadDirDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
	if (tokens.size() < 2) {
		std::cerr << "Error: Empty upload_dir directive" << std::endl;
		return false;
	}

	std::string value = tokens[1];
	if (!value.empty() && value[value.length() - 1] == ';') {
		value = value.substr(0, value.length() - 1);
	}

	if (value.empty()) {
		std::cerr << "Error: upload_dir cannot be empty" << std::endl;
		return false;
	}

	location.upload_dir = value;
	return true;
}

bool ConfigParser::parseLocationCgiExtensionDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
	if (tokens.size() < 2) {
		std::cerr << "Error: Empty cgi_extension directive" << std::endl;
		return false;
	}

	std::string value = tokens[1];
	if (!value.empty() && value[value.length() - 1] == ';') {
		value = value.substr(0, value.length() - 1);
	}

	if (value.empty()) {
		std::cerr << "Error: cgi_extension cannot be empty" << std::endl;
		return false;
	}

	if (value[0] != '.') {
		std::cerr << "Error: cgi_extension must start with '.': " << value << std::endl;
		return false;
	}

	location.cgi_extension = value;
	return true;
}

bool ConfigParser::parseLocationCgiPathDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
	if (tokens.size() < 2) {
		std::cerr << "Error: Empty cgi_path directive" << std::endl;
		return false;
	}

	std::string value = tokens[1];
	if (!value.empty() && value[value.length() - 1] == ';') {
		value = value.substr(0, value.length() - 1);
	}

	if (value.empty()) {
		std::cerr << "Error: cgi_path cannot be empty" << std::endl;
		return false;
	}

	location.cgi_path = value;
	return true;
}

bool ConfigParser::parseLocationClientMaxBodySizeDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
    if (tokens.size() < 2) {
        std::cerr << "Error: Empty client_max_body_size directive in location" << std::endl;
        return false;
    }

    std::string value = tokens[1];
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }

    // ✅ USAR FUNCIÓN CENTRALIZADA
    return validateAndSetClientMaxBodySize(value, location.client_max_body_size, " in location");
}

bool ConfigParser::parseLocationReturnDirective(const std::vector<std::string>& tokens, LocationConfig& location) {
    if (tokens.size() < 3) {
        std::cerr << "Error: return directive requires code and URL" << std::endl;
        return false;
    }

    std::string codeStr = tokens[1];
    std::string url = tokens[2];

    // ✅ REMOVER ; AL FINAL (C++98 COMPATIBLE)
    if (!url.empty() && url[url.length() - 1] == ';') {
        url = url.substr(0, url.length() - 1);
    }

    // ✅ VALIDAR CÓDIGO DE REDIRECT
    int code = atoi(codeStr.c_str());
    if (code < 300 || code > 399) {
        std::cerr << "Error: Invalid redirect code: " << code
                  << " (must be 300-399)" << std::endl;
        return false;
    }

    // ✅ VALIDAR CÓDIGOS ESPECÍFICOS
    if (code != 301 && code != 302 && code != 303 && code != 307 && code != 308) {
        std::cerr << "Error: Unsupported redirect code: " << code
                  << " (supported: 301, 302, 303, 307, 308)" << std::endl;
        return false;
    }

    if (url.empty()) {
        std::cerr << "Error: Redirect URL cannot be empty" << std::endl;
        return false;
    }

    // ✅ VALIDACIÓN BÁSICA DE URL
    if (url[0] != '/' && url.find("http://") != 0 && url.find("https://") != 0) {
        std::cerr << "Error: Redirect URL must be absolute path or full URL: "
                  << url << std::endl;
        return false;
    }

    location.has_return = true;
    location.return_code = code;
    location.return_url = url;

    std::cout << "Redirect configured: " << code << " -> " << url << std::endl;
    return true;
}

