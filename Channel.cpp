

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



int Channel::isMode(char m)
{
	std::map<char,int>::iterator it;
	it = this->_mode.begin();
	while (it != this->_mode.end())
	{
		if(it->first == m)
		{
			if(it->second == 1)
				return(1);
			else if(it->second == 0)
				return(0);
		}
		it++;
	}
	return (2);
} // need to understand more about map (what is first and second)

int Channel::user_length(void)
{
	int length = 0;
	it_user = users.begin();

	while (it_user != users.end())
	{
		length++;
	}
	return(length);

}

void Channel::addUserToChannel(User new_user_object)
{
	if(operators.size() == 0)
	{
		operators.push_back(User(new_user_object));
	}
	if(this->isMode('l'))
	{
		if(this->user_length() == this->_user_limit)
		{
			ErrorMsg(new_user_object._fd, ("getname() here --->" +this->getName() + "Channel is Full"), "471"); // remove "getname() ---> here after testing"
			return ;
		}
	}
	users.push_back(User(new_user_object));
	std::string channel_welcome_msg;
	channel_welcome_msg = "\n - Welcome to Channel \n";
	send(new_user_object._fd, channel_welcome_msg.c_str(), strlen(channel_welcome_msg.c_str()), 0);
}
// need to connet to main funtion and execute my channel



// void Channel::kickUser(std::string kick_user, const std::vector<std::string> &splitmsg, User user_object)
// {
// 	std::vector<User>::iterator it;
// 	std::vector<User>::iterator it_s;
// 	std::vector<User>::iterator it_o;
// 	//unsigned long i = 3;
// 	it_s = this->users.begin();

// 	while (it_s != this->users.end())
// 	{
// 		if(it_s->_nickname == kick_user)
// 		{
// 			if(this->isOperator(user_object))
// 			{
// 				ErrorMsg(user_object._fd, "Permission Denied- You're not an operator of the channel.\n", "482");
// 				return ;
// 			}
// 			else
// 			{
// 				if(user_object._nickname == kick_user)
// 				{
// 					ErrorMsg(user_object._fd, "You cannot kick youself\n", "404");
// 					return ;
// 				}
// 				send(it_s->_fd, "You have been kicked\n", strlen("You have been kicked\n"), 0);
// 			}
// 		}
// 		++it_s; // needs testing
// 	}
// 	if (it_s == this->users.end())
// 		ErrorMsg(user_object._fd, (kick_user + "No such nickname\n"), "401");

// } // reason for kicking msg/output not added
// // new push


