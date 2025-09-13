#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

// Foward declaration
struct LocationConfig;

// DEFINICIONES COMPLETAS DE ESTRUCTURAS
struct LocationConfig {
	std::string path;
	std::vector<std::string> methods;
	std::string root;
	std::string index;
	bool autoindex;
	std::string upload_dir;
	std::string cgi_extension;
	std::string cgi_path;
	size_t client_max_body_size;

	bool has_return;
	int return_code;
	std::string return_url;

	// Constructor con valores por defecto
	LocationConfig();
};

struct ServerConfig {
	int port;
	std::string host;
	std::string server_name;
	std::string root;
	std::string index;
	size_t client_max_body_size;
	std::map<int, std::string> error_pages;
	std::string default_error_page;
	std::vector<LocationConfig*> locations;

	// Constructor con valores por defecto
	ServerConfig();
	~ServerConfig();

	// Critical methods for Part C
	const LocationConfig *findLocation(const std::string &path) const;
};

#endif
