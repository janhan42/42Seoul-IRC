#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461) o
	ERR_NOSUCHCHANNEL (403) o
	ERR_NOTONCHANNEL (442) o
	ERR_CHANOPRIVSNEEDED (482) o
	RPL_NOTOPIC (331) o
	RPL_TOPIC (332) o
	RPL_TOPICWHOTIME (333) irssi 쪽에서 관리함
 */
void Command::Topic(int fd, std::vector<std::string> commandVec)
{
	/* TOPIC <channel> <topic-message> */
	//class User& user = *mServer.GetUserList().find(fd)->second;
	class User& user = *mServer.FindUser(fd);
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(user, commandVec[1]);
		return;
	}
	Channel* channel = mServer.FindChannel(commandVec[1]);
	if (channel == NULL)
	{
		mResponse.ErrorNosuchChannel403(user, commandVec[1]);
		return;
	}
	std::vector<int> channelUserFdList = channel->GetUserFdList();
	std::vector<int>::iterator iter = channelUserFdList.begin();
	for (; iter != channelUserFdList.end(); iter++) // find User in Channel
	{
		if (*iter == fd)
			break;
	}
	if (iter == channelUserFdList.end()) // User Not on Channel
	{
		mResponse.ErrorNotOnChannel442(user, commandVec[1]);
		return;
	}
	if (channel->CheckMode(TOPIC)) // Channel Mode +t
	{
		if (channel->CheckOperator(fd) == false)
		{
			mResponse.ErrorChanOprivsNeeded482(user, commandVec[1]);
			return;
		}
	}
	if (commandVec.size() == 2)
	{
		if (channel->GetTopic().length() == 0)
		{
			mResponse.ErrorNoTopic331(user, commandVec[1]);
			return;
		}
		user.AppendUserSendBuf("332 " + user.GetNickName() + " " + commandVec[1] + " :" + channel->GetTopic() + "\r\n");
	}
	else // SetTopic
	{
		if (commandVec[2] == ":")
			channel->SetTopic("");
		else
		{
			std::string topic = commandVec[2];
			for (size_t i = 3; i < commandVec.size(); i++)
			{
				topic += " " + commandVec[i];
			}
			channel->SetTopic(topic);
		}
		MsgToAllChannel(fd, commandVec[1], "TOPIC", channel->GetTopic());
	}
}
