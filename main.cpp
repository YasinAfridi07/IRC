/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mayan <mayan@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 19:18:17 by mayan             #+#    #+#             */
/*   Updated: 2024/10/01 19:20:30 by mayan            ###   ########.fr       */
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

// #include "Server.hpp"
// #include "Channel.hpp"
// #include <iostream>
// #include <string>
// #include <cstring>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <vector>
// #include <cstdlib>
// #include <ctime>

// #define BUFFER_SIZE 512

// // Magic 8-ball responses
// std::vector<std::string> responses = {
//     "It is certain.",
//     "It is decidedly so.",
//     "Without a doubt.",
//     "Yes – definitely.",
//     "You may rely on it.",
//     "As I see it, yes.",
//     "Most likely.",
//     "Outlook good.",
//     "Yes.",
//     "Signs point to yes.",
//     "Reply hazy, try again.",
//     "Ask again later.",
//     "Better not tell you now.",
//     "Cannot predict now.",
//     "Concentrate and ask again.",
//     "Don't count on it.",
//     "My reply is no.",
//     "My sources say no.",
//     "Outlook not so good.",
//     "Very doubtful."
// };

// std::string getRandomResponse() {
//     return responses[rand() % responses.size()];
// }

// int main(int ac, char **av)
// {
//     if (ac < 4) {
//         std::cerr << "Usage: " << av[0] << " <port> <password> <channel>" << std::endl;
//         return 1;
//     }

//     Check(ac);
//     int port_num = std::atoi(av[1]);
//     std::string port(av[1]), password(av[2]), channel(av[3]);
//     valid_arg(port, password, port_num);

//     try
//     {
//         Server::_port = port_num;
//         Server::_password = password;
//         Server::openSocket();
//         Server::run();
//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << "Exception: " << e.what() << RESET << '\n';
//     }

//     srand(time(NULL));  // Seed for random number generation

//     // Create a socket
//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock == -1) {
//         std::cerr << "Error creating socket" << std::endl;
//         return 1;
//     }

//     // Server address setup
//     struct sockaddr_in server_addr;
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(port_num);
//     inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

//     // Connect to the IRC server
//     if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
//         std::cerr << "Error connecting to server" << std::endl;
//         return 1;
//     }

//     // Send NICK and USER commands to register the bot
//     std::string nick_cmd = "NICK Magic8Bot\r\n";
//     std::string user_cmd = "USER Magic8Bot 0 * :Magic8Bot\r\n";
//     send(sock, nick_cmd.c_str(), nick_cmd.length(), 0);
//     send(sock, user_cmd.c_str(), user_cmd.length(), 0);

//     // Join the channel
//     std::string join_cmd = "JOIN " + channel + "\r\n";
//     send(sock, join_cmd.c_str(), join_cmd.length(), 0);

//     char buffer[BUFFER_SIZE];

//     // Main loop to listen and respond to messages
//     while (true) {
//         memset(buffer, 0, BUFFER_SIZE);
//         int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);

//         if (bytes_received > 0) {
//             std::string response(buffer);

//             // Respond to PING messages to keep the bot alive
//             if (response.find("PING") != std::string::npos) {
//                 std::string pong = "PONG " + response.substr(response.find("PING") + 5) + "\r\n";
//                 send(sock, pong.c_str(), pong.length(), 0);
//             }

//             // Look for a question in the form of "!8ball <question>"
//             if (response.find("PRIVMSG") != std::string::npos && response.find("!8ball") != std::string::npos) {
//                 std::string msg = "PRIVMSG " + channel + " :" + getRandomResponse() + "\r\n";
//                 send(sock, msg.c_str(), msg.length(), 0);
//             }
//         }
//     }

//     // Close the socket when done
//     close(sock);
//     return 0;
// }