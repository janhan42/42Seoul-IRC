#include "../Command.hpp"
#include "../Server.hpp"
#include "../Utils/Split.hpp"
#include "../User.hpp"
#include "../Bot.hpp"
#include <iostream>

void Command::Join(int fd, std::vector<std::string> commandVec)
{
	/* JOIN <channel> (<options>) */
	/* TESTOUTPUT */
	std::cout << "JOIN TEST OUT PUT1 " << std::endl;
	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second);
		return;
	}
	std::vector<std::string> joinChannel = split(commandVec[1], ',');
	std::vector<std::string>::iterator iter = joinChannel.begin();
	std::vector<std::string> joinKey;
	std::vector<std::string>::iterator keyIt;
	/* TESTOUTPUT */
	std::cout << "JOIN TEST OUT PUT2 " << std::endl;
	if (commandVec.size() > 2)
	{
		joinKey = split(commandVec[2], ',');
		keyIt = joinKey.begin();
	}
	std::map<int, class User*>& userList = mServer.GetUserList();
	class User*& user = userList.find(fd)->second;
	/* TESTOUTPUT */
	std::cout << "JOIN TEST OUT PUT3 " << std::endl;
	while(iter != joinChannel.end())
	{
		if ((*iter)[0] != '#' && (*iter)[0] != '&')
		{
			mErrManager.ErrorNosuchChannel403(*user, *iter);
			iter++;
			if (commandVec.size() > 2 || keyIt != joinKey.end())
				keyIt++;
			continue;
		}
		/* TESTOUTPUT */
		std::cout << "JOIN TEST OUT PUT4 " << std::endl;
		std::map<std::string, Channel*>& channelList = mServer.GetChannelList();
		std::map<std::string, Channel*>::iterator channelIt = channelList.find(*iter);
		if (channelIt != channelList.end()) // channel exists
		{
			/* conditions: if users can join to channel */
			Channel* channel = channelIt->second;
			if (channel->CheckUserInChannel(fd)) // aready in channel
			{
				iter++;
				if (commandVec.size() > 2 || keyIt != joinKey.end())
					keyIt++;
				continue;
			}
			if (channel->CheckMode(INVITE)) // MODE(invite) == true;
			{
				if (!channel->CheckInvite(fd))
				{
					mErrManager.ErrorInviteOnlychan473(*user, *iter);
					iter++;
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}
			if (channel->CheckMode(KEY)) // MODE(key) == true
			{
				if (commandVec.size() <= 2 || keyIt == joinKey.end() || !channel->CheckKey(*keyIt)) // if Invalid key
				{
					mErrManager.ErrorBadChannelKey475(*user, *iter);
					iter++;
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}
			if (channel->CheckMode(LIMIT)) // MODE(limit) == true
			{
				if (channel->GetUserFdList().size() >= channel->GetLimit())
				{
					mErrManager.ErrorChannelIsFull471(*user, *iter);
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}
			std::string channelName = (*channelIt).second->GetChannelName();
			(*channelIt).second->AppendUserFdList(fd);
			user->AppendChannelList(channelName); 			// join user to channel
			MsgToAllChannel(fd, channelName, "JOIN", "");	// send join-message to users(in channel)
			TopicMsg(fd, channelName);
		}
		else // channel not exicsts (new channel)
		{
			/* TESTOUTPUT */
			std::cout << "JOIN TEST OUT PUT5 " << std::endl;
			std::cout << "Join New Channel : " << *iter << std::endl;
			mServer.AppendNewChannel(*iter, fd);			// create new channel
			mServer.FindChannel(*iter)->AppendInviteFdList(-1);
			/* TESTOUTPUT */
			std::cout << "JOIN TEST OUT PUT6 " << std::endl;
			mServer.FindChannel(*iter)->AppendUserFdList(fd); // join user to channel
			user->AppendChannelList(*iter);
			MsgToAllChannel(fd, *iter, "JOIN", "");
			mServer.FindChannel(*iter)->AddOperatorFd(fd);
			std::vector<int> newChannelUserList = mServer.FindChannel(*iter)->GetUserFdList();
			std::vector<int>::iterator it = newChannelUserList.begin();
			for(; it != newChannelUserList.end(); it++)
			{
				std::cout << "User Fd : " << *it << std::endl;
			}
		}
		NameListMsg(fd, *iter);
		MsgToAllChannel(-1, *iter, "PRIVMSG", mServer.FindChannel(*iter)->GetBot()->Introduce());
		iter++;
		if (commandVec.size() > 2 || keyIt != joinKey.end())
			keyIt++;
	}
}

void Command::TopicMsg(int fd, std::string channelName)
{
	std::map<std::string, Channel* >& channelList = mServer.GetChannelList();
	Channel* channel = channelList.find(channelName)->second;
	std::string topic = channel->GetTopic();

	if (topic.length() == 0)
		return;

	topic = topic.substr(1, topic.length() - 1);
	class User*& user = mServer.GetUserList().find(fd)->second;
	user->AppendUserRecvBuf("332 " + user->GetNickName() + " " + channelName + " :" + topic + "\r\n");
}
