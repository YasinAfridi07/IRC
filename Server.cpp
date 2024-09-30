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
std::vector<Channel> Server::_channels;

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

//This function call will block until a client connects, at which point it 
//returns a new socket descriptor for the connection, allowing you to communicate with that client.
void Server::acceptConnection() {
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
    std::cout << "Handling USER command: " << message << std::endl;
    if (!users[i].getUser().empty()) {
        std::cerr << "Error: USER already set and cannot be changed" << std::endl;
        const char *errorMsg = "Error: USER already set and cannot be changed.\n";
        send(sd, errorMsg, strlen(errorMsg), 0);
    }
	else
	{
        std::string user = message.substr(5); // Извлекаем имя пользователя после "USER "
        users[i].setUser(user);
    }
}
 
// Обработка команды NICK
void Server::handleNickCommand(size_t i, const std::string& message)
{
    std::cout << "Handling NICK command: " << message << std::endl;
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
    }
}

// Обработка команды PASS
void Server::handlePassCommand(size_t i, const std::string& message) {
    std::cout << "Handling PASS command: " << message << std::endl;
    std::string pass = message.substr(5);
    pass.erase(0, pass.find_first_not_of(" \n\r\t"));  // Удаляем пробелы в начале
    pass.erase(pass.find_last_not_of(" \n\r\t") + 1);  // Удаляем пробелы в конце
    users[i].setPass(pass);
    // Проверка установленного пароля
    if (users[i].getPass().empty()) {
        if (strcmp(pass.c_str(), _password.c_str()) == 0) {
            users[i].setPass(pass);
            std::cout << "Set PASS: " << pass << std::endl;
        } else {
            std::cerr << "Error: invalid password" << std::endl;
            const char *errorMsg = "Error: invalid password.\n";
            send(users[i]._fd, errorMsg, strlen(errorMsg), 0);
        }
    } 
}


// Проверка, авторизован ли пользователь
bool Server::isUserAuthorized(size_t i) {
    if (users[i]._nickname.empty()) {
        std::cerr << "Error: user not authorized, missing NICK" << std::endl;
        std::cout << users[i]._nickname << std::endl;
        const char *errorMsg = "Error: user not authorized. Please provide NICK\n";
        send(sd, errorMsg, strlen(errorMsg), 0);
        return false;
    }
    if (users[i]._password.empty()) {
        std::cerr << "Error: user not authorized, missing PASS" << std::endl;
        const char *errorMsg = "Error: user not authorized. Please provide PASS.\n";
        send(sd, errorMsg, strlen(errorMsg), 0);
        return false;
    }
    if (users[i]._username.empty()) {
        std::cerr << "Error: user not authorized, missing USER" << std::endl;
        const char *errorMsg = "Error: user not authorized. Please provide USER.\n";
        send(sd, errorMsg, strlen(errorMsg), 0);
        return false;
    }
    return true;
}

void Command::who(std::string channel_s, User user)
{
    // Find the channel
    std::vector<Channel>::iterator it = channel_exist(channel_s);
    if (it == Server::_channels.end())
    {
        ErrorMsg(user._fd, "Channel does not exist.", "403");
        return;
    }
    // Prepare the user list
    std::string userList = "Users in " + it->getName() + ": ";
    // Get the users in the channel
    std::vector<User> usersInChannel = it->getUsers();
    // Use a traditional for loop to append all user nicknames
    for (std::vector<User>::iterator it_user = usersInChannel.begin(); it_user != usersInChannel.end(); ++it_user) {
        userList += it_user->_nickname + ", "; // Append each user's nickname
    }
    // Trim the trailing space
    if (!userList.empty()) {
        userList.pop_back(); // Remove the last space
    }
    // Send the user list to the user who requested it
    send(user._fd, userList.c_str(), userList.length(), 0);
}


void User::execute(std::string mes, User *user)
{
   std::vector<std::string> splitmsg = split(mes);
    if (splitmsg.empty()) {
        return;
    }
    
    Command cmd; // Create a Command object
    std::string cmdType = splitmsg.at(0);
    if (cmdType == "JOIN") {
        if (splitmsg.size() == 2) 
        {
            cmd.ajoin(splitmsg.at(1), "", *user);            
        } 
        else if (splitmsg.size() == 3)
        {
	        cmd.ajoin(splitmsg.at(1), splitmsg.at(2), *user);
        }
	    else 
        {
		    return ;
        }
    }
    else if(cmdType == "CHANUSER")
    {   
        if (splitmsg.size() == 2)
            cmd.who(splitmsg.at(1), *user);
        else
            return ;
    }
    else if(cmdType == "PRIVMSG" || cmdType == "MSG")
    {
            if (splitmsg.size() >= 3) 
            {
		        cmd.privmsg(splitmsg.at(1), splitmsg, *user); // second argument will be the split message for mutiple words
	        } 
            else if (splitmsg.size() == 2) 
            {
		        // no such nickname, if nickname doesn't exist
		        ErrorMsg(user->_fd, " Min 3 arg required\n", "412");
	        } 
            else if(splitmsg.size() == 1) 
            {
		        ErrorMsg(user->_fd, "Need 2 more arg after PRIVMSG ", "401");
	        }
             else 
            { // if PRIVMSG nickname exist and msg dosent exist
		        std::string S = "461";
		        S.append(" :Not enough parameters\r\n");
		        send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
		        return;
	        }
    }
    else if (cmdType == "PING") 
    {
		std::string pong = "PONG\r\n";
		send(user->_fd, pong.c_str(), pong.length(), 0);
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
                // printSplitMsg(splitmsg);
                for (size_t j = 0; j < splitmsg.size(); j++) {
                    if (splitmsg[j] == "NICK" &&  splitmsg.size() > 1) {
                        std::string nick = splitmsg[j + 1];
                        users[i]._nickname = nick; // Сохраняем ник
                        users[i].nick_flag = 1;
                    }  
                    if (splitmsg[j] == "USER" && splitmsg.size() > 1) {
                        std::string user = splitmsg[j + 1];
                        users[i].setUser(user); // Сохраняем пользователя
                        users[i].user_flag = 1;
                    } 
                    if (splitmsg[j] == "PASS" && splitmsg.size() > 1) {
                        std::string pass = splitmsg[j + 1];
                        users[i]._password = pass; // Сохраняем пароль
                        users[i].pass_flag = 1;
                    }
                }
                std::cout << sd << " его sd" << std::endl;
                std::cout << "NICKNAME = " <<users[i]._nickname << " and flag = " << users[i].nick_flag << std::endl;
                std::cout << "PASSWORD = " <<users[i]._password << " and flag = " << users[i].pass_flag << std::endl;
                std::cout << "USERNAME = " <<users[i]._username << " and flag = " << users[i].user_flag << std::endl;
                if (users[i].pass_flag == 1 && users[i].nick_flag == 1 && users[i].user_flag == 1) {
                    if (users[i].cap == 0) {
			std::string wlcmMsg = ":irc 001 " + users[i]._nickname + " :Welcome to FT_IRC, " + users[i]._username + "@" + Server::_hostName + "\n"
								  ":irc 002 " + users[i]._nickname + " :Host is " + Server::_hostName + ", running version 1.0\n"
								  ":irc 003 " + users[i]._nickname + " :Created in 42 labs at July\n";
                        send(users[i]._fd, wlcmMsg.c_str(), strlen(wlcmMsg.c_str()), 0);
                        std::string firstServerMsg = "CAP * ACK multi-prefix\r\n";
                        send(sd, firstServerMsg.c_str(), firstServerMsg.length(), 0);
                        std::cout << "new client connected FD:" << sd << RESET << std::endl;
                        users[i].cap = 1;
                    }
                    users[i].execute(message, &users[i]); // Pass the current user
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



