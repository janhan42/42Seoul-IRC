#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <map>
#include <string>

#ifdef __linux__
#include <sys/epoll.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || \
	defined(__OpenBSD__)
#include <sys/_types/_socklen_t.h>
#include <sys/event.h>
#endif

#include "Channel.hpp"
#include "Command.hpp"

#define MAX_EVENT 10
#define BUF_SIZE  1024

class User;
class Channel;
class Command;

class Server
{
  public:
	/* OCCF */
	Server(const std::string& port, const std::string& password);
	~Server();

	User* mBot;
	void  Init(void);
	void  DeleteDisconnectedUser(int& i);
	void  SendBufferToUser();
	void  Run(void);

	// Getter
	std::map<int, User*>&			 GetUserList(void);
	std::string						 GetPassWord(void);
	std::string						 GetMessage(int fd);
	std::map<std::string, Channel*>& GetChannelList(void);
	int								 GetPollFd(void);

	// Others
	Channel*					   FindChannel(std::string channelName);
	std::map<int, User*>::iterator FindUser(std::string userName);
	void						   RemoveChannel(std::string channelName);
	void AppendNewChannel(std::string& channelName, int fd);

  private:
	unsigned short int SetPortNum(const std::string& port);
	std::string		   SetPassword(const std::string& password);
	void			   SetServerSock(void);
	void			   SetServerAddr(void);
	void			   SetServerBind(void);
	void			   SetServerListen(void);
	void			   SetEventLoop(void);
	void			   SetBot(void);

	int	 RecvMessage(int fd);
	void AddUser(int fd, User* newUser);
	void AcceptUser(void);
	bool CheckMessageEnds(int fd);
	void DoCommand(int fd);

  private:
	std::string		   mPassword;
	unsigned short int mPort;
	int				   mServerSock;
	struct sockaddr_in mServerAddr;
	struct sockaddr_in mUserAddr;
	int				mUserAddrLen;
	bool			   mbRunning;

#ifdef __linux__
	// epoll-specific members
	int				   mEpollFd;
	struct epoll_event mEvents[MAX_EVENT];
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || \
	defined(__OpenBSD__)
	// kqueue-specific members
	int			  mKqFd;
	struct kevent mServerEvent;
	struct kevent mUserEventList[MAX_EVENT];
#endif

	std::map<int, User*>			mUserList;
	std::map<std::string, Channel*> mChannelList;
	std::string						mMessage[BUF_SIZE];
	Command*						mCommand;

  private:
	Server();
	Server(const Server& rhs);
	Server& operator=(const Server& rhs);
};

#endif

