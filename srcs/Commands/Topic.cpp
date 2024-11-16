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
// 토픽 설정하는 함수
// 토픽 하고 채널이름이 왔는지 체크,
// 채널이 존재하는 채널인지 체크,
// topic명령어 쓴 유저가 해당 채널에 있는지 체크함
//
// 채널 이름 뒤에 토픽 메세지가 온 경우 mode체크하고 변경 가능하면 변경
// 토픽 메세지 인자가 없는 경우 토픽을 띄워주는데, 
// 기존에 설정된 토픽이 없으면 에러를 보낸다
//
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

	// 해당 채널 내부에 내가 존재하는지 체크하는 부분
	// 기존에 있는 함수로 줄일 수 있음
	//
	// std::vector<int> channelUserFdList = channel->GetUserFdList();
	// std::vector<int>::iterator iter = channelUserFdList.begin();
	// for (; iter != channelUserFdList.end(); iter++) // find User in Channel
	// {
	// 	if (*iter == fd)
	// 		break;
	// }
	// if (iter == channelUserFdList.end()) // User Not on Channel
	// {
	// 	mResponse.ErrorNotOnChannel442(user, commandVec[1]);
	// 	return;
	// }
	if (user.IsInChannel(commandVec[1]) == false)
	{
		mResponse.ErrorNotOnChannel442(user, commandVec[1]);
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
