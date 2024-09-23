

#include "Channel.hpp"
#include "Server.hpp"
#include <cstring>
#include "Command.hpp"
#define JOIN "JOIN"

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
}

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

void Channel::addUserToChannel(User user_object)
{
	if(operators.size() == 0)
	{
		operators.push_back(User(user_object));
	}
	if(this->isMode('l'))
	{
		if(this->user_length() == this->_user_limit)
		{
			ErrorMsg(user_object._fd, ("getname() here --->" +this->getName() + "Channel is Full"), "471"); // remove "getname() ---> here after testing"
			return ;
		}
	}
	users.push_back(User(user_object));
	std::string channel_welcome_msg;
	channel_welcome_msg = "\n - Welcome to Channel \n";
	send(user_object._fd, channel_welcome_msg.c_str(), strlen(channel_welcome_msg.c_str()), 0);
}

void handleJoinCommand(const std::vector<std::string>& splitmsg, Command& command, User* user)
{
	if (splitmsg.size() == 2)
	{
		command.join(splitmsg.at(1), "", *user);
	} else if (splitmsg.size() == 3)
	{
		command.join(splitmsg.at(1), splitmsg.at(2), *user);
	}
	else
	{
		return ;
	}
}


void Command::join(std::string channel_s, std::string key_s, User user_object)
{
	std::vector<std::string> channel_split = ft_split(channel_s, ',');
	std::vector<std::string> key_split = ft_split(key_s, ',');
	std::vector<Channel>::iterator it;
	std::vector<User>::iterator it_invite;
	std::vector<std::string>::iterator it_split;
	std::vector<std::string>::iterator it_key;

	it_key = key_split.begin();
	it_split = channel_split.begin();

	while (it_split != channel_split.end())
	{
		
		it_split++;
	}


}


void user_cmds(User* user, std::vector<std::string> splitmsg)
{
    if (splitmsg.empty()) {
        return;
    }

    Command cmd;
    std::string cmdType = splitmsg.at(0);
    if (cmdType == JOIN)
	{
        handleJoinCommand(splitmsg, cmd, user);
    }
} // tim part
