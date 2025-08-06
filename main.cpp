#include "CGI/CGI.hpp"

int	main(void)
{
	int returnValue = htpp_request();
	std::cout << "REQUEST RETURN VALUE : " << returnValue << std::endl;
	return (0);
}
