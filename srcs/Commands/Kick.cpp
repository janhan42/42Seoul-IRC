#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include <sstream>
#include <iostream>
#include <string>

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461)
	ERR_NOSUCHCHANNEL (403)
	ERR_CHANOPRIVSNEEDED (482)
	ERR_USERNOTINCHANNEL (441)
	ERR_NOTONCHANNEL (442)
 */

/*
	TODO: /kick #test janhan,sangshin,min을 했을떄
	KICK #test janhan,sangshin,min
	KICK #test janhan
	KICK #test sangshin
	KICK #test min
	이렇게 3번 입력이 들어옴 Irssi 클라이언트에서 처리를 따로 해줌 밑에 로직을 다시 만들어야할듯
*/

void Command::Kick(int fd, std::vector<std::string> commandVec)
{
	/* KICK <channel> <nickname> */
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);

	/* TEST OUTPUT */
	int i = 0;
	for (std::vector<std::string>::iterator it = commandVec.begin(); it != commandVec.end(); it++)
	{
		std::cout << "commandVec[" + std::to_string(i) + "] : " << commandVec[i] << std::endl;
		i++;
	}
	/* END */

	if (commandVec.size() < 3)
	{
		mResponse.ErrorNeedMoreParams461(*userIt->second, commandVec[1]);
		return;
	}
	if (userIt->second->AmIInChannel(commandVec[1]) == false)
	{
		mResponse.ErrorNotOnChannel442(*userIt->second, commandVec[1]);
		return;
	}
	std::istringstream iss(commandVec[1]);
	std::string buffer;
	std::vector<std::string> vec;
	while(getline(iss, buffer, ',')) // TargetUser
		vec.push_back(buffer);
	std::vector<std::string>::iterator vecIt = vec.begin();

	/* TEST OUTPUT */
	i = 0;
	for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++)
	{
		std::cout <<  "VecIt[" << std::to_string(i) << "] : " << *it << std::endl;
		i++;
	}
	/* END */

	Channel* channel = mServer.FindChannel(*vecIt);
	if (channel && channel->CheckOperator(fd) == false)
	{
		mResponse.ErrorChanOprivsNeeded482(*userIt->second, *vecIt);
		return;
	}
	for (; vecIt != vec.end(); vecIt++)
	{
		Channel* channel = mServer.FindChannel(*vecIt);
		std::cout << "Find Channel : " << *vecIt << std::endl; // TEST OUTPUT
		if (channel == NULL) // channel not exists
		{
			mResponse.ErrorNosuchChannel403(*userIt->second, *vecIt);
		}
		else
		{
			std::map<int, class User*>::iterator target = mServer.FindUser(commandVec[2]);
			if (target == mServer.GetUserList().end()) // user not exists
			{
				mResponse.ErrorUserNotInChannel441(*userIt->second, commandVec[2], *vecIt);
				return;
			}
			if (target->second->GetUserFd() == -1) // if user == bot : ignore
				return;
			else
			{
				if (!channel->CheckUserInChannel(target->second->GetUserFd())) // user not exists "in channel"
				{
					mResponse.ErrorUserNotInChannel441(*userIt->second, commandVec[2], *vecIt);
				}
				else // kick user from channel
				{
					std::string message = commandVec[2];
					if (commandVec.size() > 3)
					{
						for (size_t i = 3; i < commandVec.size(); i++)
						{
							message = message + " " + commandVec[i];
						}
					}
					MsgToAllChannel(fd, *vecIt, "KICK", message);
					channel->RemoveUserFdList(target->second->GetUserFd());
					channel->RemoveOperatorFd(target->second->GetUserFd()); // 오퍼레이터일수도 있어서 추가
					target->second->RemoveChannel(*vecIt);
					if (channel->GetUserFdList().size() <= 1)// if no user in channel
					{
						mServer.RemoveChannel(channel->GetChannelName());
						delete channel;
					}
				}
			}
		}
	}
}

