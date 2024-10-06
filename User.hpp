#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <sys/_types/_socklen_t.h>

class User
{
	public:
		User(int UserSocket);
		~User();
	private:
		std::string 		mUserName;
		int					mUserSocket;
		socklen_t			mUserAddrLen;
		struct sockaddr_in	mUserAddr;
	private:
		User(const User& rhs);
		User& operator=(const User& rhs);
};

#endif
