#include "RequestHandler/RequestHandler.hpp"
#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "Requests/server.hpp"

// int	main(void)
// {
// 	Server server(8080);
// 	ConfigParser parserConfig;
// 	ServerConfig serverConfig;

// 	if (!server.start()) {
// 		return 1;
// 	}
// 	return (0);
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
	std::vector<ServerConfig> servers = parser.getServers();
    if (servers.empty()) {
        std::cerr << "No servers defined in configuration." << std::endl;
        return 1;
    }

    // Start each server
    for (size_t i = 0; i < servers.size(); ++i) {
        const ServerConfig& cfg = servers[i];
        std::cout << "Starting server on " << cfg.host << ":" << cfg.port << std::endl;

        // Create server instance
        Server server;  // no arguments
		std::vector<ServerConfig> singleServer;
		singleServer.push_back(cfg);
		server.start(singleServer);
    }

    return 0;
}


// int main() {
//	 ConfigParser parser;

//	 // Test parsing
//	 if (parser.parseFile("default.conf")) {
//	 std::cout << "✅ Parsing successful" << std::endl;
//	 parser.printConfig();

//	 // Test server access
//	 const std::vector<ServerConfig>& servers = parser.getServers();
//	 if (!servers.empty()) {
//	 const ServerConfig& server = servers[0];
//	 std::cout << "✅ First server port: " << server.port << std::endl;

//	 // Test location finding
//	 const LocationConfig* loc = server.findLocation("/upload");
//	 if (loc) {
//	 std::cout << "✅ Found /upload location with " << loc->methods.size() << " methods" << std::endl;
//	 }
//	 }
//	 } else {
//	 std::cout << "❌ Parsing failed" << std::endl;
//	 }

//	 return 0;
// }
