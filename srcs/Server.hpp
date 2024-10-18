#ifndef SERVER_HPP
# define SERVER_HPP

#include "../Common.hpp"
#include "User.hpp"
#include <netinet/in.h>
#include <sys/event.h>

class User;
class Channel;
class Command;

class Server
{
	public:	// OCCF
		Server(const int port, const std::string& password);
		~Server();

		void									Run(void);
		std::map<int, User>&					GetUserList(void);
		const std::string&						GetPasswrod(void);
		const std::string&						GetMessage(int fd);
		std::map<std::string, Channel *>&		GetChannelList(void);
		Channel*								FindChannle(std::string ChannelName);
		std::map<int, User>::iterator			FindUser(std::string UserName);
		void									RemoveChannel(std::string ChannelName);
		void									AppendNewChennel(std::string& ChannelName, int fd);

		User									*mBot;
		private: // Init Functions
		void									SetServerSock(void);
		void									SetServerAddr(void);
		void									SetServerBind(void);
		void									SetServerListen(void);
		void									SetBot(void);

		// others
		void									RecvMessage(int fd);
		void									AddUser(int fd);
		bool									CheckMessageEnds(int fd);
		void									DoCommand(int fd);

	private: // member variables
		const std::string						mPassword;
		const unsigned short int				mPort;
		int										mServerSock;
		struct sockaddr_in						mServerAddr;
		int										mfdCount;
		struct kevent							mKqueues[256];

		int										mUserSock;
		struct sockaddr_in						mUserAddr;
		socklen_t								mUserAddrSize;
		std::map<int, User>						mUserList;
		std::map<std::string, Channel *>		mChannelList;
		std::string								mMessage[BUF_SIZE];
		int										mStrlen;
		Command*								mCommand;

	private: // delete OCCF
		Server();
		Server(const Server& rhs);
		Server& operator=(const Server& rhs);
};

#endif
