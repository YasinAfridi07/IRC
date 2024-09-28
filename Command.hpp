
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

		void join(std::string channel_name, std::string key_pass, User user_object); // Yasin
		void privmsg(std::string receiver, const std::vector<std::string>& splitmsg, User user); // Yasin
		void invite(std::string user, std::string channel, User user_object); // Yasin

		std::vector<std::string> ft_split(std::string str, char delimiter);
		std::vector<Channel>::iterator channel_exist(std::string channel);
		std::vector<User>::iterator user_exist(std::string nick);
};

#endif
