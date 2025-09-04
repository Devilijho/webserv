#include "RequestHandler/RequestHandler.hpp"
// #include "config/ConfigParser.hpp"
// #include "config/ServerConfig.hpp"
#include "config/ConfigParser.hpp"
#include "config/ServerConfig2.hpp"
#include "Requests/server.hpp"
#include <iostream>
#include <vector>

// // FUNCIÓN DE DEBUG (definida ANTES del main):
// void debugConfig(const ServerConfig& config) {
//	 std::cout << "\n=== DEBUG: TESTING CONFIG INTEGRATION ===" << std::endl;

//	 // Test location finding
//	 const LocationConfig* loc_root = config.findLocation("/");
//	 const LocationConfig* loc_cgi = config.findLocation("/cgi-bin/test.php");
//	 const LocationConfig* loc_upload = config.findLocation("/upload");

//	 std::cout << "🔍 Location tests:" << std::endl;
//	 std::cout << "  / -> " << (loc_root ? "FOUND" : "NOT FOUND") << std::endl;
//	 std::cout << "  /cgi-bin/test.php -> " << (loc_cgi ? loc_cgi->path : "NOT FOUND") << std::endl;
//	 std::cout << "  /upload -> " << (loc_upload ? "FOUND" : "NOT FOUND") << std::endl;

//	 if (loc_cgi) {
//		 std::cout << "🔧 CGI config:" << std::endl;
//		 std::cout << "  Extension: " << loc_cgi->cgi_extension << std::endl;
//		 std::cout << "  Path: " << loc_cgi->cgi_path << std::endl;
//		 std::cout << "  Root: " << loc_cgi->root << std::endl;
//	 }

//	 std::cout << "📄 Error pages:" << std::endl;
//	 for (std::map<int, std::string>::const_iterator it = config.error_pages.begin();
//		  it != config.error_pages.end(); ++it) {
//		 std::cout << "  " << it->first << " -> " << it->second << std::endl;
//	 }
// }


int main(int argc, char* argv[])
{
	// Parse configuration
	ConfigParser parser;
	std::string configFile = (argc > 1) ? argv[1] : "default.conf";

	if (!parser.parseFile(configFile)) {
		std::cerr << "Error: Failed to parse configuration file" << std::endl;
		return 1;
	}

	if (!parser.isValid()) {
	std::cerr << "Error: Invalid configuration" << std::endl;
	return 1;
	}

	// Debug: Print loaded configuration
	parser.printConfig();

	// Get servers from configuration
	std::vector<ServerConfig*> servers = parser.getServers();
	if (servers.empty()) {
		std::cerr << "No servers defined in configuration." << std::endl;
		return 1;
	}

	// Create one Server instance to manage all sockets
    Server server;
    if (!server.start(servers, configFile)) {
        std::cerr << "Error: Failed to start servers" << std::endl;
        return 1;
    }

	// // Convert to vector of pointers
	// std::vector<ServerConfig*> serverPtrs;
	// for (size_t i = 0; i < servers.size(); ++i) {
	// 	 ServerConfig* server = new ServerConfig(*servers[i]); // heap-allocate a copy
	// 	serverPtrs.push_back(server);
	// }

	// // Start each server
	// for (size_t i = 0; i < serverPtrs.size(); ++i) {
	// 	ServerConfig* cfg = serverPtrs[i];
	// 	std::cout << "Starting server on " << cfg->host << ":" << cfg->port << std::endl;

	// 	// Create server instance
	// 	Server server;
	// 	std::vector<ServerConfig*> singleServer;
	// 	singleServer.push_back(cfg);

	// 	server.start(singleServer, configFile);
	// }

	// // Free memory (avoid leaks)
	// for (size_t i = 0; i < serverPtrs.size(); ++i) {
	// 	delete serverPtrs[i];
	// }

	return 0;
}
