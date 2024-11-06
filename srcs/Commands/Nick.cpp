#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include "../ErrDefine.hpp"
#include "../Utils/IsSpecial.hpp"

// void ErrorNotRegistered451(User& user)
// {
// 	user.AppendUserSendBuf("451: ");
// 	user.AppendUserSendBuf(ERR_NOTREGISTERED);
// }

// void ErrorErronusNickName432(User& user)
// {
// 	user.AppendUserSendBuf("432: ");
// 	user.AppendUserSendBuf(ERR_ERRONEUSNICKNAME);
// }

// void ErrorNickNameInuse433(User& user, std::string name)
// {
// 	user.AppendUserSendBuf("433: " + name + " " + name + " ");
// 	user.AppendUserSendBuf(ERR_NICKNAMEINUSE);
// }

void Command::Nick(int fd, std::vector<std::string> commandVec)
{
	/* NICK <nickname> */
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator it = userList.find(fd);
	if (!it->second->GetPassRegist()) // pass-authentication == false
	{
		mErrManager.ErrorNotRegistered451(*it->second);
		send(fd, it->second->GetUserSendBuf().c_str(), it->second->GetUserSendBuf().length(), 0);
		// delete it->second;
		// userList.erase(fd);
		// close(fd);
		mServer.DeleteUserFromServer(fd);
		return ;
	}
	if (commandVec[1] == "_")
	{
		while (1)
		{
			std::map<int, class User*>::iterator userIt = mServer.FindUser(commandVec[1]);
			if (userIt != mServer.GetUserList().end())
				commandVec[1] = userIt->second->GetNickName() + "_";
			else
				break;
		}
	}
	if (!CheckNickNameValidate(commandVec[1]))
	{
		mErrManager.ErrorErronusNickName432(*it->second, commandVec[1]);
		it->second->AppendUserSendBuf("/NICK <nickname> First Letter is not digit and length is under 10.\r\n");
		return ;
	}
	if (!CheckNickNameDuplicate(commandVec[1], mServer.GetUserList()))
	{
		mErrManager.ErrorNickNameInuse433(*it->second, commandVec[1]);
		return ;
	}
	std::string oldNickName = it->second->GetNickName();
	if (oldNickName == "User")
		oldNickName = it->second->GetNickName();
	std::vector<std::string> channelList = it->second->GetChannelList();
	std::vector<std::string>::iterator channelIt = channelList.begin();
	for (; channelIt != channelList.end(); channelIt++)
	{
		Channel* channel = mServer.FindChannel(*channelIt);
		if (channel)
			NickMsgToAllChannel(fd, channel->GetChannelName(),  oldNickName, commandVec[1]);
	}
	it->second->SetNickName(commandVec[1]); // TEST NICKNAME COLOR
	it->second->AppendUserSendBuf(":" + oldNickName + " NICK " + it->second->GetNickName() + "\r\n");
	it->second->SetNickRegist(true);
}

bool Command::CheckNickNameValidate(std::string nickName)
{
	if (nickName.length() == 0 || nickName.length() > 9)
		return (false);
	if ('0' <= nickName[0] && nickName[0] <= '9')
		return (false);
	for (size_t i = 1; i < nickName.length(); i++)
	{
		if (!isalnum(nickName[i]) && !isSpecial(nickName[i]))
			return (false);
	}
	return (true);
}

bool Command::CheckNickNameDuplicate(std::string nickName, std::map<int, class User*>& userList)
{
	std::map<int, class User*>::iterator it = userList.begin();
	for (; it != userList.end(); it++)
	{
		std::string userNickName = it->second->GetNickName();
		bool isSame = true;
		if (userNickName.length() != nickName.length())
			continue;
		for (size_t i = 0; i < nickName.length(); ++i)
		{
			if (std::toupper(nickName[i]) != std::toupper(userNickName[i]))
			{
				isSame = false;
				break;
			}
		}
		if (isSame)
			return (false);
	}
	return (true);
}
