
#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <iostream>
#include <string>
#include <vector>

class User;
class Channel;

class Command{
	private:
		std::string message;
		std::vector<Channel>::iterator chan_it;
		std::vector<User>::iterator user_it;
	public:
		Command(void);
		~Command(void);

	void join(std::string channel_s, std::string key_s, User user);
	std::vector<std::string> Command::ft_split(std::string str, char delimiter);
};

#endif
