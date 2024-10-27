#include "./Server.hpp"
#include "User.hpp"
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <sys/event.h>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/syslimits.h>
#include <unistd.h>
#include <utility>

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
	std::map<std::string, Channel*>::iterator channelIter = mChannelList.begin();
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
	SetServerKqueue();
	SetBot();
	mbRunning = true;
}

void Server::Run()
{
	while (mbRunning)
	{
		// 루프당 mEventList 초기화
		memset(mUserEventList, 0, sizeof(mUserEventList));
		mEventCount = kevent(mKqFd, NULL, 0, mUserEventList, MAX_EVENT, NULL);
		if (mEventCount < 0)
			throw std::logic_error("ERROR:: Run():kevent() error");
		for (int i = 0; i < mEventCount; i++)
		{
			// New User Connection
			if (static_cast<int>(mUserEventList[i].ident) == mServerSock)
				AcceptUser();
			else
			{
				if (mUserEventList[i].filter & EVFILT_READ)
				{
					mStrLen = RecvMessage(mUserEventList[i].ident);
					std::cout << "User Sned: " << mMessage[mUserEventList[i].ident] << std::endl;
				}
				if (mStrLen <= 0) // from outside signal(ctrl+C, ...)
				{
					std::cout << "fd [" << mUserEventList[i].ident << "is quit connet" << std::endl;
					std::map<int, User*>::iterator userIt = mUserList.find(mUserEventList[i].ident); // find 안해도될듯
					if (userIt != mUserList.end()) // 접속 해제 유저 처리
					{
						struct kevent evSet;
						EV_SET(&evSet, userIt->second->GetUserFd(), EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0,NULL);
						kevent(mKqFd, &evSet, 1, NULL, 0, NULL);
						mMessage[userIt->first].clear();
						userIt->second->ClearUser();
						delete userIt->second;
						mUserList.erase(mUserEventList[i].ident);
						close(mUserEventList[i].ident);
						std::cout << "User Delete [" << mUserEventList[i].ident << "]" << std::endl;
					}
				}
				else
				{
					if (CheckMessageEnds(mUserEventList[i].ident))
					{
						std::cout << "recv fd [" << mUserEventList[i].ident << "]:" << mMessage[mUserEventList[i].ident] << std::endl;
						// 명령어 실행 분기
						DoCommand(mUserEventList[i].ident);
						// User Fd 메시지 버퍼 초기화
						mMessage[mUserEventList[i].ident] = "";
						std::cout << "User read buffer claer after: " << mMessage[mUserEventList[i].ident] << std::endl;
					}
				}
			}
		}

		std::map<int, User*>::iterator It = mUserList.begin();
		for (; It != mUserList.end(); It++)
		{
			if (It->second->GetUserRecvBuf().length() > 0)
			{
				std::cout << "Server Send fd[" << It->second->GetUserFd() << "]: "<< It->second->GetUserRecvBuf() << std::endl;
				send(It->first, It->second->GetUserRecvBuf().c_str(), It->second->GetUserRecvBuf().length(), 0);
				It->second->ClearUserRecvBuf();
			}
		}
	}
}

std::map<int, User*>&	Server::GetUserList()
{
	return (mUserList);
}

std::string	Server::GetPassWord()
{
	return (mPassword);
}

std::string	Server::GetMessage(int fd)
{
	return (mMessage[fd]);
}

std::map<std::string, Channel *>&	Server::GetChannelList()
{
	return (mChannelList);
}

int	Server::GetKqFd()
{
	return (mKqFd);
}

Channel*	Server::FindChannel(std::string channelName)
{
	std::map<std::string, Channel*>::iterator It = mChannelList.find(channelName);
	if (It == mChannelList.end())
		return (NULL);
	return (It->second);
}

bool containsWordWithoutAnsi(const std::string& input, const std::string& word)
{
	std::string cleaned;
	bool in_ansi = false;

	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] == '\033' && input[i + 1] == '[') {
			in_ansi = true;
		}
		if (!in_ansi) {
			cleaned += input[i];
		}
		if (in_ansi && input[i] == 'm')
		{
			in_ansi = false;
		}
	}
	return cleaned.find(word) != std::string::npos;
}

std::map<int, User*>::iterator	Server::FindUser(std::string userName)
{
	std::map<int, User*>::iterator it = mUserList.begin();
	for (; it != mUserList.end(); it++)
	{
		if (containsWordWithoutAnsi(it->second->GetNickName(), userName))
			return (it);
	}
	return (it);
}

void	Server::RemoveChannel(std::string channelName)
{
	mChannelList.erase(channelName);
}

void	Server::AppendNewChannel(std::string& channelName, int fd)
{
	mChannelList.insert(std::make_pair(channelName, new Channel(channelName, fd)));
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
	while (password[strIndex])
		strIndex++;
	if (strIndex >= 9)
		throw std::logic_error("ERROR:: Password is under 9 digit");
	return (password);
}

void Server::SetServerSock()
{
	int option = 1;

	mServerSock = socket(PF_INET, SOCK_STREAM, 0);
	if (mServerSock == -1)
		throw std::logic_error ("ERROR:: socket() error");
	if (setsockopt(mServerSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) != 0)
		throw std::logic_error ("ERROR:: setsockopt() fail");
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
	if (bind(mServerSock, (struct sockaddr *)&mServerAddr, sizeof(mServerAddr)) == -1)
		throw std::logic_error("ERROR:: bind() error");
}

void Server::SetServerListen()
{
	if (listen(mServerSock, 15) == -1)
		throw std::logic_error("ERROR:: listen() error");
	fcntl(mServerSock, F_SETFL, O_NONBLOCK);
	// F_SETFL:		re-set file fd flag with arg
	// O_NONBLOCK:	set fd NONBLOCK
}

void Server::SetServerKqueue()
{
	mKqFd = kqueue();
	if (mKqFd < 0)
		throw std::logic_error("ERROR:: kqueue() error");
	EV_SET(&mServerEvent, mServerSock, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(mKqFd, &mServerEvent, 1, NULL, 0, NULL);
}

void Server::SetBot()
{
	mBot = new User(-1);
	mBot->MakeUserToBot();
	mUserList.insert(std::make_pair(-1, mBot));
}

int Server::RecvMessage(int fd)
{
	char buf[2];
	ssize_t read_len;
	int ret = 0;
	while ((read_len = recv(fd, buf, 1, 0)) == 1)
	{
		ret++;
		mMessage[fd] += buf[0];
		if (buf[0] == '\n')
			break;
	}
	return (ret);
}

void	Server::AddUser(int fd, User* newUser)
{
	mUserList.insert(std::make_pair(fd, newUser));
}

void Server::AcceptUser()
{
	mUserSock = accept(mServerSock, (struct sockaddr *)&mUserAddr, &mUserAddrLen);
	if (mUserSock < 0)
		std::logic_error("ERROR:: AcceptUser accept() failed");
	fcntl(mUserSock, F_SETFL, O_NONBLOCK);
	std::cout << "New User Conneted fd [" << mUserSock << "]" << std::endl;

	User* newUser = new User(mUserSock);
	struct kevent evSet;
	EV_SET(&evSet, mUserSock, EVFILT_READ | EVFILT_WRITE, EV_ADD, 0, 0, newUser);
	kevent(mKqFd, &evSet, 1, NULL, 0, NULL);
	AddUser(mUserSock, newUser);
}

bool	Server::CheckMessageEnds(int fd)
{
	if (mMessage[fd].length() == 1 && mMessage[fd][0] == '\n')
		mMessage[fd] = "";
	if (mMessage[fd][mMessage[fd].length() - 1] == '\n'
	&& mMessage[fd][mMessage[fd].length() -2] == '\r')
		return (true);
	return (false);
}

void Server::DoCommand(int fd)
{
	std::cout << "Docommand  FD : "<< fd << std::endl;
	mCommand->Run(fd);
}
