/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mayan <mayan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2024/09/28 17:37:59 by mayan            ###   ########.fr       */
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
    // signal( SIGINT, Utils::signalHandler );
    // signal( SIGQUIT, Utils::signalHandler );
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
