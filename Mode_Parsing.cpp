 #include "Server.hpp"
#include "Channel.hpp"

int Channel::isOperator(User user)
{
	std::vector<User>::const_iterator it;
	it = this->operators.begin();
	while (it != this->operators.end())
	{
		if (it->_nickname == user._nickname)
			return (1);
		it++;
	}
	return (0);
} // this function is used for checking if the user is an operator

std::vector<User>::iterator Channel::channel_operator(int fd)
{
	this->it_operators = this->operators.begin();
	while ( this->it_operators != this->operators.end())
	{
		if (this->it_operators->_fd == fd)
			return (it_operators);
		it_operators++;
	}
	return (it_operators);
} // this function is used to get the user fd

void Channel::setMode(char modeChar, char sign)
{
    std::map<char, int>::iterator it;
    it = this->_mode.find(modeChar);// If the mode character is found
    if (it != this->_mode.end())//(i.e., it is not equal to _mode.end()), the function proceeds to check the value of sign
    {
        if (sign == '+')
            it->second = 1;// the key "it->second" represents if the mode is in the "+" status
        else// If mode is in '-' status (eg; MODE #channel -i)
            it->second = 0;
    }
}

void Channel::exec_mode(std::string mode, User &user_object, std::string arg)
{
	switch (mode[1])
	{
		case 'k':
				if (mode[0] == '+')
				{
						if (arg == "")
								ErrorMsg(user_object._fd, "Key for Channel Not Provided\n", "461"); // if no pass is provided
						else
								this->_pass = arg; // if pass is provided this line sets the pass
				}
				else
				{
						this->_pass = ""; // else reset the pass
				}
				break;
		case 'o':
		{
				std::vector<User>::iterator it_s = this->users.begin();
				while (it_s != this->users.end())
				{
						if (it_s->_nickname == arg)
								break;
						++it_s;
				}
				if (it_s != this->users.end())
				{
						if (mode[0] == '+')
						{
								if (this->isOperator(*it_s))
										send(user_object._fd, "User is Already The Operator\n", strlen("User is Already The Operator\n"), 0);
								else
								{
										this->operators.push_back(*it_s);
										send(it_s->_fd, "You are an operator of the channel \n", strlen("You are an operator of the Channel\n"), 0);
								}
						}
						else
						{
								it_s = this->channel_operator(it_s->_fd);
								if (it_s != this->operators.end())
								{
										if (it_s->_nickname != user_object._nickname)
										{
												send(it_s->_fd, "You are no longer an operator", strlen("You are no longer an operator"), 0);
												this->operators.erase(it_s);
										}
										else
												send(it_s->_fd, "Cannot remove yourself from operators\n", strlen("Cannot remove yourself from operators\n"), 0);
								}
								else
										send(user_object._fd, "User is not an Operator\n", strlen("User is not an Operator\n"), 0);
						}
				}
				else
						ErrorMsg(user_object._fd, (arg + "No such nickname\n"), "401"); // test arg
				break;
		}
		case 'l':
				if (mode[0] == '+')
				{
						if (std::atoi(arg.c_str()) <= 0)
								send(user_object._fd, "Invalid Limit Number\n", strlen("Invalid Limit Number\n"), 0);
						else
								this->_user_limit = std::atoi(arg.c_str());
				}
				break;
	}
	this->setMode(mode[1], mode[0]); // Where the mode is set
}