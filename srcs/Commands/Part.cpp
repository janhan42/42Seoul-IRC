#include <sstream>
#include <iostream>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

void Command::Part(int fd, std::vector<std::string> commandVec)
{
	/* PART <channel> <nickname> (<reasons, ...>) */
	std::map<int, class User*> userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);
	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*userIt->second);
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
		std::cout << "상현!!!: " << *vecIt << std::endl;
		std::vector<std::string>::iterator channelIt = userIt->second->FindChannel(*vecIt);
		/* TESTOUTPUT */
		std::vector<std::string>::iterator it = userIt->second->GetChannelList().begin();
		for (; it != userIt->second->GetChannelList().end(); it++)
		{
			std::cout << "user channel list : " << *it << std::endl;
		}
		/* */

		if (channelIt != userIt->second->GetChannelList().end())
		{
			Channel* channel = mServer.FindChannel(*channelIt);
			MsgToAllChannel(fd, *vecIt, "PART", ChannelMessage(2, commandVec));
			channel->RemoveUserFdList(fd);
			channel->RemoveOperatorFd(fd);
			userIt->second->RemoveChannel(*channelIt);
			if (channel->GetUserFdList().size() == 1) // if last-user in channel: remove channel
			{
				mServer.RemoveChannel(channel->GetChannelName());
				delete channel;
			}
		}
		else	// Error
		{
			if (mServer.FindChannel(*vecIt))
			{
				mErrManager.ErrorNotOnChannel442(*userIt->second, *vecIt);
			}
			else
			{
				mErrManager.ErrorNosuchChannel403(*userIt->second, *vecIt);
			}
		}
	}
}
