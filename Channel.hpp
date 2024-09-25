#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include "Server.hpp"



class User;
class Channel
{
	private:
		int _user_limit;
		std::string _pass;
		std::string _topic;
		std::map<char, int> _mode;
	public:
		Channel(); // why private
		std::string name;
		std::vector<User> invites;
		std::vector<User> operators;
		std::vector<User> users;
		std::string message;
		std::vector<User>::iterator it_user;
		std::vector<User>::iterator it_operators;
		std::vector<User>::iterator it_invites;
		Channel(std::string str_name, std::string str_pass);
		~Channel();

		int getUserInfo();
		std::string getTopic();
		std::string getPass();
		std::map<char, int> getMode();
		std::vector<User> getUsers();
		std::vector<User> getOperators();
		std::string getName() const;

		void setUserInfo(int num);
		void setTopic(std::string str);
		void setPass(std::string str);
		void setMode(char m, char sign);

		void addUserToChannel(User new_user_object);
		//void kickUser(std::string user_kick, const std::vector<std::string>& splitmsg, User user); // tim
		//void exec_mode(std::string mode, User &user_object, std::string arg); // ayan
		std::vector<User>::iterator user_in_chan(int fd);
		std::vector<User>::iterator channel_operator(int fd);
		std::vector<User>::iterator inv_in_chan(int fd);
		int user_length(void);


		int isInvited(User user); // tim
		int isMode(char m); // ayan
		int isOperator(User user);
		int isUser(User user); //  tim



};


#endif
