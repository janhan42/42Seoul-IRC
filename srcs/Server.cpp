
#include "./Server.hpp"
#include <string.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <utility>
#include "User.hpp"

Server::Server(const std::string& port, const std::string& password)
{
	mPort = SetPortNum(port);
	mPassword = SetPassword(password);
	mUserAddrLen = sizeof(mUserAddr);
	mCommand = new Command(*this);
	mbRunning = false;
}

Server::~Server()
{
	close(mServerSock);
	std::map<int, User*>::iterator iter = mUserList.begin();
	for (; iter != mUserList.end(); iter++)
	{
		close(iter->first);
		delete iter->second;
	}
	std::map<std::string, Channel*>::iterator channelIter =
		mChannelList.begin();
	for (; channelIter != mChannelList.end(); channelIter++)
	{
		delete channelIter->second;
	}
	mUserList.clear();
	mChannelList.clear();
	delete mCommand;
	delete mBot;
}

void Server::Init()
{
	SetServerSock();
	SetServerAddr();
	SetServerBind();
	SetServerListen();
	SetServerEpoll();
	SetBot();
	mbRunning = true;
}

void Server::Run()
{
	while (mbRunning)
	{
		memset(mEvents, 0, sizeof(mEvents));
		mEventCount = epoll_wait(mEpollFd, mEvents, MAX_EVENT, -1);
		if (mEventCount < 0)
			throw std::logic_error("ERROR:: Run():epoll_wait() error");
		for (int i = 0; i < mEventCount; i++)
		{
			if (mEvents[i].data.fd == mServerSock)
			{
				AcceptUser();
				continue;
			}
			else if (mEvents[i].events & EPOLLIN)
			{
				mStrLen = RecvMessage(mEvents[i].data.fd);

				if (mStrLen <= 0)
					DeleteDisconnectedUser(i);
				else
					std::cout << "User Send: " << mMessage[mEvents[i].data.fd] << std::endl;

				if (CheckMessageEnds(mEvents[i].data.fd))
					DoCommand(mEvents[i].data.fd);
			}
			SendBufferToUser();
		}
	}
}

std::map<int, User*>& Server::GetUserList() { return (mUserList); }

std::string Server::GetPassWord() { return (mPassword); }

std::string Server::GetMessage(int fd) { return (mMessage[fd]); }

std::map<std::string, Channel*>& Server::GetChannelList()
{
	return (mChannelList);
}

int Server::GetEpollFd() { return (mEpollFd); }

Channel* Server::FindChannel(std::string channelName)
{
	std::map<std::string, Channel*>::iterator It =
		mChannelList.find(channelName);
	if (It == mChannelList.end()) return (NULL);
	return (It->second);
}

bool containsWordWithoutAnsi(const std::string& input, const std::string& word)
{
	std::string cleaned;
	bool		in_ansi = false;

	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] == '\033' && input[i + 1] == '[')
		{
			in_ansi = true;
		}
		if (!in_ansi)
		{
			cleaned += input[i];
		}
		if (in_ansi && input[i] == 'm')
		{
			in_ansi = false;
		}
	}
	return cleaned.find(word) != std::string::npos;
}

std::map<int, User*>::iterator Server::FindUser(std::string userName)
{
	std::map<int, User*>::iterator it = mUserList.begin();
	for (; it != mUserList.end(); it++)
	{
		if (containsWordWithoutAnsi(it->second->GetNickName(),
									userName))
			return (it);
	}
	return (it);
}

void Server::RemoveChannel(std::string channelName)
{
	mChannelList.erase(channelName);
}

void Server::AppendNewChannel(std::string& channelName, int fd)
{
	mChannelList.insert(
		std::make_pair(channelName, new Channel(channelName, fd)));
}

unsigned short int Server::SetPortNum(const std::string& port)
{
	int ret;
	int strIndex = 0;
	while (port[strIndex])
	{
		if (!('0' <= port[strIndex] || port[strIndex] <= '0'))
			throw std::out_of_range("ERROR:: Port Number is only number");
		strIndex++;
	}
	ret = std::atoi(port.c_str());
	if (!(1024 <= ret && ret <= 65535))
		throw std::logic_error("ERROR:: Port Number range is 1024~65535");
	return (static_cast<unsigned short int>(ret));
}

std::string Server::SetPassword(const std::string& password)
{
	int strIndex = 0;
	while (password[strIndex]) strIndex++;
	if (strIndex >= 9)
		throw std::logic_error("ERROR:: Password is under 9 digit");
	return (password);
}

void Server::SetServerSock()
{
	int option = 1;

	mServerSock = socket(PF_INET, SOCK_STREAM, 0);
	if (mServerSock == -1) throw std::logic_error("ERROR:: socket() error");
	if (setsockopt(mServerSock, SOL_SOCKET, SO_REUSEADDR, &option,
				   sizeof(option)) != 0)
		throw std::logic_error("ERROR:: setsockopt() fail");
}

void Server::SetServerAddr()
{
	memset(&mServerAddr, 0, sizeof(mServerAddr));
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = INADDR_ANY;
	mServerAddr.sin_port = htons(mPort);
}

void Server::SetServerBind()
{
	if (bind(mServerSock, (struct sockaddr*)&mServerAddr,
			 sizeof(mServerAddr)) == -1)
		throw std::logic_error("ERROR:: bind() error");
}

void Server::SetServerListen()
{
	if (listen(mServerSock, 15) == -1)
		throw std::logic_error("ERROR:: listen() error");
	fcntl(mServerSock, F_SETFL, O_NONBLOCK);
}

void Server::SetServerEpoll()
{
	mEpollFd = epoll_create1(0);
	if (mEpollFd < 0) throw std::logic_error("ERROR:: epoll_create1() error");
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = mServerSock;
	if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mServerSock, &ev) < 0)
		throw std::logic_error("ERROR:: epoll_ctl() error");
}

void Server::SetBot()
{
	mBot = new User(-1);
	mBot->MakeUserToBot();
	mUserList.insert(std::make_pair(-1, mBot));
}

int Server::RecvMessage(int fd)
{
	char	buf[2];
	ssize_t read_len;
	int		ret = 0;
	while ((read_len = recv(fd, buf, 1, 0)) == 1)
	{
		ret++;
		mMessage[fd] += buf[0];
		if (buf[0] == '\n') break;
	}
	return (ret);
}

void Server::AddUser(int fd, User* newUser)
{
	mUserList.insert(std::make_pair(fd, newUser));
}

void Server::AcceptUser()
{
	mUserSock =
		accept(mServerSock, (struct sockaddr*)&mUserAddr, &mUserAddrLen);
	if (mUserSock < 0) std::logic_error("ERROR:: AcceptUser accept() failed");
	fcntl(mUserSock, F_SETFL, O_NONBLOCK);
	std::cout << "New User Conneted fd [" << mUserSock << "]" << std::endl;

	User*		  newUser = new User(mUserSock);
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = mUserSock;
	if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mUserSock, &ev) < 0)
		throw std::logic_error("ERROR:: epoll_ctl() error");
	AddUser(mUserSock, newUser);
}

bool Server::CheckMessageEnds(int fd)
{
	if (mMessage[fd].length() == 1 && mMessage[fd][0] == '\n')
		mMessage[fd] = "";
	if (mMessage[fd][mMessage[fd].length() - 1] == '\n' &&
		mMessage[fd][mMessage[fd].length() - 2] == '\r')
		return (true);
	return (false);
}

void Server::DoCommand(int fd)
{
	std::cout << "Docommand  FD : " << fd << std::endl;
	std::cout << "fd [" << fd << "] command exec:" << mMessage[fd] << std::endl;
	mCommand->Run(fd);
	mMessage[fd] = "";
}

void Server::DeleteDisconnectedUser(int& i)
{
	std::cout << "fd [" << mEvents[i].data.fd << "]is quit connet"
			  << std::endl;
	std::map<int, User*>::iterator userIt =
		mUserList.find(mEvents[i].data.fd);
	if (userIt != mUserList.end())
	{
		if (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, userIt->second->GetUserFd(), NULL) < 0)
			throw std::logic_error("ERROR:: epoll_ctl() error");
		mMessage[userIt->first].clear();
		delete userIt->second;
		mUserList.erase(mEvents[i].data.fd);
		close(mEvents[i].data.fd);
		std::cout << "User Deleted [" << mEvents[i].data.fd << "]"
				  << std::endl;
	}
}

void Server::SendBufferToUser()
{
	std::map<int, User*>::iterator It = mUserList.begin();
	for (; It != mUserList.end(); It++)
	{
		User* usr = It->second;
		if (!usr->GetUserSendBuf().empty() && usr->GetUserFd() != -1)
		{
			int sent_byte =
				send(usr->GetUserFd(), usr->GetUserSendBuf().c_str(),
					 usr->GetUserSendBuf().length(), 0);
			std::cout << usr->GetUserSendBuf() << std::endl;
			if (sent_byte > 0)
				usr->ClearUserSendBuf(sent_byte);
			else
				std::cout << "send error on fd [" << usr->GetUserFd() << "]"
						  << std::endl;
		}
	}
}
