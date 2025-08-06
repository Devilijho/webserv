#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>

// Foward declaration
struct LocationConfig;

struct ServerConfig
{
	// Data needed by Part A (sockets)
	int port;		  // Port number for the server
	std::string host; // Hostname or IP address

	// Data needed by Part C (request handling)
	std::string root;						// Root directory for serving files
	std::string index;						// Default index file
	size_t client_max_body_size;			// Upload limit for client requests
	std::map<int, std::string> error_pages; // Custom error pages: 404 -> "./errors/404.html"

	// Additional configurations
	std::string server_name;			   // Server name
	std::vector<LocationConfig> locations; // List of location configurations

	// Constructor with default values
	ServerConfig();

	// Critical methods for Part C
	const LocationConfig *findLocation(const std::string &path) const;
};

struct LocationConfig
{
	// Data needed by Part C (request handling)
	std::string path;				  // "/" "/upload" "/api"
	std::vector<std::string> methods; // "GET", "POST", "DELETE"
	std::string root;				  // Overrides server root for this location
	std::string index;				  // Overrides server index for this location
	std::string upload_dir;			  // Directory for file uploads
	bool autoindex;					  // Enable/disable directory listing

	// For CGI (Part C)
	std::string cgi_extension; // .php, .py, etc.
	std::string cgi_path;	   // Path to the CGI executable

	LocationConfig();
};

#endif
