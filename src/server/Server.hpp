/**
 * @file Server.hpp
 * @author jang hun han (janhan@student.42Seoul.kr)
 * @brief
 * @version 0.1
 * @date 2024-10-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <sys/_types/_sa_family_t.h>
#define SERVER_VERSION "v0.1"
#define CRLF "\r\n"

#include "../../common.hpp"

class User;
class Channel;
class Command;

class Server
{
	public:
		/* OCCF */
		Server(const int port, const std::string& password);
		~Server();

		bool	Init(void);
		void	Run(void);
	private:
		bool	SetSocket(void);
		bool	SetAddr(void);
		bool	SetListen(void);

	private: // server socket variables
		const	int 			mPort;
		const	std::string 	mPassword;
		int						mServerSock;
		struct	sockaddr_in		mServerAddr;
		int						mFdCount;
		struct pollfd			mFds[256];
		bool					mbRunning;

	private: // others variables
		int									mUserSock;
		struct	sockaddr_in					mUserAddr;
		socklen_t							mUserAddrSize;
		std::map<int, User>					mUsers;
		std::map<std::string, Channel *>	mChannels;
		std::string							mMessage[BUFFER_SIZE];
		int									mStrLen;
		Command*							mCommand;
	private: // delete OCCF
		Server();
		Server(const Server& rhs);
		Server* operator=(const Server& rhs);

};

#endif
