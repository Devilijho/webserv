// test_parsing.cpp - Test independiente para ConfigParser
#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include <iostream>

void testBasicParsing() {
	std::cout << "\n=== TEST 1: BASIC PARSING ===" << std::endl;

	ConfigParser parser;

	if (!parser.parseFile("default.conf")) {
		std::cout << "❌ parseFile() failed" << std::endl;
		return;
	}

	std::cout << "✅ parseFile() successful" << std::endl;

	if (!parser.isValid()) {
		std::cout << "❌ Configuration invalid" << std::endl;
		return;
	}

	std::cout << "✅ Configuration is valid" << std::endl;

	const std::vector<ServerConfig>& servers = parser.getServers();
	std::cout << "📊 Found " << servers.size() << " servers" << std::endl;
}

void testServerDetails() {
	std::cout << "\n=== TEST 2: SERVER DETAILS ===" << std::endl;

	ConfigParser parser;
	parser.parseFile("default.conf");

	const std::vector<ServerConfig>& servers = parser.getServers();

	if (servers.empty()) {
		std::cout << "❌ No servers found" << std::endl;
		return;
	}

	// Test Server 1 (8080)
	const ServerConfig& srv1 = servers[0];
	std::cout << "🖥️  Server 1:" << std::endl;
	std::cout << "   Port: " << srv1.port << " (expected: 8080)" << std::endl;
	std::cout << "   Host: " << srv1.host << " (expected: 0.0.0.0)" << std::endl;
	std::cout << "   Root: " << srv1.root << " (expected: ./www)" << std::endl;
	std::cout << "   Index: " << srv1.index << " (expected: index.html)" << std::endl;
	std::cout << "   Max body size: " << srv1.client_max_body_size << " (expected: 1048576)" << std::endl;
	std::cout << "   Locations: " << srv1.locations.size() << std::endl;
	std::cout << "   Error pages: " << srv1.error_pages.size() << std::endl;

	// Verificar valores específicos
	bool server1_ok = (srv1.port == 8080 &&
					  srv1.host == "0.0.0.0" &&
					  srv1.root == "./www" &&
					  srv1.index == "index.html");

	std::cout << (server1_ok ? "✅" : "❌") << " Server 1 basic config" << std::endl;

	// Test Server 2 (8081) si existe
	if (servers.size() >= 2) {
		const ServerConfig& srv2 = servers[1];
		std::cout << "\n🖥️  Server 2:" << std::endl;
		std::cout << "   Port: " << srv2.port << " (expected: 8081)" << std::endl;
		std::cout << "   Host: " << srv2.host << " (expected: 0.0.0.0)" << std::endl;
		std::cout << "   Root: " << srv2.root << " (expected: ./api_server)" << std::endl;

		bool server2_ok = (srv2.port == 8081 && srv2.root == "./api_server");
		std::cout << (server2_ok ? "✅" : "❌") << " Server 2 basic config" << std::endl;
	}
}

void testLocations() {
	std::cout << "\n=== TEST 3: LOCATION PARSING ===" << std::endl;

	ConfigParser parser;
	parser.parseFile("default.conf");

	const std::vector<ServerConfig>& servers = parser.getServers();
	const ServerConfig& srv = servers[0];

	std::cout << "📍 Testing locations for Server 1:" << std::endl;

	// Verificar locations específicas
	for (size_t i = 0; i < srv.locations.size(); ++i) {
		const LocationConfig& loc = srv.locations[i];
		std::cout << "   Location " << i << ": " << loc.path << std::endl;
		std::cout << "	  Methods: ";
		for (size_t j = 0; j < loc.methods.size(); ++j) {
			std::cout << loc.methods[j] << " ";
		}
		std::cout << std::endl;

		if (!loc.root.empty()) {
			std::cout << "	  Root override: " << loc.root << std::endl;
		}

		if (!loc.upload_dir.empty()) {
			std::cout << "	  Upload dir: " << loc.upload_dir << std::endl;
		}

		if (!loc.cgi_extension.empty()) {
			std::cout << "	  CGI ext: " << loc.cgi_extension << ", path: " << loc.cgi_path << std::endl;
		}

		std::cout << "	  Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
		std::cout << std::endl;
	}
}

void testLocationFinding() {
	std::cout << "\n=== TEST 4: LOCATION FINDING ===" << std::endl;

	ConfigParser parser;
	parser.parseFile("default.conf");

	const std::vector<ServerConfig>& servers = parser.getServers();
	const ServerConfig& srv = servers[0];

	// Test location finding
	struct TestCase {
		std::string path;
		std::string expected_location;
	};

	TestCase tests[] = {
		{"/", "/"},
		{"/index.html", "/"},
		{"/upload", "/upload"},
		{"/upload/file.txt", "/upload"},
		{"/script/query.php", "/script"},
		{"/script/upload.php", "/script"},
		{"/api/users", "/api"},
		{"/api/users/123", "/api"},
		{"/nonexistent", "/"}  // Should fallback to root
	};

	for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
		const LocationConfig* found = srv.findLocation(tests[i].path);
		std::string result = found ? found->path : "NOT FOUND";

		bool correct = (result == tests[i].expected_location);
		std::cout << (correct ? "✅" : "❌") << " " << tests[i].path
				  << " → " << result
				  << " (expected: " << tests[i].expected_location << ")" << std::endl;
	}
}

void testErrorPages() {
	std::cout << "\n=== TEST 5: ERROR PAGES ===" << std::endl;

	ConfigParser parser;
	parser.parseFile("default.conf");

	const std::vector<ServerConfig>& servers = parser.getServers();
	const ServerConfig& srv = servers[0];

	std::cout << "🚨 Error pages configured:" << std::endl;

	for (std::map<int, std::string>::const_iterator it = srv.error_pages.begin();
		 it != srv.error_pages.end(); ++it) {
		std::cout << "   " << it->first << " → " << it->second << std::endl;
	}

	// Test specific error pages
	int expected_errors[] = {404, 500, 403, 405};
	for (size_t i = 0; i < sizeof(expected_errors)/sizeof(expected_errors[0]); ++i) {
		int code = expected_errors[i];
		bool found = (srv.error_pages.find(code) != srv.error_pages.end());
		std::cout << (found ? "✅" : "❌") << " Error page " << code << " configured" << std::endl;
	}
}

void testConfigPrint() {
	std::cout << "\n=== TEST 6: CONFIG PRETTY PRINT ===" << std::endl;

	ConfigParser parser;
	parser.parseFile("default.conf");

	// This calls your printConfig() method
	parser.printConfig();
}

int main() {
	std::cout << "🧪 TESTING CONFIG PARSER STANDALONE" << std::endl;
	std::cout << "====================================" << std::endl;

	testBasicParsing();
	testServerDetails();
	testLocations();
	testLocationFinding();
	testErrorPages();
	testConfigPrint();

	std::cout << "\n🎉 ALL TESTS COMPLETED!" << std::endl;
	std::cout << "If you see ✅ for most tests, your parsing is working correctly." << std::endl;

	return 0;
}
