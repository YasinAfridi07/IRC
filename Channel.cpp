
#include "Server.hpp"
// #include "Command.hpp"
#include <cstring>
#define JOIN "JOIN"
#define PRIVMSG_RPLY(senderNick, senderUsername, clientIp, revieverNick, msg)                std::string(userHostMask(senderNick, senderUsername, clientIp)+" PRIVMSG "+revieverNick+" :"+msg+"\r\n")
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
    std::map<char, int>::iterator it = this->_mode.find(m);
    if (it != this->_mode.end())
    {
        if (it->second == 1) // If mode is in '+' it is set to 1 status
            return 1;
        else if (it->second == 0) // If mode is in '-' it is set to 0 status (eg; MODE #channel -i)
            return 0;
    }
    return 2;
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

void Channel::addUserToChannel(User user_object) {
    // If no operators exist, add the first user as an operator
    if (operators.empty()) {
        operators.push_back(User(user_object));
    }

    // Check for user limit if the channel is in limit mode
    if (this->isMode('l')) {
        if (this->user_length() == this->_user_limit) {
            ErrorMsg(user_object._fd, (this->getName() + " :Channel is full.\r\n"), "471");
            return;
        }
    }

    // Add the user to the channel's user list
    users.push_back(User(user_object));

    // Prepare the welcome message
    std::string channel_welcome_msg;
    channel_welcome_msg = GREEN "Welcome to " + this->getName() + " channel, " + user_object._nickname + "!" RESET "\r\n";
    channel_welcome_msg += BLUE "Here are some commands you can use:" RESET "\r\n";
    channel_welcome_msg += CYAN " - JOIN <channel> [key]: Join a channel" RESET "\r\n";
    channel_welcome_msg += CYAN " - PART <channel>: Leave a channel" RESET "\r\n";
    channel_welcome_msg += CYAN " - MSG <user/channel> <message>: Send a private message" RESET "\r\n";
    channel_welcome_msg += CYAN " - NICK <new_nickname>: Change your nickname" RESET "\r\n";
    channel_welcome_msg += CYAN " - WHO <channel>: List users in a channel" RESET "\r\n";
    channel_welcome_msg += CYAN " - INVITE <user> <channel>: Invite a user to a channel" RESET "\r\n";
    channel_welcome_msg += CYAN " - KICK <user> <channel>: Kick a user from a channel" RESET "\r\n";
    channel_welcome_msg += CYAN " - TOPIC <channel> <topic>: Set the channel topic" RESET "\r\n";
    channel_welcome_msg += CYAN " - MODE <channel> <mode(eg: +/-i, +/-o, +/-t, +/-k, +/-l)> [parameters]: Set channel modes" RESET "\r\n";

    // Send the welcome message to the user
    send(user_object._fd, channel_welcome_msg.c_str(), channel_welcome_msg.length(), 0);
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

void Command::ajoin(std::string channel_s, std::string key_s, User user) {
    std::vector<Channel>::iterator it;
    std::vector<User>::iterator it_i;

    // Check if the channel name is valid
    if (channel_s.at(0) != '#' && channel_s.at(0) != '&') {
        send(user._fd, "401 :Invalid Channel Name\r\n", strlen("401 :Invalid Channel Name\r\n"), 0);
        return;
    } else if (channel_s.size() <= 1) {
        send(user._fd, "401 :Invalid Channel Name\r\n", strlen("401 :Invalid Channel Name\r\n"), 0);
        return;
    }

    // Check if the channel exists
    it = channel_exist(channel_s);
    if (it != Server::_channels.end()) {
        // If the user is already in the channel
        if (it->isUser(user)) {
            ErrorMsg(user._fd, "443 :" + user._nickname + " " + it->getName() + " :User Already in Channel\r\n", "443");
            return;
        }

        // Handle key (password) for the channel
        if (key_s != "") {
            if (it->isMode('k') == 1) { // If channel requires a key
                if (key_s == it->getPass()) { // Correct key provided
                    if (it->isMode('i') == 1) { // Check invite-only mode
                        if (it->isInvited(user)) {
                            it_i = it->inv_in_chan(user._fd);
                            if (it_i != it->invites.end())
                                it->invites.erase(it_i);
                            it->addUserToChannel(user);
                        } else {
                            ErrorMsg(user._fd, "473 :" + it->getName() + " :Invite Only Mode is on\r\n", "473");
                            return;
                        }
                    } else {
                        it->addUserToChannel(user);
                    }
                } else {
                    ErrorMsg(user._fd, "475 :" + it->getName() + " :Keypass Mode is on\r\n", "475");
                    return;
                }
            } else {
                ErrorMsg(user._fd, "475 :Key Not required to join channel\r\n", "475");
                return;
            }
        } else {
            // If no key is provided, check invite-only mode
            if (it->isMode('i') == 1) {
                if (it->isInvited(user)) {
                    it_i = it->inv_in_chan(user._fd);
                    if (it_i != it->invites.end())
                        it->invites.erase(it_i);
                    it->addUserToChannel(user);
                } else {
                    ErrorMsg(user._fd, "473 :" + it->getName() + " :Invite Only Mode is on\r\n", "473");
                    return;
                }
            } else {
                it->addUserToChannel(user);
            }
        }
    } else {
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

std::vector<Channel>::iterator Command::channel_exist(std::string channel)
{
	this->channel_it = Server::_channels.begin();
	while (this->channel_it != Server::_channels.end())
	{
		if (channel_it->getName() == channel)
			return channel_it;
		channel_it++;
	}
	return channel_it;
}

std::vector<User>::iterator Command::user_exist(std::string nick)
{
	this->user_it = Server::users.begin();
	while (this->user_it != Server::users.end())
	{
		if (user_it->_nickname == nick)
			return user_it;
		user_it++;
	}
	return user_it;
}

void Command::privmsg(std::string receiver, const std::vector<std::string>& splitmsg, User user) {
    std::vector<Channel>::iterator it_channel;
    std::vector<User>::iterator it_user;
    unsigned long i = 2;

    // Check if the receiver is a user
    it_user = user_exist(receiver);
    if (it_user == Server::users.end()) {
        // If not a user, check if it's a channel
        it_channel = channel_exist(receiver);
        if (it_channel != Server::_channels.end()) {
            // Check if the user is part of the channel
            if (it_channel->isUser(user)) {
                std::vector<User> temp_users = it_channel->getUsers();
                for (std::vector<User>::iterator it = temp_users.begin(); it != temp_users.end(); ++it) {
                    if (it->_fd != user._fd) {
                        // Send message to all users in the channel except the sender
                        std::string msg = ":" + user._nickname + " PRIVMSG " + receiver + " :";
                        send(it->_fd, msg.c_str(), msg.length(), 0);
                        while (i < splitmsg.size()) {
                            send(it->_fd, (splitmsg.at(i)).c_str(), strlen((splitmsg.at(i)).c_str()), 0);
                            if (i + 1 < splitmsg.size()) {
                                send(it->_fd, " ", 1, 0); // Send space only if there's another message
                            }
                            i++;
                        }
                        send(it->_fd, "\r\n", 2, 0); // Use \r\n for proper termination
                        i = 2; // Reset for next user
                    }
                }
            } else {
                ErrorMsg(user._fd, (it_channel->getName() + " :You are not part of the channel.\r\n"), "404");
            }
        }
    } else {
        // Handle the case if the receiver is a user
        if (user._fd == it_user->_fd) {
            send(it_user->_fd, "Cannot send message to yourself.\r\n", strlen("Cannot send message to yourself.\r\n"), 0);
        } else {
            std::string msg = ":" + user._nickname + " PRIVMSG " + receiver + " :";
            send(it_user->_fd, msg.c_str(), msg.length(), 0);
            while (i < splitmsg.size()) {
                send(it_user->_fd, (splitmsg.at(i)).c_str(), strlen((splitmsg.at(i)).c_str()), 0);
                if (i + 1 < splitmsg.size()) {
                    send(it_user->_fd, " ", 1, 0); // Send space only if there's another message
                }
                i++;
            }
            send(it_user->_fd, "\r\n", 2, 0); // Use \r\n for proper termination
            i = 2; // Reset for next message
        }
    }

    // If receiver is not found as user or channel, send error message
    if (it_user == Server::users.end() && it_channel == Server::_channels.end()) {
        ErrorMsg(user._fd, (receiver + " :No such nickname or channel.\r\n"), "401");
    }
}



void Command::invite(std::string user, std::string channel, User user_object) {
    std::vector<Channel>::iterator it_c = channel_exist(channel);
    std::vector<User>::iterator it_s = user_exist(user);

    if (it_c != Server::_channels.end()) {
        if (it_s != Server::users.end()) {
            if (it_c->isOperator(user_object) != 1) {
                ErrorMsg(user_object._fd, "You are not an operator.\r\n", "482");
            } else {
                if (it_c->isUser(*it_s)) {
                    ErrorMsg(user_object._fd, "User is already in the channel.\r\n", "443");
                } else {
                    if (it_c->isMode('i') == 1) {
                        if (it_c->isInvited(*it_s)) {
                            send(user_object._fd, "User is already invited.\r\n", strlen("User is already invited.\r\n"), 0);
                        } else {
                            std::string message = "You're invited to the channel " + channel + "\r\n";
                            send(it_s->_fd, message.c_str(), message.length(), 0);
                            it_c->invites.push_back(*it_s); // Pushing the user object to the channel's invite list

                            std::string confirmation = "Invite was successfully sent to " + user + " for the channel " + channel + "\r\n";
                            send(user_object._fd, confirmation.c_str(), confirmation.length(), 0);
                        }
                    } else {
                        send(user_object._fd, "Channel is not in +i mode.\r\n", strlen("Channel is not in +i mode.\r\n"), 0);
                    }
                }
            }
        } else {
            ErrorMsg(user_object._fd, "Invalid nickname.\r\n", "401");
        }
    } else {
        ErrorMsg(user_object._fd, "Invalid channel.\r\n", "403");
    }
}


void Command::kick(std::string channel, std::string user_kick, const std::vector<std::string>& splitmsg, User user) {
    // Find the channel to kick the user from
    std::vector<Channel>::iterator it_c = channel_exist(channel);

    if (it_c != Server::_channels.end()) // Check if the channel exists
    {
        // Find the user to be kicked within the channel
        std::vector<User>::iterator it_s;
        std::vector<User>::iterator it_o;
        unsigned long i = 3;

        // Iterate through users in the channel to find the one to be kicked
        for(it_s = it_c->users.begin(); it_s != it_c->users.end(); ++it_s) {
            if (it_s->_nickname == user_kick) {
                // Check if the command user is an operator
                if (it_c->isOperator(user) != 1) {
                    ErrorMsg(user._fd, "Not an operator.\r\n", "482"); // Not an operator error
                    return;
                } else {
                    if(user._nickname == user_kick) {
                        ErrorMsg(user._fd, "You cannot kick yourself.\r\n", "404"); // Self-kick error
                        return;
                    }

                    // Inform the user they're being kicked
                    send(it_s->_fd, "You have been kicked from the channel.\r\n", strlen("You have been kicked from the channel.\r\n"), 0);

                    // If a reason is provided, send it to the kicked user
                    if (splitmsg.size() > 3) {
                        send(it_s->_fd, "Reason for kicking: ", strlen("Reason for kicking: "), 0);
                    }

                    while (i < splitmsg.size()) {
                        send(it_s->_fd, (splitmsg.at(i)).c_str(), strlen((splitmsg.at(i)).c_str()), 0);
                        send(it_s->_fd, " ", strlen(" "), 0);
                        i++;
                    }

                    send(it_s->_fd, "\r\n", strlen("\r\n"), 0);

                    // Remove the user from the channel's user list
                    it_c->users.erase(it_s);

                    // Remove the user from the operator list if they are an operator
                    for (it_o = it_c->operators.begin(); it_o != it_c->operators.end(); ++it_o) {
                        if (it_o->_nickname == user_kick) {
                            it_c->operators.erase(it_o);
                            break;
                        }
                    }

                    return;
                }
            }
        }

        // If the user to kick was not found in the channel's users list
        if (it_s == it_c->users.end()) {
            ErrorMsg(user._fd, (user_kick + " :No such nickname.\r\n"), "401");
        }
    } else {
        // Channel not found error
        ErrorMsg(user._fd, (channel + " :No such channel.\r\n"), "403");
    }
}

