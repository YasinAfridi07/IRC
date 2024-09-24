

#include "Server.hpp"
#include "Channel.hpp"

void ErrorMsg(int fd, const std::string &message, const std::string &key)
{
	std::string errormsg = key + " ERROR: " + message;
	send(fd, errormsg.c_str(), strlen(errormsg.c_str()), 0);
}
