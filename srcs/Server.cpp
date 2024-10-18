/**
 * @file Server.cpp
 * @author jang hun han (janhan@student.42Seoul.kr)
 * @brief
 * @version 0.1
 * @date 2024-10-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "Server.hpp"
#include <unistd.h>

/* Constructor */
Server::Server(const int port, const std::string& password)
: mPort(port)
, mPassword(password)
, mfdCount(1)
, mUserAddrSize(sizeof(mUserAddr))
{
	this->mCommand = new Command(*this);
}

/* Destructor */
Server::~Server()
{
	close(mServerSock);
	std::map<int, User>::iterator UserIt = mUserList.begin();
	for (; UserIt != mUserList.end(); UserIt++)
	{
		close(UserIt->first);
	}
	std::map<std::string, CHannel *>::iterator ChannelIt = mChannelList.begin();
	for (; ChannelIt != mChannelList.end(); ChannelIt++)
	{
		delete ChannelIt->second;
	}
	mUserList.clear();
	mChannelList.clear();
	delete mCommand;
	delete mBot;
}
