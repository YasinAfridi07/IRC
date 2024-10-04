/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tzhakypo <tzhakypo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 19:18:17 by mayan             #+#    #+#             */
/*   Updated: 2024/10/04 18:13:46 by tzhakypo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"

int main(int ac, char **av)
{

    Check(ac);
    int port_num = std::atoi(av[1]);
	std::string	port(av[1]), password(av[2]);
	valid_arg(port, password, port_num);
	try
	{
		Server::_port = port_num;
		Server::_password = password;
		Server::openSocket();
		Server::run();
	} catch(const std::exception& e) {
		std::cerr << "Exception: " << e.what() << RESET << '\n';

	}
}

