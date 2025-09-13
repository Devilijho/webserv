#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <map>
#include <algorithm>

#include "ServerConfig2.hpp"

// // Forward declarations de estructuras
// struct LocationConfig;
// struct ServerConfig;

class ConfigParser {
private:
	std::vector<ServerConfig*> _servers;
	bool _is_parsed;

	// FUNCIONES PRINCIPALES
	bool finalizeConfig();

	// PARSING DE BLOQUES
	bool parseServerBlock(std::ifstream &file, ServerConfig* server);
	bool parseLocationBlock(std::ifstream &file, LocationConfig* location);

	// DISPATCHERS
	bool parseServerDirective(const std::vector<std::string>& tokens, ServerConfig* server);
	bool parseLocationDirective(const std::vector<std::string>& tokens, LocationConfig* location);

	// PARSING SERVER DIRECTIVES
	bool parseServerPortDirective(const std::vector<std::string>& tokens, ServerConfig* server);
	bool parseServerHostDirective(const std::vector<std::string>& tokens, ServerConfig* server);
	bool parseServerRootDirective(const std::vector<std::string>& tokens, ServerConfig* server);
	bool parseServerIndexDirective(const std::vector<std::string>& tokens, ServerConfig* server);
	bool parseServerNameDirective(const std::vector<std::string>& tokens, ServerConfig* server);
	bool parseServerClientMaxBodySizeDirective(const std::vector<std::string>& tokens, ServerConfig* server);
	bool parseServerErrorPageDirective(const std::vector<std::string>& tokens, ServerConfig* server);
	bool parseServerDefaultErrorPageDirective(const std::vector<std::string>& tokens, ServerConfig* server);

	// PARSING LOCATION DIRECTIVES
	bool parseLocationHeader(const std::string& line, LocationConfig& location);
	bool parseLocationMethodsDirective(const std::vector<std::string>& tokens, LocationConfig* location);
	bool parseLocationRootDirective(const std::vector<std::string>& tokens, LocationConfig& location);
	bool parseLocationIndexDirective(const std::vector<std::string>& tokens, LocationConfig& location);
	bool parseLocationAutoindexDirective(const std::vector<std::string>& tokens, LocationConfig& location);
	bool parseLocationUploadDirDirective(const std::vector<std::string>& tokens, LocationConfig& location);
	bool parseLocationCgiExtensionDirective(const std::vector<std::string>& tokens, LocationConfig& location);
	bool parseLocationCgiPathDirective(const std::vector<std::string>& tokens, LocationConfig& location);
	bool parseLocationClientMaxBodySizeDirective(const std::vector<std::string>& tokens, LocationConfig& location);
	bool parseLocationReturnDirective(const std::vector<std::string>& tokens, LocationConfig& location);


	// VALIDACIÓN MODULAR
	bool validateServer(size_t serverIndex, const ServerConfig* srv);
	bool validateServerBasics(size_t serverIndex, const ServerConfig* srv);
	bool validateServerFiles(size_t serverIndex, const ServerConfig* srv);
	bool validateServerLocations(size_t serverIndex, const ServerConfig* srv);
	bool validateLocation(size_t serverIndex, const ServerConfig* srv, size_t locationIndex, const LocationConfig& loc);
	bool validateLocationFiles(size_t serverIndex, const ServerConfig* srv, const LocationConfig& loc);
	bool validateDuplicateServers();

	// UTILIDADES
	std::string trim(const std::string& str);
	std::vector<std::string> split(const std::string& str, char delimiter);
	std::string removeInlineComment(const std::string& str);
	bool isValidMethod(const std::string& method);
	bool isValidPort(int port);
	bool isValidIPAddress(const std::string& ip);
	bool isValidHost(const std::string& host);

	// Validación básica de hostnames (sin wildcards)
	bool isValidHostname(const std::string& hostname);
	bool isValidHostnameLabel(const std::string& label);
	bool containsOnlyValidChars(const std::string& str);

	bool isValidClientMaxBodySize(long size);
    bool validateAndSetClientMaxBodySize(const std::string& value, size_t& target, const std::string& context);
    bool validateAndSetPort(const std::string& value, int& target);
    bool validateAndSetHost(const std::string& value, std::string& target);


public:
	ConfigParser();
	~ConfigParser();

	// FUNCIONES PÚBLICAS
	bool parseFile(const std::string &filename);
	bool validateConfig();
	const std::vector<ServerConfig*> &getServers() const;
	bool isValid() const;
	void printConfig() const;
};

#endif
