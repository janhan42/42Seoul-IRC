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
#pragma once
#ifndef SERVER_HPP
# define SERVER_HPP

#define SERVER_VERSION "v0.1"
#define CRLF "\r\n"

#include "../../Common.hpp"
#include "../irc/Channel.hpp"
#include "../irc/Command.hpp"

class User;
class Channel;
class Command;

class Server
{
	public:
		/* OCCF */
		Server(const int port, const std::string& password);
		~Server();

		User*					mBot;

		bool	Init(void);
		void	Run(void);
	public: // Getter
		std::map<int, User>&				GetUsers(void);
		std::string							GetPassWord(void);
		std::string							GetMessage(int fd);
		std::map<std::string, Channel *>&	GetChannelList(void);
	private: // init Setter
		bool								SetSocket(void);
		bool								SetAddr(void);
		bool								SetListen(void); // Bind도 같이 함
		bool								SetBot(void);

	private: // Run() Utils
		void								AppendNewChannel(const std::string& channelname, int fd);
		int									RecvMessage(int fd);
		bool								CheckMessageEnds(int fd);
		void								DoCommand(int fd);
		void								AddUser(int fd);
		Channel*							FindChannel(std::string channelName);
		std::map<int, User>::iterator		FindUser(std::string nickName);
		void								RemoveChannel(std::string channelName);
	private: // server socket variables
		const	int 						mPort;			// 서버 포트
		const	std::string 				mPassword;		// 서버 패스워드
		int									mServerSock;	// 서버 소켓
		struct	sockaddr_in					mServerAddr;	// 서버 sockaddr 구조체
		int									mFdCount;		// 서버 fdCount
		struct pollfd						mFds[256];		// 서버 Poll fd 구조체
		bool								mbRunning;		// 서버 작동 상태

	private: // others variables
		int									mUserSock;
		struct	sockaddr_in					mUserAddr;
		socklen_t							mUserAddrSize;
		std::map<int, User>					mUsers;			// 접속해있는 UserList
		std::map<std::string, Channel *>	mChannels;		// 서버에 있는 ChannelList
		std::string							mMessage[BUFFER_SIZE]; // recv버퍼
		int									mStrLen;
		Command*							mCommand;		// Command 클래스
	private: // delete OCCF
		Server();
		Server(const Server& rhs);
		Server* operator=(const Server& rhs);

};

#endif
