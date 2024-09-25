
#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <iostream>
#include <string>
#include <vector>
#include "Server.hpp"


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

		//void ajoin(std::string channel_name, std::string key_pass, User user_object); // Yasin
		void ajoin(std::string channel_s, std::string key_s, User user);
		void who(std::string channel_s, User user);
		std::vector<std::string> ft_split(std::string str, char delimiter);
		std::vector<Channel>::iterator chan_exist(std::string channel);
};

#endif
