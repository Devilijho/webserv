

#include "CGI/CGIHandler.hpp"
#include "config/ConfigParser.hpp"
#include "server.hpp"

int	main(void)
{
	Server server(8080);
	ConfigParser defaultConfig;

	if (!server.start()) {
		return 1;
	}

	htpp_request();
	return (0);
}
