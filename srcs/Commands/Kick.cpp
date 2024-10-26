#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include <sstream>

void Command::Kick(int fd, std::vector<std::string> commandVec)
{
	/* KICK <channel> <nickname> */
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);
	if (commandVec.size() < 3)
	{
		mErrManager.ErrorNeedMoreParams461(* userIt->second);
		return;
	}
	std::istringstream iss(commandVec[1]);
	std::string buffer;
	std::vector<std::string> vec;
	while(getline(iss, buffer, ','))
		vec.push_back(buffer);
	std::vector<std::string>::iterator vecIt = vec.begin();
	Channel* channel = mServer.FindChannel(*vecIt);
	if (channel && !channel->CheckOperator(fd))
	{
		mErrManager.ErrorChanOprivsNeeded482(*userIt->second, *vecIt);
		return;
	}
	for (; vecIt != vec.end(); vecIt++)
	{
		Channel* channel = mServer.FindChannel(*vecIt);
		if (channel == NULL) // channel not exists
		{
			mErrManager.ErrorNosuchChannel403(*userIt->second, *vecIt);
		}
		else
		{
			std::map<int, class User*>::iterator target = mServer.FindUser(commandVec[2]);
			if (target == mServer.GetUserList().end()) // user not exists
			{
				mErrManager.ErrorNosuchNick401(*userIt->second, commandVec[2]);
				return;
			}
			if (target->second->GetUserFd() == -1) // if UserFd == -1 -> Bot: Error(ignore)
			{
				mErrManager.ErrorNosuchNick401(*userIt->second, commandVec[2]);
				return;
			}
			if (target->second->GetNickName() == userIt->second->GetNickName()) // if user == me: ignore
			{
				return;
			}
			else
			{
				if (!channel->CheckUserInChannel(target->second->GetUserFd())) // user not exists "in channel"
				{
					mErrManager.ErrorUserNotInChannel441(*userIt->second, commandVec[2], *vecIt);
				}
				else // kick user from channel
				{
					std::string message = commandVec[2];
					if (commandVec.size() > 3)
						message += " " + commandVec[3];
					MsgToAllChannel(fd, *vecIt, "KICK", message);
					channel->RemoveUserFdList(target->second->GetUserFd());
					target->second->RemoveChannel(*vecIt);
				}
			}
		}
	}
}
