#include "Command.hpp"
#include "Server.hpp"
#include "User.hpp"

Command::Command(void)
{
	// defualt constructor
	this->message = "";

}

Command::~Command(void)
{
	// destructor
}

void Command::who(std::string channel_s, User user)
{
	// Find the channel
	std::vector<Channel>::iterator it = channel_exist(channel_s);
	if (it == Server::_channels.end())
	{
		ErrorMsg(user._fd, "Channel does not exist.", "403");
		return ;
	}
	// Prepare the user list
	std::string userList = "Users in " + it->getName() + ": ";
	// Get the users in the channel
	std::vector<User> usersInChannel = it->getUsers();
	// Use a traditional for loop to append all user nicknames
	for (std::vector<User>::iterator it_user = usersInChannel.begin(); it_user != usersInChannel.end(); ++it_user)
	{
		userList += it_user->_nickname + ", "; // Append each user's nickname
	}
	// Trim the trailing space
	if (!userList.empty())
	{
		userList.pop_back(); // Remove the last space
	}
	// Send the user list to the user who requested it
	send(user._fd, userList.c_str(), userList.length(), 0);
}

void Command::mode(std::string channel_s, std::string mode, User user, std::string arg)
{
    std::vector<Channel>::iterator it_c;

    // Verify if mode string is valid and if mode is known
    if (mode.size() != 2 && (mode[0] != '+' && mode[0] != '-'))
		{
        ErrorMsg(user._fd, (mode + " :is unknown mode char to me.\n"), "472");
        return;
    }
    // Check if channel exists
    it_c = channel_exist(channel_s);
    if (it_c == Server::_channels.end())
		{
        ErrorMsg(user._fd, (channel_s + " :No such channel.\n"), "403");
        return;
    }
    // Check if user is an operator
    if (!it_c->isOperator(user)) {
        ErrorMsg(user._fd, "Permission Denied- You're not an operator of the channel.\n", "482");
        return;
    }
		if (it_c->isMode(mode[1]) == 2)
    {
        ErrorMsg(user._fd, (mode + " :is unknown mode char to me.\n"), "472");
        return;
    }
    // Execute mode change
    it_c->exec_mode(mode, user, arg);
}

// Pseudocode:
// Check if the channel exists.
// If the channel exists:
// Check if the channel is in 't' mode.
// If in 't' mode, check if the user is an operator.
// If the user is an operator, set or get the topic based on the provided topic string.
// If the user is not an operator, send an error message.
// If not in 't' mode, set or get the topic based on the provided topic string.
// If the channel does not exist, send an error message.
void Command::topic(std::string channel_s, std::string topic, User user)
{
        // Check if channel exists
        std::vector<Channel>::iterator it_c = channel_exist(channel_s);
        if (it_c != Server::_channels.end())
        {
                // Check if channel is in 't' mode
                if (it_c->isMode('t') == 1)
                {
                        // Check if user is an operator
                        if (it_c->isOperator(user))
                        {
                                if (topic.empty())
                                {
                                        // Get the topic
                                        std::string currentTopic = it_c->getTopic();
                                        // Send the topic to the user
                                        send(user._fd, currentTopic.c_str(), currentTopic.length(), 0);
                                }
                                else
                                {
                                        // Set the topic
                                        it_c->setTopic(topic);
                                        send(user._fd, "Topic set successfully.\n", strlen("Topic set successfully.\n"), 0);
                                }
                        }
                        else
                        {
                                ErrorMsg(user._fd, "Permission Denied- You're not an operator of the channel.\n", "482");
                        }
                }
                else // if not in mode 't'
                {
                        if (topic.empty())
                        {
                                // Get the topic
                                std::string currentTopic = it_c->getTopic();
                                // Send the topic to the user
                                send(user._fd, currentTopic.c_str(), currentTopic.length(), 0);
                        }
                        else
                        {
                                // Set the topic
                                it_c->setTopic(topic);
                                send(user._fd, "Topic set successfully.\n", strlen("Topic set successfully.\n"), 0);
                        }
                }
        }
        else
        {
                ErrorMsg(user._fd, (channel_s + " :No such channel.\n"), "403");
        }
}

void User::process_cmd(std::string mes, User *user)
{
	std::vector<std::string> splitmsg = split(mes);
	if (splitmsg.empty())
	{
		return ;
	}
	Command cmd; // Create a Command object
	std::string cmdType = splitmsg.at(0);
	if (cmdType == "JOIN")
	{
		if (splitmsg.size() == 2)
		{
			cmd.ajoin(splitmsg.at(1), "", *user);
		}
		else if (splitmsg.size() == 3)
		{
			cmd.ajoin(splitmsg.at(1), splitmsg.at(2), *user);
		}
		else
			return ;
	}
	else if (cmdType == "WHO")
	{
		if (splitmsg.size() == 2)
			cmd.who(splitmsg.at(1), *user);
		else
			// Handle other cases or add an error message
			ErrorMsg(user->_fd, "Invalid number of arguments for /CHANUSER", "461");
	}
	else if (cmdType == "PRIVMSG")
	{
		if (splitmsg.size() >= 3)
		{
			cmd.privmsg(splitmsg.at(1), splitmsg, *user);
			// second argument will be the split message for mutiple words
		}
		else if (splitmsg.size() == 2)
		{
			// no such nickname, if nickname doesn't exist
			ErrorMsg(user->_fd, " Min 3 arg required\n", "412");
		}
		else if (splitmsg.size() == 1)
		{
			ErrorMsg(user->_fd, "Need 2 more arg after PRIVMSG ", "401");
		}
		else
		{ // if PRIVMSG nickname exist and msg dosent exist
			std::string S = "461";
			S.append(" :Not enough parameters\r\n");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			return ;
		}
	}
	else if (cmdType == "TOPIC")
	{
		if (splitmsg.size() == 3)
		{
			cmd.topic(splitmsg.at(1), splitmsg.at(2), *user);
		}
		else if (splitmsg.size() == 2)
		{
			cmd.topic(splitmsg.at(1), "", *user);
		}
		else
		{
			ErrorMsg(user->_fd, "TOPIC command requires 1 or 2 arguments\n", "461");
		}
	}
	else if (cmdType == "MODE")
	{
		if (splitmsg.size() == 4)
		{
			cmd.mode(splitmsg.at(1), splitmsg.at(2), *user, splitmsg.at(3));
			// channel, mode, user, arg
		}
		// MODE #channel +o nickname
		else if (splitmsg.size() == 3)
		{
			cmd.mode(splitmsg.at(1), splitmsg.at(2), *user, ""); //
		}
		else
		{
			ErrorMsg(user->_fd, "Not enough parameters", "461");
		}
	}
}

std::vector<std::string> Command::ft_split(std::string str, char delimiter)
{
	std::vector<std::string> substrings;
	std::string substring = "";
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] != delimiter)
		{
			substring += str[i];
		}
		else
		{
			substrings.push_back(substring);
			while (str[i] == delimiter)
				i++;
			i--;
			substring = "";
		}
	}
	substrings.push_back(substring);
	return (substrings);
}
