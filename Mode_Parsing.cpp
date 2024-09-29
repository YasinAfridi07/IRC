 #include "Server.hpp"


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

// std::vector<User>::iterator Channel::channel_operator(int fd)
// {
// 	this->it_operators = this->operators.begin();
// 	while ( this->it_operators != this->operators.end())
// 	{
// 		if (this->it_operators->_fd == fd)
// 			return (it_operators);
// 		it_operators++;
// 	}
// 	return (it_operators);
// } // this function is used to get the user fd


// void Channel::setMode(char m, char sign)
// {
// 	std::map<char, int>::iterator it;
// 	it = this->_mode.begin();
// 	while ( it != this->_mode.end())
// 	{
// 		if (it->first == m) // the key "it->first" represents the mode charactor
// 		{
// 			if (sign == '+')
// 				it->second = 1; // the key "it->second" represents if the mode is in the "+" status
// 			else
// 				it->second = 0;
// 		}
// 		it++;
// 	}
// }

// void Channel::exec_mode(std::string mode, User &user_object, std::string arg)
// {
// 	if(mode[1] == 'k')
// 	{
// 		if(mode[0] == '+')
// 		{
// 			if(arg == "")
// 				ErrorMsg(user_object._fd, "Key for Channel Not Provided\n", "461"); // if no pass is provided
// 			else
// 			this->_pass = arg; // if pass is provided this line sets the pass
// 		}
// 		else
// 			this->_pass = ""; // else reset the pass
// 	}
// 	else if(mode[1] == 'o')
// 	{
// 		std::vector<User>::iterator it_s; // it_s represents iterator pointing to an element in the operators container.
// 		it_s = this->users.begin();
// 		while (it_s != this->users.end())
// 		{
// 			if(it_s->_nickname == arg) // need to understand more
// 				break;
// 			++it_s; // test this loop
// 		}
// 		if(it_s != this->users.end())
// 		{
// 			if(mode[0] == '+')
// 			{
// 				if(this->isOperator(*it_s))
// 					send(user_object._fd, "User is Already The Operator\n", strlen("User is Already The Operator\n"), 0 );
// 				else
// 				{
// 					this->operators.push_back(*it_s);
// 					send(it_s->_fd, "You are an operator of the channel \n", strlen("You are an operator of the Channel\n"), 0);
// 				}
// 			}
// 			else
// 			{
// 				it_s = this->channel_operator(it_s->_fd);
// 				if(it_s != this->operators.end())
// 				{
// 					if(it_s->_nickname != user_object._nickname)
// 					{
// 						send(it_s->_fd, "You are no longer an operator", strlen("You are no longer an operator"), 0);
// 						this->operators.erase(it_s);
// 					}
// 					else
// 						send(it_s->_fd, "Cannot remove youself from operatores\n", strlen("Cannot remove youself from operatores\n"), 0);
// 				}
// 				else
// 					send(user_object._fd, "User is not an Operator\n", strlen("User is not an Operator\n"), 0);
// 			}
// 		}
// 		else
// 			ErrorMsg(user_object._fd, (arg + "No such nickname\n"),  "401"); // test arg

// 	}
// 	else if(mode[1] == 'l')
// 	{
// 		if(mode[0] == '+')
// 		{
// 			if(std::atoi(arg.c_str()) <= 0)
// 				send(user_object._fd, "Invalid Limit Number\n", strlen("Invalid Limit Number\n"), 0);
// 			else
// 				this->_user_limit = std::atoi(arg.c_str());
// 		}
// 	}
// 	this->setMode(mode[1], mode[0]);
// }
