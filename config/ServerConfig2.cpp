#include "ServerConfig2.hpp"

// ServerConfig constructor with default values
ServerConfig::ServerConfig()
	: port(8080),
	  host("localhost"),
	  server_name(""),
	  root("./www"),
	  index("index.html"),
	  client_max_body_size(1000000)
{
	// locations vector se auto-inicializa vacío
	// error_pages map se auto-inicializa vacío
}

// Critical method for Part C - finds the best matching location for a given path
const LocationConfig *ServerConfig::findLocation(const std::string &path) const
{
	const LocationConfig *bestMatch = NULL;
	size_t longestMatch = 0;

	// Find the most specific location that matches the path
	 for (std::vector<LocationConfig*>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        const LocationConfig* loc = *it;  // dereference pointer
        if (path.find(loc->path) == 0 && loc->path.length() > longestMatch)
        {
            bestMatch = loc;
            longestMatch = loc->path.length();
        }
    }

	return bestMatch;
}

// LocationConfig constructor with default values
LocationConfig::LocationConfig()
	: path(""),                    // ← Inicializar explícitamente
	  root(""),                    // ← Inicializar explícitamente
	  index(""),                   // ← Inicializar explícitamente
	  autoindex(false),            // ← Ya estaba bien
	  upload_dir(""),              // ← Inicializar explícitamente
	  cgi_extension(""),           // ← Inicializar explícitamente
	  cgi_path(""),                 // ← Inicializar explícitamente
	  client_max_body_size(0) // ← ¡CRÍTICO! Estaba sin inicializar
{
	// methods vector se auto-inicializa vacío
}
