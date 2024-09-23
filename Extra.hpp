#ifndef EXTRA_HPP
#define EXTRA_HPP

#include "Server.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
class User;


void ErrorMsg(int fd, const std::string &message, const std::string &key);
std::vector<std::string> split(const std::string str);
std::vector<std::string> ft_split(std::string str, char delimiter);

#endif
