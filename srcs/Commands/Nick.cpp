#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include "../Define.hpp"
#include "../Utils/IsSpecial.hpp"
#include <iostream>

/*
	- RESPONSE LIST -
	ERR_NONICKNAMEGIVEN (431) o
	ERR_ERRONEUSNICKNAME (432) o
	ERR_NICKNAMEINUSE (433) o
	ERR_NICKCOLLISION (436) (다른 서버에 대한 처리라서 안함)
 */
void Command::Nick(int fd, std::vector<std::string> commandVec)
{
	/* NICK <nickname> */
	// std::map<int, class User*>& userList = mServer.GetUserList();
	// std::map<int, class User*>::iterator it = userList.find(fd);

	class User* user = mServer.FindUser(fd);
	if (!user->GetPassRegist()) // pass-authentication == false
	{
		mResponse.ErrorNotRegistered451(*user);
		send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
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
			class User* userIt = mServer.FindUser(commandVec[1]);
			if (userIt != NULL)
				commandVec[1] = userIt->GetNickName() + "_";
			else
				break;
		}
	}
	/* TESTOUTPUT */
	std::cout << "NICK TEST OUT PUT [" << commandVec[1] << "]" << std::endl;
	std::cout << commandVec[1].empty() << std::endl;
	/* END */
	if (commandVec[1].empty())
	{
		mResponse.ErrorNoNickNameGiven431(*user);
		return ;
	}
	if (!CheckNickNameValidate(commandVec[1]))
	{
		mResponse.ErrorErronusNickName432(*user, commandVec[1]);
		user->AppendUserSendBuf("/NICK <nickname> First Letter is not digit and length is under 10.\r\n");
		return ;
	}
	if (!CheckNickNameDuplicate(commandVec[1], mServer.GetUserList()))
	{
		mResponse.ErrorNickNameInuse433(*user, commandVec[1]);
		return ;
	}
	std::string oldNickName = user->GetNickName();
	if (oldNickName == "User")
		oldNickName = user->GetNickName();
	std::vector<std::string> channelList = user->GetChannelList();
	std::vector<std::string>::iterator channelIt = channelList.begin();
	for (; channelIt != channelList.end(); channelIt++)
	{
		Channel* channel = mServer.FindChannel(*channelIt);
		if (channel)
			NickMsgToAllChannel(fd, channel->GetChannelName(),  oldNickName, commandVec[1]);
	}
	user->SetNickName(commandVec[1]); // TEST NICKNAME COLOR
	user->AppendUserSendBuf(":" + oldNickName + " NICK " + user->GetNickName() + "\r\n");
	user->SetNickRegist(true);
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
