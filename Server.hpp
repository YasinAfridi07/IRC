#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <cerrno>
#include <iomanip>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <cstdlib>
#include "Channel.hpp"
#include "User.hpp"
#include "Command.hpp"
#include "Extra.hpp"

#define MAX_PORT UINT16_MAX
#define MAX_BUFFER 1024
#define MAX_HOST_NAME 512
#define CYAN "\033[36m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define BLINK "\033[5m"
#define REVERSE "\033[7m"
#define HIDDEN "\033[8m"
#define BLACK "\033[30m"
#define WHITE "\033[37m"
#define BBLACK "\033[40m"
#define BWHITE "\033[47m"
#define BRED "\033[41m"
#define BGREEN "\033[42m"
#define BYELLOW "\033[43m"
#define BBLUE "\033[44m"
#define BMAGENTA "\033[45m"
#define BCYAN "\033[46m"
#define ITALIC "\033[3m"
#define STRIKE "\033[9m"
#define CLEAR "\033[2J"
#define CLRLINE "\033[K"
#define CLRL "\033[2K"
#define CLRS "\033[1J"
#define CLRLS "\033[2J\033[1;1H"
#define CLRE "\033[0K"

#define RESET "\033[0m"


class Server
{
private:
	Server(void);

public:
	static const int MAX_CLIENTS = FD_SETSIZE;
	static const int BUFFER_SIZE = MAX_BUFFER;
	static std::string _password;
	static std::string _hostName;
	static std::string bufferStr;
	static char c_buffer[MAX_BUFFER];
	static char c_hostName[MAX_HOST_NAME];
	static int serverSocket;
	static int max_sd;
	static int sd;
	static int valread;
	static int _port;
	static int newSocket;
	static int curIndex;
	static int addrlen;
	static struct sockaddr_in address;
	static fd_set readfds;
	class ServerException : public std::exception
	{
		private:
			std::string _msg;
		public:
			ServerException(std::string msg) : _msg(msg) {}
			virtual ~ServerException() throw() {}
			virtual const char *what() const throw() { return _msg.c_str(); }
	};
	static std::vector<int> _fds;
	static std::vector<User> users;  // Контейнер для пользователей
	static std::vector<Channel> _channels;
    static void handleClientMessages();
	static void openSocket(void);
	static void run(void);
	static void acceptConnection(void);
	static void handleClientDisconnection(size_t i);
	static void handleNickCommand(size_t i, const std::string& message);
	static void handleUserCommand(size_t i, const std::string& message);
	static bool isUserAuthorized(size_t i);
	static void handlePassCommand(size_t i, const std::string& message);
	// static void showUsers(void);
	// static void showChannels(void);
	// static std::string getPassword(void);
};

void valid_arg(std::string a, std::string b, int c);
void Check(int ac);


#endif
