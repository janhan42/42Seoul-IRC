#pragma once
#ifndef SERVER_HPP
# define SERVER_HPP

/**
 * @file Server.hpp
 * @author jang hun han (janhan@student.42Seoul.kr)
 * @brief Server hpp 파일
 * @version 0.1
 * @date 2024-11-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <string>
#include <map>
#include <netinet/in.h>
#include <sys/_types/_socklen_t.h>
#include <sys/event.h>
#include "Channel.hpp"
#include "Command.hpp"
#include "Define.hpp"

class User;
class Channel;
class Command;

typedef std::map<int, class User*>			UserMap;
typedef std::map<std::string, Channel*>	ChannelMap;

class Server
{
	public:
		/* OCCF */
		Server(const std::string& port, const std::string& password);
		~Server();

		User										*mBot;
		void										Init(void);
		void										Run(void);

		//Getter
		UserMap&									GetUserList(void);
		std::string									GetPassWord(void);
		std::string									GetMessage(int fd);
		ChannelMap&									GetChannelList(void);

		//Ohters
		Channel*									FindChannel(std::string channelName);
		User*										FindUser(std::string& userName);
		User*										FindUser(int fd);
		void										RemoveChannel(std::string channelName);
		void										AppendNewChannel(std::string& channelName, int fd);
		void										SendBufferToUser();
		void										DeleteUserFromServer(int fd);


	private: // Init func
		unsigned short int							SetPortNum(const std::string& port);
		std::string									SetPassword(const std::string& password);
		void										SetServerSock(void);
		void										SetServerAddr(void);
		void										SetServerBind(void);
		void										SetServerListen(void);
		void										SetServerKqueue(void);
		void										SetBot(void);

	private: // ohter func
		int											RecvMessage(int fd);
		void										AddUser(int fd, User* newUser);
		void										AcceptUser(void);
		bool										CheckMessageEnds(int fd);
		void										DoCommand(int fd);
		void										HandleReadEvent(int fd);

	private: // member variables
		std::string									mPassword;					// Server 패스워드
		unsigned short int							mPort;						// Server 포트
		int											mServerSock;				// Server 소켓
		struct sockaddr_in							mServerAddr;				// Server Addr
		struct kevent								mUserEventList[MAX_EVENT];	// 서버 작동중 kevent list
		int											mKqFd;
		UserMap										mUserList;
		ChannelMap									mChannelList;
		std::string									mMessage[MAX_USER + 5];
		Command*									mCommand;

	private: // delete OCCF
		Server();
		Server(const Server& rhs);
		Server& operator=(const Server& rhs);
};

#endif
