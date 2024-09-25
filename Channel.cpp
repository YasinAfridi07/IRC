
#include "Server.hpp"
// #include "Command.hpp"
#include <cstring>
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

Channel::~Channel(void){}

// GETTERS
int	Channel::getUserInfo(void){return (_user_limit);}

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
std::vector<Channel>::iterator Command::chan_exist(std::string channel)
{
	for (this->chan_it = Server::_channels.begin(); this->chan_it != Server::_channels.end(); chan_it++)
	{
		if (chan_it->getName() == channel)
			return chan_it;
	}
	return chan_it;
}

void Command::ajoin(std::string channel_s, std::string key_s, User user)
{
    std::vector<Channel>::iterator it;
    std::vector<User>::iterator it_i;

    // Check if the channel name is valid
    if (channel_s.at(0) != '#' && channel_s.at(0) != '&')
    {
        send(user._fd, "Invalid Channel Name", strlen("Invalid Channel Name"), 0);
        return;
    }
    else if (channel_s.size() <= 1)
    {
        send(user._fd, "Invalid Channel Name", strlen("Invalid Channel Name"), 0);
        return;
    }

    // Check if the channel exists
    it = chan_exist(channel_s);
    if (it != Server::_channels.end())
    {
        // If the user is already in the channel
        if (it->isUser(user))
        {
            ErrorMsg(user._fd, (user._nickname + " " + it->getName() + " User Already in Channel"), "443");
            return;
        }

        // Handle key (password) for the channel
        if (key_s != "")
        {
            if (it->isMode('k') == 1) // If channel requires a key
            {
                if (key_s == it->getPass()) // Correct key provided
                {
                    if (it->isMode('i') == 1) // Check invite-only mode
                    {
                        if (it->isInvited(user))
                        {
                            it_i = it->inv_in_chan(user._fd);
                            if (it_i != it->invites.end())
                                it->invites.erase(it_i);
                            it->addUserToChannel(user);
                        }
                        else
                        {
                            ErrorMsg(user._fd, (it->getName() + " Invite Only Mode is on"), "473");
                            return;
                        }
                    }
                    else
                    {
                        it->addUserToChannel(user);
                    }
                }
                else
                {
                    ErrorMsg(user._fd, (it->getName() + " Keypass Mode is on"), "475");
                    return;
                }
            }
            else
            {
                ErrorMsg(user._fd, "Key Not required to join channel\n", "475");
                return;
            }
        }
        else
        {
            // If no key is provided, check invite-only mode
            if (it->isMode('i') == 1)
            {
                if (it->isInvited(user))
                {
                    it_i = it->inv_in_chan(user._fd);
                    if (it_i != it->invites.end())
                        it->invites.erase(it_i);
                    it->addUserToChannel(user);
                }
                else
                {
                    ErrorMsg(user._fd, (it->getName() + " Invite Only Mode is on"), "473");
                    return;
                }
            }
            else
            {
                it->addUserToChannel(user);
            }
			who(channel_s, user);
        }

    }
    else
    {
        // If channel does not exist, create a new one
        Channel new_channel(channel_s, key_s);
        new_channel.addUserToChannel(user);
        Server::_channels.push_back(new_channel);
    }
}

int Channel::isInvited(User user)
{
	std::vector<User>::iterator it;
	for (it = this->invites.begin(); it != this->invites.end(); it++)
	{
		if (it->_nickname == user._nickname)
			return (1);
	}
	return (0);
}
std::vector<User>::iterator Channel::inv_in_chan(int fd)
{
	for (this->it_invites = this->invites.begin(); this->it_invites != this->invites.end(); it_invites++)
	{
		if (this->it_invites->_fd == fd)
			return (it_invites);
	}
	return (it_invites);
}

int Channel::isUser(User user)
{
	std::vector<User>::const_iterator it;
	for (it = this->users.begin(); it != this->users.end(); it++)
	{
		if (it->_nickname == user._nickname)
			return (1);
	}
	return (0);
}

// hello my name is yasin
// hi
