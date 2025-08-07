#include "CGI/CGIHandler.hpp"
#include "server.hpp"

// int main(void)
// {
// 	return (SUCCESS);
// }

// SANTI MAIN
int	main(void)
{
	int returnValue = htpp_request();
	if (returnValue == 0)
		returnValue = OK;
	std::cout << "REQUEST RETURN VALUE : " << returnValue << std::endl;
	return (0);
}

//PIERRE MAIN
// int main()
// {
//     // 1. Create a server instance listening on port 8080
//     Server server(8080);

//     // 2. Start the server
//     if (!server.start()) {
//         return 1; // Something went wrong
//     }

//     return 0;
// }
