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

void Channel::exec_mode(std::string mode, User &user_object, std::string arg) {
    switch (mode[1]) {
        case 'k':
            if (mode[0] == '+') {
                if (arg.empty()) {
                    ErrorMsg(user_object._fd, "Key for channel not provided.\r\n", "461"); // No pass provided
                } else {
                    this->_pass = arg; // Set the pass
                }
            } else {
                this->_pass.clear(); // Reset the pass
            }
            break;

        case 'o': {
            std::vector<User>::iterator it_s = this->users.begin();
            while (it_s != this->users.end()) {
                if (it_s->_nickname == arg)
                    break;
                ++it_s;
            }
            if (it_s != this->users.end()) {
                if (mode[0] == '+') {
                    if (this->isOperator(*it_s)) {
                        send(user_object._fd, "User is already the operator.\r\n", strlen("User is already the operator.\r\n"), 0);
                    } else {
                        this->operators.push_back(*it_s);
                        send(it_s->_fd, "You are now an operator of the channel.\r\n", strlen("You are now an operator of the channel.\r\n"), 0);
                    }
                } else {
                    it_s = this->channel_operator(it_s->_fd);
                    if (it_s != this->operators.end()) {
                        if (it_s->_nickname != user_object._nickname) {
                            send(it_s->_fd, "You are no longer an operator.\r\n", strlen("You are no longer an operator.\r\n"), 0);
                            this->operators.erase(it_s);
                        } else {
                            send(it_s->_fd, "Cannot remove yourself from operators.\r\n", strlen("Cannot remove yourself from operators.\r\n"), 0);
                        }
                    } else {
                        send(user_object._fd, "User is not an operator.\r\n", strlen("User is not an operator.\r\n"), 0);
                    }
                }
            } else {
                ErrorMsg(user_object._fd, (arg + " :No such nickname.\r\n"), "401"); // User not found
            }
            break;
        }
        case 'l':
            if (mode[0] == '+') {
                int new_limit = std::atoi(arg.c_str());
                if (new_limit <= 0) {
                    send(user_object._fd, "Invalid limit number.\r\n", strlen("Invalid limit number.\r\n"), 0);
                } else {
                    if (this->user_length() > new_limit) {
                        // Prevent setting the limit if the current user count exceeds the new limit
                        send(user_object._fd, "Cannot set limit. Current user count exceeds the new limit.\r\n", strlen("Cannot set limit. Current user count exceeds the new limit.\r\n"), 0);
                    } else {
                        // Set the new limit
                        this->_user_limit = new_limit;
                        // Notify the client that the channel now has a limit
                        std::string limit_msg = "Channel " + this->getName() + " now has a user limit of " + std::to_string(this->_user_limit) + ".\r\n";
                        send(user_object._fd, limit_msg.c_str(), limit_msg.length(), 0);
                    }
                }
            }
            break;

    }
    this->setMode(mode[1], mode[0]); // Set the mode
}
