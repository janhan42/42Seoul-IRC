/**
 * @file Server.cpp
 * @author jang hun han (janhan@student.42Seoul.kr)
 * @brief
 * @version 0.1
 * @date 2024-10-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "Server.hpp"
#include <sys/fcntl.h>

Server::Server(const int port, const std::string& password)
: mPort(port)
, mPassword(password)
, mFdCount(1)
, mbRunning(false)
{
	mUserAddrSize = sizeof(mUserAddr);
	mCommand = new Command(*this);
}

Server::~Server()
{
	close(mServerSock);
	for (std::map<int, User>::iterator it = mUsers.begin(); it != mUsers.end(); ++it)
	{
		close(it->first);
	}

	for (std::map<std::string, Channel *>::iterator it = mChannels.begin(); it != mChannels.end(); ++it)
	{
		delete it->second;
	}
	mUsers.clear();
	mChannels.clear();
	delete mCommand;
	delete mBot;
}

void Server::Run()
{
	while(mbRunning)
	{
		/* poll: events detection from socket-fds */
		int ret_poll = poll(mFds, mFdCount, -1);

		/* No Events */
		if (ret_poll < 0) // Poll Error
		{
			/*
				전체적인 에러 처리 방식을 생각 해봐야 할듯
			 */
			std::cerr << "ERROR: poll() error" << std::endl;
			break;
		}
		else if (ret_poll == 0)
			continue ;

		/* Event occurred */
		for (int i = 0; i < mFdCount; i++)
		{
			if (mFds[i].revents == 0)
				continue;
			if (mFds[i].fd == mServerSock) // New Client Authentication
			{
				mUserSock = accept(mServerSock, (struct sockaddr *)&mUserSock, &mUserAddrSize);
				fcntl(mUserSock, F_SETFL, O_NONBLOCK);
				mFds[mFdCount].fd = mUserSock;
				mFds[mFdCount].events = POLL_IN;
				mFdCount++;
				AddUser(mUserSock);
				std::cout << "User FD [" << mUserSock << "] is connected" << std::endl;
			}
			else
			{
				if (mFds[i].revents & POLLIN)
				{
					mStrLen = RecvMessage(mFds[i].fd);
					if (mStrLen <= 0) // from outside signal(ctrl+c, ...)
					{
						std::cout << "User FD[" << mFds[i].fd << "] is quit connect" << std::endl;
						std::map<int, User>::iterator UserIt = mUsers.find(mFds[i].fd);
						if (UserIt != mUsers.end())
						{
							UserIt->second.ClearUser();
							mUsers.erase(mFds[i].fd);
							close(mFds[i].fd);
						}
					}
					else
					{
						if (CheckMessageEnds(mFds[i].fd))
						{
							std::cout << "FD [" << mFds[i].fd << "]: " << mMessage[mFds[i].fd];
							DoCommand(mFds[i].fd);
							mMessage[mFds[i].fd] = "";
						}
					}
				}
			}
		}

		/* After DoCmmand: send Results to User */
		std::map<int, User>::iterator It = mUsers.begin();
		for (; It != mUsers.end(); It++)
		{
			if (It->second.GetUserRecvBuf().length() > 0)
			{
				send(It->first,
					It->second.GetUserRecvBuf().c_str(),
					It->second.GetUserRecvBuf().length(), 0);
				It->second.ClearUserRecvBuf();
			}
		}
	}
}

bool Server::Init()
{
	if (SetSocket()
	|| SetAddr()
	|| SetListen()
	|| SetBot())
	{
		std::cerr << "ERROR: Init Failed" << std::endl;
		return (EXIT_FAILURE);
	}
	memset(mFds, 0, sizeof(mFds));
	mFds[0].fd = mServerSock;
	mFds[0].events = POLLIN;
	mbRunning = true;	// Init 성공시 Running 가능
	return (EXIT_SUCCESS);
}
/**************************************************** */
// Init()

/**
 * @brief
 * Server클래스의 mServerSock을 세팅한다
 */
bool Server::SetSocket()
{
	int	opt = 1;
	mServerSock = socket(PF_INET, SOCK_STREAM, 0);
	setsockopt(mServerSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (mServerSock == -1)
	{
		std::cerr << "ERROR: socket() error" << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/**
 * @brief
 * Server클래스의 mServerAddr을 세팅 및 bind한다
 */
bool Server::SetAddr()
{
	memset(&this->mServerAddr, 0, sizeof(mServerAddr));
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = INADDR_ANY;
	mServerAddr.sin_port = htons(mPort);
	if (bind(mServerSock, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr)) == -1)
	{
		std::cerr << "ERROR: bind() error" << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/**
 * @brief
 * Server클래스의 listen을 설정한다
 * @return true
 * @return false
 */
bool Server::SetListen()
{
	if (listen(mServerSock, 15) == -1)
	{
		std::cerr << "ERROR: listen() error" << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/**
 * @brief
 * 보너스용 User클래스의 봇을 설정한다.
 * @return true
 * @return false
 */
bool Server::SetBot()
{
	mBot = new User(-1);
	mBot->MakeUserToBot();
	mUsers.insert(std::make_pair(-1, *mBot));
}

/**************************************************** */
// Run() Utils

/**
 * @brief
 * Server클래스에 mChannels에 new Channel클래스를 매개변수 channelName, fd를 토대로 insert하는 함수
 * @param channelName
 * @param fd
 */
void Server::AppendNewChannel(const std::string& channelName, int fd)
{
	mChannels.insert(std::make_pair(channelName, new Channel(channelName, fd)));
}

/**
 * @brief
 * 매개변수 fd에서의 메세지를 수신하여 mMessage[fd]에 저장하는 함수
 * @param fd
 * @return int
 */
int Server::RecvMessage(int fd)
{
	char	buf[2];
	ssize_t	read_len;
	int		ret = 0;
	while ((read_len = recv(fd, buf, 1, 0)) == 10)
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
 *	mMessage[fd]의 메세지 끝을 체크하기 위한 함수
 * @param fd
 * @return true
 * @return false
 */
bool Server::CheckMessageEnds(int fd)
{
	if (mMessage[fd].length() < 2 &&
		mMessage[fd][0] == '\n')
		mMessage[fd] = "";
	if (mMessage[fd][mMessage[fd].length() - 1] == '\n'&&
		mMessage[fd][mMessage[fd].length() - 2] == '\r')
		return (true);
	return (false);
}

void Server::DoCommand(int fd)
{
	mCommand->Excute(fd);
}

/**
 * @brief
 * 들어온 fd를 토대로한 User클래스를 생성 및 mUsers에 추가하는 함수
 * @param fd
 */
void Server::AddUser(int fd)
{
	mUsers.insert(std::make_pair(fd, User(fd)));
}

/**
 * @brief
 * 매개변수 channelName을 가지고 있는 channel를 반환하는 함수
 * 이름의 채널이 없을경우 NULL반환
 * @param channelName
 * @return Channel*
 */
Channel* Server::FindChannel(std::string channelName)
{
	std::map<std::string, Channel *>::iterator it = mChannels.find(channelName);
	if (it == mChannels.end())
		return (NULL);
	return (it->second);
}

/**
 * @brief
 * 매개변수 userName을 mUsers에서 찾은뒤 iterator를 반환하는 함수
 * @param userName
 * @return std::map<int, User>::iterator
 */
std::map<int, User>::iterator Server::FindUser(std::string userName)
{
	std::map<int, User>::iterator it = mUsers.begin();
	for(; it != mUsers.end(); ++it)
	{
		if (it->second.GetNickName() == userName)
			return (it);
	}
	return (it);
}

/**
 * @brief
 * 매개변수 channelName인 채널을 mChannels에서 제거하는 함수
 * @param channelName
 */
void Server::RemoveChannel(std::string channelName)
{
	mChannels.erase(channelName);
}

/* Getter */
/**
 * @brief
 * Server클래스의 mUsers를 반환하는 함수
 * @return std::map<int, User>&
 */
std::map<int, User>& Server::GetUsers(void)
{
	return (mUsers);
}

/**
 * @brief
 * Server클래스의 mPassword를 반환하는 함수
 * @return std::string
 */
std::string Server::GetPassWord(void)
{
	return (mPassword);
}

/**
 * @brief
 * Server클래스의 mMessage에서 mMessage[fd]를 반환 하는 함수
 * @param fd
 * @return std::string
 */
std::string Server::GetMessage(int fd)
{
	return (mMessage[fd]);
}

std::map<std::string, Channel *>& Server::GetChannelList(void)
{
	return (mChannels);
}

