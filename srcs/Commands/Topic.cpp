#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

void Command::Topic(int fd, std::vector<std::string> commandVec)
{
	/* TOPIC <channel> <topic-message> */
	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second, commandVec[1]);
		return;
	}
	Channel* channel = mServer.FindChannel(commandVec[1]);
	if (channel == NULL)
	{
		mErrManager.ErrorNosuchChannel403(*mServer.GetUserList().find(fd)->second, commandVec[1]);
		return;
	}
	std::vector<int> channelUserFdList = channel->GetUserFdList();
	std::vector<int>::iterator iter = channelUserFdList.begin();
	for (; iter != channelUserFdList.end(); iter++)
	{
		if (*iter == fd)
			break;
	}
	if (iter == channelUserFdList.end())
	{
		mErrManager.ErrorNotOnChannel442(*mServer.GetUserList().find(fd)->second, commandVec[1]);
		return;
	}
	if (channel->CheckMode(TOPIC))
	{
		if (channel->CheckOperator(fd) == false)
		{
			mErrManager.ErrorChanOprivsNeeded482(*mServer.GetUserList().find(fd)->second, commandVec[1]);
			return;
		}
	}
	if (commandVec.size() == 2)
	{
		if (channel == NULL)
		{
			mErrManager.ErrorNosuchChannel403(*mServer.GetUserList().find(fd)->second, commandVec[1]);
			return;
		}
		if (channel->GetTopic().length() == 0)
		{
			mServer.GetUserList().find(fd)->second->AppendUserSendBuf("331 " + mServer.GetUserList().find(fd)->second->GetNickName() + "  " + commandVec[1] + " :" + RPL_NOTOPIC);
			return;
		}
		mServer.GetUserList().find(fd)->second->AppendUserSendBuf("332 " + mServer.GetUserList().find(fd)->second->GetNickName() + " " + commandVec[1] + " :" + channel->GetTopic() + "\r\n");
	}
	else
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
