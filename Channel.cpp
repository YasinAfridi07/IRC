

#include "Channel.hpp"
#include "Server.hpp"


Channel::Channel(std::string str_name, std::string str_pass)
{
	this->name = str_name;
	this->_pass = str_pass;
	this->_user_limit = 0;
	this->_topic = "";
	this->message = "";
	this->_mode['i'] = 0;
	this->_mode['t'] = 0;
	this->_mode['k'] = 0;
	this->_mode['o'] = 0;
	this->_mode['l'] = 0;
}
// Init all the veriables before creating the channel

Channel::~Channel(void)
{

}

// GETTERS
int	Channel::getUserInfo(void)
{
	return (_user_limit);
}

std::string	Channel::getTopic(void)
{
	return (_topic);
}

std::string	Channel::getPass(void)
{
	return (_pass);
}

std::map<char, int> Channel::getMode(void)
{
	return (_mode);
}


std::vector<User> Channel::getUsers(void)
{
	return (users);
}

std::vector<User> Channel::getOperators(void)
{
	return (operators);
}

std::string Channel::getName(void) const
{
	return (name);
}


// SETTERS

void Channel::setUserInfo(int num)
{
	_user_limit = num;
}

void Channel::setTopic(std::string str)
{
	_topic = str;
}

void Channel::setPass(std::string str)
{
	_pass = str;
}

void Channel::setMode(char m, char sign)
{
	std::map<char, int>::iterator it;
	for (it = this->_mode.begin(); it != this->_mode.end(); it++)
	{
		if (it->first == m)
		{
			if (sign == '+')
				it->second = 1; // what is second?
			else
				it->second = 0;
		}
	}
} // need to understand.
