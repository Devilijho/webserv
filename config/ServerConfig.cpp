#include "ServerConfig.hpp"

// ServerConfig constructor with default values
ServerConfig::ServerConfig()
	: port(8080),
	  host("localhost"),
	  root("./www"),
	  index("index.html"),
	  client_max_body_size(1000000)
{}

// Critical method for Part C - finds the best matching location for a given path
const LocationConfig *ServerConfig::findLocation(const std::string &path) const
{
	const LocationConfig *bestMatch = nullptr;
	size_t longestMatch = 0;

	// Find the most specific location that matches the path
	for (const LocationConfig &loc : locations)
	{
		if (path.find(loc.path) == 0 && loc.path.length() > longestMatch)
		{
			bestMatch = &loc;
			longestMatch = loc.path.length();
		}
	}

	return bestMatch;
}

// LocationConfig constructor with default values
LocationConfig::LocationConfig() : autoindex(false) {}
