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
		std::cerr << RED << "Usage: ./ircserv [port] [PASS]" << RESET << std::endl;
		exit(0);
	}
}
void	valid_arg(std::string a, std::string b, int c)
{
	if (a.empty() || b.empty() || c > MAX_PORT || a.length() > 5 || a.find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << RED << "Error: invalid arguments !" << RESET << std::endl;
		exit(0);
	}
}

void Server::openSocket()
{
	int	opt;

	if ((Server::serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		throw ServerException(RED "Failed to create socket" RESET);
	}
	opt = 1;
	if (setsockopt(Server::serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
			sizeof(opt)) < 0)
	{
		throw ServerException(RED "setsockopt failed" RESET);
	}
	Server::address.sin_family = AF_INET;
	Server::address.sin_addr.s_addr = INADDR_ANY;
	Server::address.sin_port = htons(_port);
	if (bind(Server::serverSocket, (struct sockaddr *)&Server::address,
			sizeof(Server::address)) < 0)
	{
		throw ServerException(RED "Bind failed" RESET);
	}
	if (listen(Server::serverSocket, MAX_CLIENTS) < 0)
	{
		throw ServerException(RED "Listen failed" RESET);
	}
	addrlen = sizeof(Server::address);
	gethostname(c_hostName, MAX_HOST_NAME);
	Server::_hostName = c_hostName;
	std::cout << UNDERLINE << GREEN << BOLD << "IRC Server started on port " << _port << " : " << _hostName << RESET << std::endl;
	std::cout << BLUE << BOLD << "Waiting for incoming connections..." << RESET << std::endl;
}

//This function call will block until a client connects, at which point it 
//returns a new socket descriptor for the connection, allowing you to communicate with that client.
void Server::acceptConnection() 
{
    // Assume newSocket is the new socket descriptor for the connected client
    int newSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    
    if (newSocket < 0) {
        std::cerr << "Failed to accept connection" << std::endl;
        return;
    }

    // Create a new User object and initialize its FD
    User newUser;
    newUser._fd = newSocket; // Assign the file descriptor
    newUser._nickname = ""; // Initialize nickname or other user attributes as needed

    // Add the new user to the users vector
    users.push_back(newUser);
    _fds.push_back(newSocket); // Also add the socket to the file descriptor list
}

// Обработка отключения клиента
void Server::handleClientDisconnection(size_t i)
{
	getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	std::cout << YELLOW << "Client disconnected" << RESET << std::endl;
	close(sd);
	_fds.erase(_fds.begin() + i);   // Удаляем сокет из вектора
	users.erase(users.begin() + i); // Удаляем пользователя из списка
}



void User::execute(std::string mes, User *user) {
    std::vector<std::string> splitmsg = split(mes);
    if (splitmsg.empty()) {
        return;
    }
    
    Command cmd; // Create a Command object
    std::string cmdType = splitmsg.at(0);
    if (cmdType == "JOIN") {
        if (splitmsg.size() == 2) {
            cmd.ajoin(splitmsg.at(1), "", *user);            
        } else if (splitmsg.size() == 3) {
            cmd.ajoin(splitmsg.at(1), splitmsg.at(2), *user);
        } else {
            return;
        }
    } else if (cmdType == "WHO") {   
        if (splitmsg.size() == 2) {
            cmd.who(splitmsg.at(1), *user);
        } else {
            return;
        }
    } else if (cmdType == "PRIVMSG" || cmdType == "MSG") {
        if (splitmsg.size() >= 3) {
            cmd.privmsg(splitmsg.at(1), splitmsg, *user);
        } else if (splitmsg.size() == 2) {
            ErrorMsg(user->_fd, "411 :No recipient given\r\n", "411");
        } else if (splitmsg.size() == 1) {
            ErrorMsg(user->_fd, "411 :No recipient given\r\n", "411");
        } else {
            std::string S = "461 :Not enough parameters\r\n";
            send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
            return;
        }
    } else if (cmdType == "PING") {
        std::string pong = "PONG\r\n";
        send(user->_fd, pong.c_str(), pong.length(), 0);
    } else if (cmdType == "INVITE") {
        if (splitmsg.size() == 3) {
            cmd.invite(splitmsg.at(1), splitmsg.at(2), *user);
        } else {
            ErrorMsg(user->_fd, "461 :INVITE command requires 3 arguments\r\n", "461");
        }
    } else if (cmdType == "KICK") {
        if (splitmsg.size() >= 3) {
            cmd.kick(splitmsg.at(1), splitmsg.at(2), splitmsg, *user);
        } else {
            ErrorMsg(user->_fd, "461 :KICK command requires 3 or 4 arguments\r\n", "461");
        }
    } else if (cmdType == "MODE") {
        if (splitmsg.size() == 4) {
            cmd.mode(splitmsg.at(1), splitmsg.at(2), *user, splitmsg.at(3));
        } else if (splitmsg.size() == 3) {
            cmd.mode(splitmsg.at(1), splitmsg.at(2), *user, "");
        } else {
            ErrorMsg(user->_fd, "461 :Not enough parameters\r\n", "461");
        }
    }
    else if (cmdType == "TOPIC")
	{
		if (splitmsg.size() == 3)
		{
			cmd.topic(splitmsg.at(1), splitmsg.at(2), *user);
		}
		else if (splitmsg.size() == 2)
		{
			cmd.topic(splitmsg.at(1), "", *user);
		}
		else
		{
			ErrorMsg(user->_fd, "TOPIC command requires 1 or 2 arguments\n", "461");
		}
	}
    else if(cmdType == "CAP")
    {
        std::string firstServerMsg = ":irc CAP * ACK multi-prefix\r\n";
        send(user->_fd, firstServerMsg.c_str(), firstServerMsg.length(), 0);
    } 
    else if (cmdType != "NICK" && cmdType != "PASS" && cmdType != "USER" && cmdType != "CAP") {
        ErrorMsg(user->_fd, "421 :Unknown command\r\n", "421");
    }
    else if(cmdType == "NICK" || cmdType == "PASS" || cmdType == "USER")
    {
        if (user->cap == 0) 
        {
        std::string wlcmMsg = ":irc 001 " + user->_nickname + " :Welcome to FT_IRC, " + user->_username + "@" + Server::_hostName + "\r\n"
                      ":irc 002 " + user->_nickname + " :Your host is " + Server::_hostName + ", running version 1.0\r\n"
                      ":irc 003 " + user->_nickname + " :This server was created in 42 labs at July\r\n"
                      ":irc 004 " + user->_nickname + " " + Server::_hostName + " 1.0 o OIRSv o oirwkl bkl\r\n"  // Sample modes
                      ":irc 005 " + user->_nickname + " MODES=20 CHANLIMIT=#&:100 NICKLEN=9 TOPICLEN=390 CHANTYPES=#& :are supported by this server\r\n";
        send(user->_fd, wlcmMsg.c_str(), wlcmMsg.length(), 0);
            user->cap = 1;
        };
    }
}



void printSplitMsg(const std::vector<std::string>& splitmsg) {
    std::cout << "Split Message:" << std::endl;
    for (size_t i = 0; i < splitmsg.size(); ++i) {
        std::cout << splitmsg[i] << "," << std::endl;
    }
}

void Server::handleClientMessages() {
    for (size_t i = 0; i < _fds.size(); i++) {
        sd = _fds[i];
        if (FD_ISSET(sd, &readfds)) {
            if ((valread = read(sd, c_buffer, BUFFER_SIZE)) == 0) {
                handleClientDisconnection(i); // Handle disconnection
            } else {
                c_buffer[valread - 1] = '\0'; // Correctly null-terminate the string
                std::string message(c_buffer);
                std::vector<std::string> splitmsg = split(c_buffer);

                // Process commands
                for (size_t j = 0; j < splitmsg.size(); j++) {
                    if (splitmsg[j] == "NICK" && splitmsg.size() > 1) {
                        std::string nick = splitmsg[j + 1];
                        if (isNicknameTaken(nick)) {
                            // Disconnect user if nickname is already taken
                            std::string errorMsg = ":irc 433 " + nick + " :Nickname is already in use\r\n";
                            send(sd, errorMsg.c_str(), errorMsg.length(), 0);
                            handleClientDisconnection(i); // Disconnect user
                            return; // Exit early
                        }
                        users[i]._nickname = nick; // Save nickname
                        users[i].nick_flag = 1;
                    }

                    if (splitmsg[j] == "USER" && splitmsg.size() > 1) {
                        std::string user = splitmsg[j + 1];
                        if (isUsernameTaken(user)) {
                            // Disconnect user if username is already taken
                            std::string errorMsg = ":irc 433 " + user + " :Username is already in use\r\n";
                            send(sd, errorMsg.c_str(), errorMsg.length(), 0);
                            handleClientDisconnection(i); // Disconnect user
                            return; // Exit early
                        }
                        users[i].setUser(user); // Save username
                        users[i].user_flag = 1;
                    }

                    if (splitmsg[j] == "PASS" && splitmsg.size() > 1) {
                        std::string pass = splitmsg[j + 1];
                        users[i]._password = pass; // Save password
                        users[i].pass_flag = 1;
                        if (users[i]._password != Server::_password) {
                            std::string errorMsg = ":irc 464 " + users[i]._nickname + " :Wrong password\r\n";
                            send(sd, errorMsg.c_str(), errorMsg.length(), 0);
                            handleClientDisconnection(i); // Disconnect user
                            return; // Exit early
                        }
                    }
                }

                // Check if user has completed registration
                if (users[i].pass_flag == 1 && users[i].nick_flag == 1 && users[i].user_flag == 1) {
                    users[i].execute(message, &users[i]); // Pass the current user
                }
            }
        }
    }
}


bool Server::isNicknameTaken(const std::string &nick) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i]._nickname == nick) {
            return true;
        }
    }
    return false;
}

bool Server::isUsernameTaken(const std::string &user) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i]._username == user) {
            return true;
        }
    }
    return false;
}


void Server::run() 
{
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        max_sd = serverSocket;
        for (size_t i = 0; i < _fds.size(); i++) {
            sd = _fds[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            std::cerr << "Select error" << std::endl;
        }
        if (FD_ISSET(serverSocket, &readfds)) {
            acceptConnection();
        }
        handleClientMessages();
    }
}
// line 199 put "serverSocket" and "&readfds" in server class
// for Tim ^^^
//hi


/* 

NOTE: look the actual doc for irc: https://modern.ircdocs.horse

1. CAP command
2. FINISH the 5 mandatory welcome name-replays
3. send Message of the Day (MOTD)
 */