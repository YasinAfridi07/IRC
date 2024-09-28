

#include "Server.hpp"


void ErrorMsg(int fd, const std::string &error_msg, const std::string &code)
{
	std::string errormsg = code + " ERROR: " + error_msg;
	send(fd, errormsg.c_str(), strlen(errormsg.c_str()), 0);
}

std::vector<std::string> split(const std::string str)
{
	std::vector<std::string> vector;
	std::istringstream input_str_stream(str);
	std::string cmd;
	while (input_str_stream >> std::skipws >> cmd)
		vector.push_back(cmd);
	return vector;
}

std::vector<std::string> ft_split(std::string str, char delimiter)
{
	std::vector<std::string> substrings;
	std::string substring = "";
	size_t i = 0;
	while ( i < str.length())
	{
		if (str[i] != delimiter)
		{
			substring = substring + str[i];
		}
		else
		{
			substrings.push_back(substring);
			while (str[i] == delimiter)
				i++;
			i--;
			substring = "";
		}
		i++;
	}
	substrings.push_back(substring);
	return (substrings);
}
