

#include "Server.hpp"


void ErrorMsg(int fd, const std::string &message, const std::string &key)
{
	std::string errormsg = key, + "Error" + message;
	send(fd, errormsg.c_str(), strlen(errormsg.c_str()), 0); 
}
