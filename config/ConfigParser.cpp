#include "ConfigParser.hpp"
#include <iostream>

ConfigParser::ConfigParser() : _is_parsed(false) {}

ConfigParser::~ConfigParser() {}

bool ConfigParser::parseFile(const std::string &filename)
{
	std::cout << "ConfigParser: Loading " << filename << std::endl;

	// TEMPORARY CONFIGURATION for other parts to start working
	ServerConfig defaultServer;
	defaultServer.port = 8080;
	defaultServer.host = "0.0.0.0";
	defaultServer.root = "./www";
	defaultServer.index = "index.html";
	defaultServer.client_max_body_size = 1000000;

	// Default error pages
	defaultServer.error_pages[404] = "./errors/404.html";
	defaultServer.error_pages[500] = "./errors/500.html";

	// Default location
	LocationConfig rootLocation;
	rootLocation.path = "/";
	rootLocation.methods.push_back("GET");
	rootLocation.methods.push_back("POST");
	rootLocation.autoindex = true;

	defaultServer.locations.push_back(rootLocation);

	_servers.clear();
	_servers.push_back(defaultServer);
	_is_parsed = true;

	std::cout << "ConfigParser: Default config loaded (real parsing TODO)" << std::endl;
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
