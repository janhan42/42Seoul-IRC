#include "User.hpp"
#include <sys/socket.h>
#include <iostream>
#include <vector>
#include "Server.hpp"

#define BUFFER_MAX 1024

User::User(int client_fd)
{
	fd = client_fd;
	char buffer[BUFFER_MAX];
	int	 len = recv(fd, buffer, BUFFER_MAX, 0);
	if (len != 0)
	{
		std::vector<std::string> message_list = Server::split_message(buffer);
		std::vector<std::string>::iterator it;
		for (it = message_list.begin(); it != message_list.end(); it++)
		{
			std::cout << *it << std::endl;
			if (it->size() >= 4 && it->substr(0, 4) == "NICK")
			{
				nickname = it->substr(5);
				break;
			}
		}
	}
	if (nickname == "") nickname = "ycho2";
}

User::~User() {}

void User::set_nickname(std::string& name) { nickname = name; }
