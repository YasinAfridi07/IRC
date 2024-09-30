
#ifndef USER_HPP
#define USER_HPP



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
#include "Server.hpp"

#define MAX_PORT UINT16_MAX
#define MAX_BUFFER 1024
#define MAX_HOST_NAME 512
#define CYAN "\033[36m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

class User {
public:
    std::string _username;
    std::string _nickname;
    std::string _password;
	int	_fd;

    User() {}

    User(const std::string &user, const std::string &nick, const std::string &pass)
        : _username(user), _nickname(nick), _password(pass) {}

    void setUser(const std::string &user) { _username = user; }
    void setNick(const std::string &nick) { _nickname = nick; }
    void setPass(const std::string &pass) { _password = pass; }

    std::string getUser() const { return _username; }
    std::string getNick() const { return _nickname; }
    std::string getPass() const { return _password; }

    void process_cmd(std::string cmd, User *it);
};

#endif
