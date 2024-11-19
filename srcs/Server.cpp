#include "./Server.hpp"
#include "User.hpp"
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <sys/_types/_socklen_t.h>
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
	mCommand = new Command(*this);
}

Server::~Server()
{
	close(mServerSock);
	UserMap::iterator iter = mUserList.begin();
	for (; iter != mUserList.end(); iter++)
	{
		close(iter->first);
		delete iter->second;
	}
	ChannelMap::iterator channelIter = mChannelList.begin();
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
}

/**
 * @brief
 * 서버의 메인 구동 함수
 */
void Server::Run()
{
	while (true)
	{
		// 루프당 mEventList 초기화
		memset(mUserEventList, 0, sizeof(mUserEventList));
		int mEventCount = kevent(mKqFd, NULL, 0, mUserEventList, MAX_EVENT, NULL);
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
				HandleReadEvent(mUserEventList[i].ident);
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
UserMap&	Server::GetUserList()
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


/* Others */
/**
 * @brief
 * 현재 서버에 열려있는 채널리스트에서 채널명을 기반으로 iterator를 찾는 함수
 * @param channelName
 * @return Channel*
 */
Channel*	Server::FindChannel(std::string channelName)
{
	ChannelMap::iterator It = mChannelList.find(channelName);
	if (It == mChannelList.end())
		return (NULL);
	return (It->second);
}

/*
 * 서버의 UserMap에서 이름으로 유저를 찾고 유저 포인터 반환
 * 없으면 널포인터 반환
 */
User*	Server::FindUser(std::string& name)
{
	UserMap::iterator it = mUserList.begin();
	for (; it != mUserList.end(); it++)
	{
		if (it->second->GetNickName() == name)
			return (it->second);
	}
	return (NULL);
}

/*
 * 서버의 UserMap에서 fd로 유저를 찾고 유저 포인터 반환
 * 없으면 널포인터 반환
 */
User*	Server::FindUser(int fd)
{
	UserMap::iterator it = mUserList.find(fd);
	if (it == mUserList.end())
		return (NULL);
	return (it->second);
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
 * 유저를 서버에서 삭제하고 유저 관련 이벤트를 삭제
 * @param fd
 */
void Server::DeleteUserFromServer(int fd)
{
	std::cout << "fd [" << fd << "] connection lost" << std::endl;
	class User* user = FindUser(fd);
	if (user!= NULL)// 접속 해제 유저 처리
	{
		struct kevent evSet;
		EV_SET(&evSet, user->GetUserFd(), EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		kevent(mKqFd, &evSet, 1, NULL, 0, NULL);
		mMessage[fd].clear();
		delete user;
		mUserList.erase(fd);
		close(fd);
		std::cout << "User Deleted [" << fd << "]" << std::endl;
	}
}

/**
 * @brief
 * read 가능 이벤트가 발생했을 때 유저를 삭제할지 커맨들을 실행할지
 * 분기하는 함수
 */
void Server::HandleReadEvent(int fd)
{
	int recvLen = RecvMessage(fd);
	if (recvLen <= 0) // from outside signal(ctrl+c, ...)
		DeleteUserFromServer(fd);
	else if (CheckMessageEnds(fd))
		DoCommand(fd);
}

/**
 * @brief
 * 현재 유저의 SendBuf에 대한 send처리와 오류 및 클리어 처리를 하는 함수
 */
void Server::SendBufferToUser()
{
	UserMap::iterator it = mUserList.begin();
	for (; it != mUserList.end(); it++)
	{
		int		user_fd = it->first;
		User*	user = it->second;

		if (!user->GetUserSendBuf().empty() && user_fd != -1)
		{
			/* TESTOUTPUT */
			std::cout << "Server Send : " << user->GetUserSendBuf() << std::endl;
			/* END */
			int sent_byte = send(user_fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
			if (sent_byte > 0)	// 전송 성공하면
				user->ClearUserSendBuf(sent_byte);
			else
				std::cout << "send error on fd [" << user_fd << "]" << std::endl;
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
	struct kevent evSet;
	EV_SET(&evSet, mServerSock, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(mKqFd, &evSet, 1, NULL, 0, NULL);
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
	struct sockaddr_in	userAddr;
	socklen_t			userAddrLen = sizeof(userAddr);
	int mUserSock = accept(mServerSock, (struct sockaddr *)&userAddr, &userAddrLen);
	if (mUserSock < 0)
		throw std::logic_error("ERROR:: AcceptUser accept() failed");
	fcntl(mUserSock, F_SETFL, O_NONBLOCK);
	User* newUser = new User(mUserSock);
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
	std::cout << "Docommand NICK : [" << FindUser(fd)->GetNickName() << "]" << std::endl;
	std::cout << "Recv Send: " << mMessage[fd] << std::endl;
	mCommand->Run(fd);
	mMessage[fd] = "";
}

