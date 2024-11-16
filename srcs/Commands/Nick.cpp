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

//  첫 닉네임을 설정하거나 닉네임을 바꾸는 함수
//  pass 명령어로 암호를 전달하지 않았으면 유저가 삭제되고 리턴됨
//  해당 닉네임이 유효한지, 중복되지 않는지 검사하고
//  검사를 통과하면 닉네임이 바뀌었다는 메세지를 유저가 접속 해있는 채널들 모두에게 전달함
//  TODO: if (commandVec[1] == "_") 이거 뭔지 모르겠음 아래 while(1) 이거도
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
	if (!CheckNickNameValid(commandVec[1]))
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

// 바꾸려고 하는 닉네임의 길이와 첫글자에 숫자가 들어가는지, 
// 허용하지 않는 문자가 들어가지 않았는지 체크해주는 함수
bool Command::CheckNickNameValid(std::string nickName)
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

// 닉네임 중복여부를 체크해주는 함수
// 대소문자를 구분하지 않고 체크함 (JANHAN == janHAN == janhan)
bool Command::CheckNickNameDuplicate(std::string nickName, UserMap& userList)
{
	UserMap::iterator it = userList.begin();
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
