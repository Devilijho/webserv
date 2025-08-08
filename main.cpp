#include "RequestHandler/RequestHandler.hpp"
#include "config/ConfigParser.hpp"
#include "config/ServerConfig.hpp"
#include "server.hpp"

int	main(void)
{
	Server server(8080);
	ConfigParser parserConfig;
	ServerConfig serverConfig;

	// if (!server.start()) {
	// 	return 1;
	// }

	std::cout << "request return value : " << htpp_request(serverConfig) << std::endl;
	return (0);
}
