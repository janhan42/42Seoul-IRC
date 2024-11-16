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
// 채널을 나가는 함수
// 유저가 인자로 들어온 채널에 있는지를 체크하고
// 해당 채널에서 유저 삭제, 만약 남은사람 없으면 채널도 삭제
// 남아있는 사람이 있으면 유저가 나갔다는 메세지를 나머지 사람들에게 보냄
void Command::Part(int fd, std::vector<std::string> commandVec)
{
	/* PART <channel> <nickname> (<reasons, ...>) */
	// std::map<int, class User*> userList = mServer.GetUserList();
	// std::map<int, class User*>::iterator userIt = userList.find(fd);

	class User* user = mServer.FindUser(fd);
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
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
		// std::vector<std::string>::iterator channelIt = user->FindChannel(*vecIt);
		// if (channelIt != user->GetChannelList().end())
		if (user->IsInChannel(*vecIt) == true)
		{
			Channel* channel = mServer.FindChannel(*vecIt);
			channel->RemoveUserFdList(fd);
			channel->RemoveOperatorFd(fd);
			user->RemoveChannel(*vecIt);
			// TODO: fd -1인 유저클래스 봇 삭제할거면 여기 수정해야함
			if (channel->GetUserFdList().size() <= 1) // if last-user in channel: remove channel
			{
				std::cout << "채널에 fd 갯수 : " << channel->GetUserFdList().size() << std::endl;
				mServer.RemoveChannel(channel->GetChannelName());
				delete channel;
			}
			else // 아무도 없으면 굳이 보낼 필요 없을 것 같아서 else에 넣음
				MsgToAllChannel(fd, *vecIt, "PART", ChannelMessage(2, commandVec));
		}
		else	// Error
		{
			if (mServer.FindChannel(*vecIt))
			{
				mResponse.ErrorNotOnChannel442(*user, *vecIt);
			}
			else
			{
				mResponse.ErrorNosuchChannel403(*user, *vecIt);
			}
		}
	}
}
