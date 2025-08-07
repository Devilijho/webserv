#include "CGI/CGIHandler.hpp"

// int	main(void)
// {
// 	int returnValue = htpp_request();
// 	std::cout << "REQUEST RETURN VALUE : " << returnValue << std::endl;
// 	return (0);
// }
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pde-vara <pde-vara@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/06 12:01:59 by pde-vara          #+#    #+#             */
/*   Updated: 2025/08/06 15:12:52 by pde-vara         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"

int main()
{
    // 1. Create a server instance listening on port 8080
    Server server(8080);

    // 2. Start the server
    if (!server.start()) {
        return 1; // Something went wrong
    }

    return 0;
}
