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
