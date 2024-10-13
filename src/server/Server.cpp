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
#include <cstdlib>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <unistd.h>

Server::Server(const int port, const std::string& password)
: mPort(port)
, mPassword(password)
, mFdCount(1)
, mbRunning(false)
{}

Server::~Server()
{}

bool Server::Init()
{
	if (SetSocket()
	|| SetAddr()
	|| SetListen())
	{
		std::cerr << "ERROR: Init Failed" << std::endl;
		return (EXIT_FAILURE);
	}
	mbRunning = true;	// Init 성공시 Running 가능
	return (EXIT_SUCCESS);
}

void Server::Run()
{
	int kq = kqueue();
	if (kq < 0)
	{
		std::cerr << "ERROR: kqueue creation Failed" << std::endl;
		close(mServerSock);
		return;
	}

    struct kevent evSet;
    EV_SET(&evSet, mServerSock, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, &evSet, 1, NULL, 0, NULL);

	std::cout << "Server: Waiting for clinet's connection..." << std::endl;

	while(mbRunning)
	{
	
	}
}


/**
 * @brief
 * Server클래스의 mServerSock을 세팅한다
 */
bool Server::SetSocket()
{
	int	opt = 1;
	mServerSock = socket(AF_INET, SOCK_STREAM, 0);
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

bool Server::SetListen()
{
	if (listen(mServerSock, SOMAXCONN) < 0)
	{
		std::cerr << "ERROR: listen() error" << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
