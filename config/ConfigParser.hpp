#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include "ServerConfig.hpp"


// Convierte archivos .conf → estructuras C++ utilizables
// Es el ÚNICO punto de contacto para configuración
class ConfigParser
{
	private:
		std::vector<ServerConfig> _servers;
		bool _is_parsed;

	public:
		ConfigParser();
		~ConfigParser();

		// Main method that other parts will use
		bool parseFile(const std::string &filename);

		// Getters needed by Part A (Sockets) and Part C (Request/Response)
		const std::vector<ServerConfig> &getServers() const; // Part A: socket creation, Part C: request routing
		bool isValid() const;								 // Part A & C: validation before using config

		// For debugging during development
		void printConfig() const;

	private:
		// Private methods for parsing (you'll implement later)
		bool parseServerBlock(std::ifstream &file, ServerConfig &server);
		bool parseLocationBlock(std::ifstream &file, LocationConfig &location);

		// Utility functions
		std::string trim(const std::string& str);                    // Quita espacios
		std::vector<std::string> split(const std::string& str, char delimiter);  // Divide strings
		bool isValidMethod(const std::string& method);               // Part C: HTTP method validation
		bool isValidPort(int port);                                  // Part A: port validation for bind()
		std::string removeInlineComment(const std::string& str);	// Quita comentarios en línea
		bool validateConfig();
		bool isValidIPAddress(const std::string& ip);
		bool isValidHost(const std::string& host);
};

#endif
