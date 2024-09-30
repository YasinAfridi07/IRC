#include "Channel.hpp"
#include "Server.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

// Initialize static member variables of the Server class
std::string Server::_password = ""; // Server password
std::string Server::bufferStr = "";
// Buffer string for message handling
std::string Server::_hostName = ""; // Hostname of the server
char Server::c_buffer[BUFFER_SIZE] = {0};
// Character buffer for reading messages
char Server::c_hostName[MAX_HOST_NAME] = {0};
// Character buffer for storing hostname
int Server::serverSocket = -1; // Server socket descriptor
int Server::max_sd = -1;
// Maximum socket descriptor for select()
int Server::sd = -1;      // Current socket descriptor
int Server::valread = -1; // Value read from socket
int Server::_port = -1;   // Server port number
int Server::newSocket = -1;
// New socket descriptor for accepted connections
int Server::curIndex = -1;
// Current index for handling clients
int Server::addrlen = sizeof(struct sockaddr_in);
// Length of the address structure
std::vector<int> Server::_fds;
// Vector of socket descriptors for connected clients
std::vector<User> Server::users;
// Vector of User objects representing connected clients
struct sockaddr_in Server::address; // Server address structure
fd_set Server::readfds;
// Set of socket descriptors for select()
std::vector<Channel> Server::_channels;

void	Check(int ac)
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv [port] [PASS]" << std::endl;
		exit(0);
	}
}
void	valid_arg(std::string a, std::string b, int c)
{
	if (a.empty() || b.empty() || c > MAX_PORT || a.length() > 5
		|| a.find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << "Error: invalid arguments !" << std::endl;
		exit(0);
	}
}

void Server::openSocket()
{
	int	opt;

	if ((Server::serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		throw ServerException("Failed to create socket");
	}
	opt = 1;
	if (setsockopt(Server::serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
			sizeof(opt)) < 0)
	{
		throw ServerException("setsockopt failed");
	}
	Server::address.sin_family = AF_INET;
	Server::address.sin_addr.s_addr = INADDR_ANY;
	Server::address.sin_port = htons(_port);
	if (bind(Server::serverSocket, (struct sockaddr *)&Server::address,
			sizeof(Server::address)) < 0)
	{
		throw ServerException("Bind failed");
	}
	if (listen(Server::serverSocket, MAX_CLIENTS) < 0)
	{
		throw ServerException("Listen failed");
	}
	addrlen = sizeof(Server::address);
	gethostname(c_hostName, MAX_HOST_NAME);
	Server::_hostName = c_hostName;
	std::cout << "IRC Server started on port " << Server::_port << " : " << _hostName << std::endl;
	std::cout << "Waiting for incoming connections..." << RESET << std::endl;
}

//This function call will block until a client connects, at which point it
//returns a new socket descriptor for the connection,
// allowing you to communicate with that client.
void Server::acceptConnection()
{
	int		newSocket;
	User	newUser;

	// Assume newSocket is the new socket descriptor for the connected client
	newSocket = accept(serverSocket, (struct sockaddr *)&address,
			(socklen_t *)&addrlen);
	if (newSocket < 0)
	{
		std::cerr << "Failed to accept connection" << std::endl;
		return ;
	}
	// Create a new User object and initialize its FD
	newUser._fd = newSocket; // Assign the file descriptor
	newUser._nickname = "";
	// Initialize nickname or other user attributes as needed
	// Add the new user to the users vector
	users.push_back(newUser);
	_fds.push_back(newSocket);
	// Also add the socket to the file descriptor list
}

// Обработка отключения клиента
void Server::handleClientDisconnection(size_t i)
{
	getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	std::cout << "Client disconnected" << std::endl;
	close(sd);
	_fds.erase(_fds.begin() + i);   // Удаляем сокет из вектора
	users.erase(users.begin() + i); // Удаляем пользователя из списка
}

// Обработка команды USER
void Server::handleUserCommand(size_t i, const std::string &message)
{
	const char	*errorMsg;

	if (!users[i].getUser().empty())
	{
		std::cerr << "Error: USER already set and cannot be changed" << std::endl;
		errorMsg = "Error: USER already set and cannot be changed.\n";
		send(sd, errorMsg, strlen(errorMsg), 0);
	}
	else
	{
		std::string user = message.substr(5);
		// Извлекаем имя пользователя после "USER "
		users[i].setUser(user);
		std::cout << "Set USER: " << user;
	}
}

// Обработка команды NICK
void Server::handleNickCommand(size_t i, const std::string &message)
{
	const char	*errorMsg;

	if (!users[i].getNick().empty())
	{
		std::cerr << "Error: NICK already set and cannot be changed" << std::endl;
		errorMsg = "Error: NICK already set and cannot be changed.\n";
		send(sd, errorMsg, strlen(errorMsg), 0);
	}
	else
	{
		std::string nick = message.substr(5); // Извлекаем никнейм после "NICK "
		users[i].setNick(nick);
		std::cout << "Set NICK: " << nick;
	}
}

// Обработка команды PASS
void Server::handlePassCommand(size_t i, const std::string &message)
{
	const char	*errorMsg;

	if (!users[i].getPass().empty())
	{
		std::cerr << "Error: PASS already set and cannot be changed" << std::endl;
		errorMsg = "Error: PASS already set and cannot be changed.\n";
		send(sd, errorMsg, strlen(errorMsg), 0);
	}
	else
	{
		std::string pass = message.substr(5);
		// Извлекаем пароль после "PASS "
		pass.erase(0, pass.find_first_not_of(" \n\r\t"));
		// Удаляем пробелы в начале
		pass.erase(pass.find_last_not_of(" \n\r\t") + 1);
		// Удаляем пробелы в конце
		if (pass != _password)
		{
			std::cerr << "Error: invalid password" << std::endl;
			errorMsg = "Error: invalid password.\n";
			send(sd, errorMsg, strlen(errorMsg), 0);
		}
		else
		{
			users[i].setPass(pass);
			std::cout << "Set PASS: " << pass << std::endl;
		}
	}
}

// Проверка, авторизован ли пользователь
bool Server::isUserAuthorized(size_t i)
{
	const char	*errorMsg;

	if (users[i].getNick().empty() || users[i].getUser().empty()
		|| users[i].getPass().empty())
	{
		std::cerr << "Error: user not authorized, missing USER, NICK, or PASS " << std::endl;
			errorMsg = "Error: user not authorized. Please provide USER, NICK, and PASS.\n ";
				send(sd, errorMsg, strlen(errorMsg), 0);
		return (false);
	}
	return (true);
}

void Server::handleClientMessages()
{
	for (size_t i = 0; i < _fds.size(); i++)
	{
		sd = _fds[i];
		if (FD_ISSET(sd, &readfds))
		{
			if ((valread = read(sd, c_buffer, BUFFER_SIZE)) == 0)
			{
				handleClientDisconnection(i); // Handle disconnection
			}
			else
			{
				c_buffer[valread - 1] = '\0';
				// Correctly null-terminate the string
				std::string message(c_buffer);
				// Process commands based on the received message
				if (message.substr(0, 4) == "USER")
				{
					handleUserCommand(i, message);
				}
				else if (message.substr(0, 4) == "NICK")
				{
					handleNickCommand(i, message);
				}
				else if (message.substr(0, 4) == "PASS")
				{
					handlePassCommand(i, message);
				}
				else if (isUserAuthorized(i))
				{
					std::cout << users[i]._fd << std::endl;
					users[i].process_cmd(message, &users[i]);
					// Pass the current user
				}
			}
		}
	}
}

void Server::run()
{
	int	activity;

	while (true)
	{
		FD_ZERO(&readfds);
		FD_SET(serverSocket, &readfds);
		max_sd = serverSocket;
		for (size_t i = 0; i < _fds.size(); i++)
		{
			sd = _fds[i];
			if (sd > 0)
			{
				FD_SET(sd, &readfds);
			}
			if (sd > max_sd)
			{
				max_sd = sd;
			}
		}
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		// poll() equivalent, used for handling I/O operations
		if ((activity < 0) && (errno != EINTR))
		{
			std::cerr << "Select error" << std::endl;
		}
		if (FD_ISSET(serverSocket, &readfds))
		{
			acceptConnection();
		}
		handleClientMessages();
	}
}
// line 199 put "serverSocket" and "&readfds" in server class
// for Tim ^^^
//hi
