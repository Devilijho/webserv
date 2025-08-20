#include "RequestHandler/RequestHandler.hpp"
#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "Requests/server.hpp"
#include <iostream>
#include <vector>

// FUNCIÓN DE DEBUG (definida ANTES del main):
void debugConfig(const ServerConfig& config) {
    std::cout << "\n=== DEBUG: TESTING CONFIG INTEGRATION ===" << std::endl;

    // Test location finding
    const LocationConfig* loc_root = config.findLocation("/");
    const LocationConfig* loc_cgi = config.findLocation("/cgi-bin/test.php");
    const LocationConfig* loc_upload = config.findLocation("/upload");

    std::cout << "🔍 Location tests:" << std::endl;
    std::cout << "  / -> " << (loc_root ? "FOUND" : "NOT FOUND") << std::endl;
    std::cout << "  /cgi-bin/test.php -> " << (loc_cgi ? loc_cgi->path : "NOT FOUND") << std::endl;
    std::cout << "  /upload -> " << (loc_upload ? "FOUND" : "NOT FOUND") << std::endl;

    if (loc_cgi) {
        std::cout << "🔧 CGI config:" << std::endl;
        std::cout << "  Extension: " << loc_cgi->cgi_extension << std::endl;
        std::cout << "  Path: " << loc_cgi->cgi_path << std::endl;
        std::cout << "  Root: " << loc_cgi->root << std::endl;
    }

    std::cout << "📄 Error pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = config.error_pages.begin();
         it != config.error_pages.end(); ++it) {
        std::cout << "  " << it->first << " -> " << it->second << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string configFile = (argc > 1) ? argv[1] : "default.conf";

    // Parse configuration
    ConfigParser parser;
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
    const std::vector<ServerConfig>& servers = parser.getServers();

    if (servers.empty()) {
        std::cerr << "Error: No servers configured" << std::endl;
        return 1;
    }

    // Para testing inicial: usar SOLO el primer servidor
    const ServerConfig& config = servers[0];

    // AQUÍ es donde llamas la función de debug (DENTRO del main):
    debugConfig(config);

    std::cout << "\n🚀 Starting server on " << config.host << ":" << config.port << std::endl;
    std::cout << "Root directory: " << config.root << std::endl;
    std::cout << "Index file: " << config.index << std::endl;
    std::cout << "Locations configured: " << config.locations.size() << std::endl;

    // Crear servidor único para testing
    Server server(config.port);

    // Cargar la configuración en el servidor (necesitas implementar esto)
    // server.loadConfig(config);  // ← Esto lo implementaremos después si es necesario

    if (!server.start()) {
        std::cerr << "Error: Failed to start server on port " << config.port << std::endl;
        return 1;
    }

    std::cout << "✅ Server started successfully!" << std::endl;
    std::cout << "Test URLs:" << std::endl;
    std::cout << "  http://localhost:" << config.port << "/" << std::endl;
    std::cout << "  http://localhost:" << config.port << "/cgi-bin/" << std::endl;
    std::cout << "  http://localhost:" << config.port << "/api/" << std::endl;

    return 0;
}

// int main() {
//     ConfigParser parser;

//     // Test parsing
//     if (parser.parseFile("default.conf")) {
//         std::cout << "✅ Parsing successful" << std::endl;
//         parser.printConfig();

//         // Test server access
//         const std::vector<ServerConfig>& servers = parser.getServers();
//         if (!servers.empty()) {
//             const ServerConfig& server = servers[0];
//             std::cout << "✅ First server port: " << server.port << std::endl;

//             // Test location finding
//             const LocationConfig* loc = server.findLocation("/upload");
//             if (loc) {
//                 std::cout << "✅ Found /upload location with " << loc->methods.size() << " methods" << std::endl;
//             }
//         }
//     } else {
//         std::cout << "❌ Parsing failed" << std::endl;
//     }

//     return 0;
// }
