#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "Server.hpp"
#include "Channel.hpp"

std::string Server::_password = "";
std::string Server::bufferStr = "";
std::string Server::_hostName = "";
char Server::c_buffer[BUFFER_SIZE]= {0};
char Server::c_hostName[MAX_HOST_NAME] = {0};
int Server::serverSocket = -1;
int Server::max_sd = -1;
int Server::sd = -1;
int Server::valread = -1;
int Server::_port = -1;
int Server::newSocket = -1;
int Server::curIndex = -1;
int Server::addrlen = sizeof(struct sockaddr_in);
std::vector<int> Server::_fds;
std::vector<User> Server::users;
struct sockaddr_in Server::address;
fd_set Server::readfds;

void Check(int ac)
{
    if (ac != 3) {
		std::cerr << "Usage: ./ircserv [port] [PASS]" << std::endl;
		exit(0);
	}
}
void valid_arg(std::string a, std::string b, int c)
{
    if (a.empty() || b.empty() || c > MAX_PORT \
		|| a.length() > 5 || a.find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << "Error: invalid arguments !" << std::endl;
		exit(0);
	}
}

void Server::openSocket() {
    if ( (Server::serverSocket = socket( AF_INET, SOCK_STREAM, 0) ) == 0 ) {
        throw ServerException( "Failed to create socket" );
    }

    int opt = 1;
    if ( setsockopt(Server::serverSocket, SOL_SOCKET, SO_REUSEADDR, ( char * )&opt, sizeof( opt )) < 0 ) {
        throw ServerException( "setsockopt failed" );
    }

    Server::address.sin_family = AF_INET;
    Server::address.sin_addr.s_addr = INADDR_ANY;
    Server::address.sin_port = htons(_port);

    if ( bind(Server::serverSocket, ( struct sockaddr * )&Server::address, sizeof( Server::address )) < 0 ) {
        throw ServerException( "Bind failed" );
    }

    if ( listen(Server::serverSocket, MAX_CLIENTS) < 0 ) {
        throw ServerException( "Listen failed" );
    }

    addrlen = sizeof( Server::address );

    gethostname( c_hostName, MAX_HOST_NAME );
    Server::_hostName = c_hostName;
    std::cout << "IRC Server started on port " << Server::_port << " : " << _hostName << std::endl;
    std::cout << "Waiting for incoming connections..." << RESET << std::endl;
}


void Server::acceptConnection() {
    if ((newSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        throw ServerException("Accept failed");
    }

    _fds.push_back(newSocket);

    // Добавляем нового пользователя при подключении
    users.push_back(User());

    std::cout << "New connection, socket " << _port << std::endl;
}

// Обработка отключения клиента
void Server::handleClientDisconnection(size_t i) {
    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    std::cout << "Client disconnected" << std::endl;

    close(sd);
    _fds.erase(_fds.begin() + i);  // Удаляем сокет из вектора
    users.erase(users.begin() + i); // Удаляем пользователя из списка
}

// Обработка команды USER
void Server::handleUserCommand(size_t i, const std::string& message)
{
    if (!users[i].getUser().empty()) {
        std::cerr << "Error: USER already set and cannot be changed" << std::endl;
        const char *errorMsg = "Error: USER already set and cannot be changed.\n";
        send(sd, errorMsg, strlen(errorMsg), 0);
    }
	else
	{
        std::string user = message.substr(5); // Извлекаем имя пользователя после "USER "
        users[i].setUser(user);
        std::cout << "Set USER: " << user;
    }
}

// Обработка команды NICK
void Server::handleNickCommand(size_t i, const std::string& message)
{
    if (!users[i].getNick().empty())
	{
        std::cerr << "Error: NICK already set and cannot be changed" << std::endl;
        const char *errorMsg = "Error: NICK already set and cannot be changed.\n";
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
void Server::handlePassCommand(size_t i, const std::string& message) {
    if (!users[i].getPass().empty()) {
        std::cerr << "Error: PASS already set and cannot be changed" << std::endl;
        const char *errorMsg = "Error: PASS already set and cannot be changed.\n";
        send(sd, errorMsg, strlen(errorMsg), 0);
    } else {
        std::string pass = message.substr(5); // Извлекаем пароль после "PASS "
        pass.erase(0, pass.find_first_not_of(" \n\r\t"));  // Удаляем пробелы в начале
        pass.erase(pass.find_last_not_of(" \n\r\t") + 1);  // Удаляем пробелы в конце
        if(pass != _password) {
            std::cerr << "Error: invalid password" << std::endl;
            const char *errorMsg = "Error: invalid password.\n";
            send(sd, errorMsg, strlen(errorMsg), 0);
        } else {
            users[i].setPass(pass);
            std::cout << "Set PASS: " << pass << std::endl;
        }
    }
}

// Проверка, авторизован ли пользователь
bool Server::isUserAuthorized(size_t i) {
    if (users[i].getNick().empty() || users[i].getUser().empty() || users[i].getPass().empty()) {
        std::cerr << "Error: user not authorized, missing USER, NICK, or PASS" << std::endl;
        const char *errorMsg = "Error: user not authorized. Please provide USER, NICK, and PASS.\n";
        send(sd, errorMsg, strlen(errorMsg), 0);
        return false;
    }
    return true;
}
std::vector<std::string> split(const std::string str) {
	std::vector<std::string> vector;
	std::istringstream iss(str);
	std::string cmd;
	while (iss >> std::skipws >> cmd)
		vector.push_back(cmd);
	return vector;
}
void User::execute(std::string cmd, User *user)
{
    std::vector<std::string> splitmsg = split(cmd);
    std::cout << user->_nickname << ":" << cmd << std::endl;
}
void Server::handleClientMessages()
{
    for (size_t i = 0; i < _fds.size(); i++)
    {
        sd = _fds[i];
        std::cout << "sd: " << sd << std::endl;
        if (FD_ISSET(sd, &readfds)) {
            if ((valread = read(sd, c_buffer, BUFFER_SIZE)) == 0)
            {
                handleClientDisconnection(i);
            } else {
                c_buffer[valread - 1] =  '\0';
                std::string message(c_buffer);

                // Обрабатываем команды
                if (message.substr(0, 4) == "USER") {
                    handleUserCommand(i, message);
                } else if (message.substr(0, 4) == "NICK") {
                    handleNickCommand(i, message);
                } else if (message.substr(0, 4) == "PASS") {
                    handlePassCommand(i, message);
                } else if (isUserAuthorized(i)) {
                    users[i].execute(message, &users[i]);
                }
            }
        }
    }
}


void Server::run() {
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
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL); // poll() equivalent, used for handling I/O operations
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



