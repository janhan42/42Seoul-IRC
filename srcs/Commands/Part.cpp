#include <sstream>
#include <iostream>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST-
	ERR_NEEDMOREPARAMS (461)
	ERR_NOSUCHCHANNEL (403)
	ERR_NOTONCHANNEL (442)
 */
void Command::Part(int fd, std::vector<std::string> commandVec)
{
	/* PART <channel> <nickname> (<reasons, ...>) */
	std::map<int, class User*> userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*userIt->second, commandVec[1]);
		return;
	}
	std::istringstream iss(commandVec[1]);
	std::string buffer;
	std::vector<std::string> vec;
	while (getline(iss, buffer, ',')) // 기존 , 였음
		vec.push_back(buffer);
	std::vector<std::string>::iterator vecIt = vec.begin();
	for (; vecIt != vec.end(); vecIt++)
	{
		std::vector<std::string>::iterator channelIt = userIt->second->FindChannel(*vecIt);
		if (channelIt != userIt->second->GetChannelList().end())
		{
			Channel* channel = mServer.FindChannel(*channelIt);
			MsgToAllChannel(fd, *vecIt, "PART", ChannelMessage(2, commandVec));
			channel->RemoveUserFdList(fd);
			channel->RemoveOperatorFd(fd);
			userIt->second->RemoveChannel(*channelIt);
			if (channel->GetUserFdList().size() <= 1) // if last-user in channel: remove channel
			{
				std::cout << "채널에 fd 갯수 : " << channel->GetUserFdList().size() << std::endl;
				mServer.RemoveChannel(channel->GetChannelName());
				delete channel;
			}
		}
		else	// Error
		{
			if (mServer.FindChannel(*vecIt))
			{
				mResponse.ErrorNotOnChannel442(*userIt->second, *vecIt);
			}
			else
			{
				mResponse.ErrorNosuchChannel403(*userIt->second, *vecIt);
			}
		}
	}
}
