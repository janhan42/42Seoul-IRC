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

/* OCCF */
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


/**
 * @brief
 * IRC 서버 구동을 위해 초기화 하는 함수
 */
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

/**
 * @brief
 * 서버의 메인 구동 함수
 */
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
			{
				AcceptUser();
				continue;
			}
			else if (mUserEventList[i].filter & EVFILT_READ)
			{
				mStrLen = RecvMessage(mUserEventList[i].ident);
				if (mStrLen <= 0) // from outside signal(ctrl+C, ...)
					DeleteUserFromServer(mUserEventList[i].ident);
				if (CheckMessageEnds(mUserEventList[i].ident)) // CRLF 체크
					DoCommand(mUserEventList[i].ident);
			}
			// Send Message
			SendBufferToUser();
		}
	}
}

/* Getter */
/**
 * @brief
 * 서버에 접속해있는 유저 리스트를 반환하는 함수
 * @return std::map<int, User*>&
 */
std::map<int, User*>&	Server::GetUserList()
{
	return (mUserList);
}

/**
 * @brief
 * 서버에 설정되어 있는 비밀번호를 반환하는 함수
 * @return std::string
 */
std::string	Server::GetPassWord()
{
	return (mPassword);
}

/**
 * @brief
 * std::string 배열에서 자신의 Fd 메세지를 리턴하는 함수
 * @param fd
 * @return std::string
 */
std::string	Server::GetMessage(int fd)
{
	return (mMessage[fd]);
}

/**
 * @brief
 * 서버에 열려있는 채널 리스트를 반환하는 함수
 * @return std::map<std::string, Channel *>&
 */
std::map<std::string, Channel *>&	Server::GetChannelList()
{
	return (mChannelList);
}

/**
 * @brief
 * 서버의 kqueue FD를 반환하는 함수
 * @return int
 */
int	Server::GetKqFd()
{
	return (mKqFd);
}

/* Others */
/**
 * @brief
 * 현재 서버에 열려있는 채널리스트에서 채널명을 기반으로 iterator를 찾는 함수
 * @param channelName
 * @return Channel*
 */
Channel*	Server::FindChannel(std::string channelName)
{
	std::map<std::string, Channel*>::iterator It = mChannelList.find(channelName);
	if (It == mChannelList.end())
		return (NULL);
	return (It->second);
}

/**
 * @brief
 * 현재 서버에 접속해있는 유저 리스트에서 유저 이름을 기반으로 iterator를 찾는 함수
 * @param userName
 * @return std::map<int, User*>::iterator
 */
std::map<int, User*>::iterator	Server::FindUser(std::string userName)
{
	std::map<int, User*>::iterator it = mUserList.begin();
	for (; it != mUserList.end(); it++)
	{
		if (it->second->GetNickName() == userName)
			return (it);
	}
	return (it);
}

/**
 * @brief
 * 채널 이름을 기반으로 리스트에서 지우는 함수
 * @param channelName
 */
void	Server::RemoveChannel(std::string channelName)
{
	mChannelList.erase(channelName);
}

/**
 * @brief
 * 채널을 새로 만들어서 채널리스트에 추가하는 함수
 * @param channelName
 * @param fd
 */
void	Server::AppendNewChannel(std::string& channelName, int fd)
{
	mChannelList.insert(std::make_pair(channelName, new Channel(channelName, fd)));
}

/**
 * @brief
 * 나가는 유저에 대한 처리를 통합적으로 해주는 함수
 * @param fd
 */
void Server::DeleteUserFromServer(int fd)
{
	std::cout << "fd [" << fd << "]is quit connet" << std::endl;
	std::map<int, User*>::iterator userIt =
		mUserList.find(fd);
	if (userIt != mUserList.end())	// 접속 해제 유저 처리
	{
		struct kevent evSet;
		EV_SET(&evSet, userIt->second->GetUserFd(), EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		kevent(mKqFd, &evSet, 1, NULL, 0, NULL);
		mMessage[userIt->first].clear();
		delete userIt->second;
		mUserList.erase(fd);
		close(fd);
		std::cout << "User Deleted [" << fd << "]" << std::endl;
	}
}

/**
 * @brief
 * 현재 유저의 SendBuf에 대한 send처리와 오류 및 클리어 처리를 하는 함수
 */
void Server::SendBufferToUser()
{
	std::map<int, User*>::iterator It = mUserList.begin();
	for (; It != mUserList.end(); It++)
	{
		User* usr = It->second;
		if (!usr->GetUserSendBuf().empty() && usr->GetUserFd() != -1)
		{
			/* TESTOUTPUT */
			std::cout << "Server Send : " << usr->GetUserSendBuf() << std::endl;
			/* END */
			int sent_byte = send(usr->GetUserFd(), usr->GetUserSendBuf().c_str(), usr->GetUserSendBuf().length(), 0);
			if (sent_byte > 0)	// 전송 성공하면
				usr->ClearUserSendBuf(sent_byte);
			else
				std::cout << "send error on fd [" << usr->GetUserFd() << "]" << std::endl;
		}
	}
}

/* Init func */
/**
 * @brief
 * 서버의 port를 Init하는 함수
 * @param port
 * @return unsigned short int
 */
unsigned short int Server::SetPortNum(const std::string& port)
{
	int ret;
	int strIndex = 0;
	while (port[strIndex])
	{
		if (!('0' <= port[strIndex] || port[strIndex] <= '9'))
			throw std::out_of_range("ERROR:: Port Number is only number");
		strIndex++;
	}
	ret = std::atoi(port.c_str());
	if (!(1024 <= ret && ret <= 65535))
		throw std::logic_error("ERROR:: Port Number range is 1024~65535");
	return (static_cast<unsigned short int>(ret));
}

/**
 * @brief
 * 서버의 Password를 init하는 함수
 * @param password
 * @return std::string
 */
std::string Server::SetPassword(const std::string& password)
{
	int strIndex = 0;
	while (password[strIndex])
		strIndex++;
	if (strIndex >= 9)
		throw std::logic_error("ERROR:: Password is under 9 length");
	return (password);
}

/**
 * @brief
 * 서버의 소켓을 Init하는 함수
 */
void Server::SetServerSock()
{
	int option = 1;

	mServerSock = socket(PF_INET, SOCK_STREAM, 0);
	if (mServerSock == -1)
		throw std::logic_error ("ERROR:: socket() error");
	if (setsockopt(mServerSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) != 0)
		throw std::logic_error ("ERROR:: setsockopt() fail");
}

/**
 * @brief
 * 서버의 addr구조체를 init하는 함수
 */
void Server::SetServerAddr()
{
	memset(&mServerAddr, 0, sizeof(mServerAddr));
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = INADDR_ANY;
	mServerAddr.sin_port = htons(mPort);
}

/**
 * @brief
 * 서버의 소켓에 addr을 bind하는 함수
 */
void Server::SetServerBind()
{
	if (bind(mServerSock, (struct sockaddr *)&mServerAddr, sizeof(mServerAddr)) == -1)
		throw std::logic_error("ERROR:: bind() error");
}

/**
 * @brief
 * 서버의 listen을 설정하는 함수
 */
void Server::SetServerListen()
{
	if (listen(mServerSock, 15) == -1)
		throw std::logic_error("ERROR:: listen() error");
	fcntl(mServerSock, F_SETFL, O_NONBLOCK);
	// F_SETFL:		re-set file fd flag with arg
	// O_NONBLOCK:	set fd NONBLOCK
}

/**
 * @brief
 * 서버의 kqueue를 설정하는 함수
 */
void Server::SetServerKqueue()
{
	mKqFd = kqueue();
	if (mKqFd < 0)
		throw std::logic_error("ERROR:: kqueue() error");
	EV_SET(&mServerEvent, mServerSock, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(mKqFd, &mServerEvent, 1, NULL, 0, NULL);
}

/**
 * @brief
 * 서버가 가지고 있는 Bot을 설정하는 함수
 */
void Server::SetBot()
{
	mBot = new User(-1);
	mBot->MakeUserToBot();
	mUserList.insert(std::make_pair(-1, mBot));
}

/* Ohter Func */
/**
 * @brief
 * fd에서 들어온 메세지를 recv해서 mMessage 배열에 저장하는 함수
 * @param fd
 * @return int
 */
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

/**
 * @brief
 * 새로운 유저를 pair를 만들어서 저장하는 함수
 * @param fd
 * @param newUser
 */
void	Server::AddUser(int fd, User* newUser)
{
	mUserList.insert(std::make_pair(fd, newUser));
}

/**
 * @brief
 * 서버에 새로운 유저가 들어왔을때 유저생성을 통합적으로 하는 함수
 */
void Server::AcceptUser()
{
	User* newUser = NULL;
	mUserSock = accept(mServerSock, (struct sockaddr *)&mUserAddr, &mUserAddrLen);
	if (mUserSock < 0)
		throw std::logic_error("ERROR:: AcceptUser accept() failed");
	fcntl(mUserSock, F_SETFL, O_NONBLOCK);
	newUser = new User(mUserSock);
	if (newUser == NULL)
		throw std::logic_error("ERROR:: AcceptUser new User failed");
	struct kevent evSet;
	EV_SET(&evSet, mUserSock, EVFILT_READ | EVFILT_WRITE, EV_ADD, 0, 0, newUser);
	kevent(mKqFd, &evSet, 1, NULL, 0, NULL);
	AddUser(mUserSock, newUser);
}

/**
 * @brief
 * 현재 User fd의 메세지가 완전히 들어왔는지 체크하는 함수
 * @param fd
 * @return true
 * @return false
 */
bool	Server::CheckMessageEnds(int fd)
{
	if (mMessage[fd].length() == 1 && mMessage[fd][0] == '\n')
		mMessage[fd] = "";
	if (mMessage[fd][mMessage[fd].length() - 1] == '\n'
	&& mMessage[fd][mMessage[fd].length() -2] == '\r')
		return (true);
	return (false);
}

/**
 * @brief
 * 유저메세지를 기반으로 Command를 파싱 및 실행 처리를 하는 함수
 * @param fd
 */
void Server::DoCommand(int fd)
{
	std::cout << "Docommand NICK : [" << mUserList.find(fd)->second->GetNickName() << "]" << std::endl;
	std::cout << "Recv Send: " << mMessage[fd] << std::endl;
	mCommand->Run(fd);
	mMessage[fd] = "";
}

