#include "ConfigParser.hpp"
#include <iostream>

ConfigParser::ConfigParser() : _is_parsed(false) {}

ConfigParser::~ConfigParser() {}

bool ConfigParser::parseFile(const std::string &filename)
{
    std::cout << "ConfigParser: Loading " << filename << std::endl;

    // Use constructor defaults and override only what's needed for development
    ServerConfig defaultServer;  // Uses constructor: port=8080, host="localhost", etc.

    // Override ONLY for development (accept external connections)
    defaultServer.host = "0.0.0.0";

    // Add configurations NOT in constructor
    defaultServer.error_pages[404] = "./errors/404.html";
    defaultServer.error_pages[500] = "./errors/500.html";

    // Create default root location
    LocationConfig rootLocation;  // Uses constructor: autoindex=false
    rootLocation.path = "/";
    rootLocation.methods.push_back("GET");
    rootLocation.methods.push_back("POST");
    rootLocation.autoindex = true;  // Override for root location

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
