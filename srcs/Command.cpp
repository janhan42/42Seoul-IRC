#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include "Channel.hpp"
#include "Define.hpp"
#include "Server.hpp"
#include "Command.hpp"
#include "User.hpp"

Command::Command(Server& Server)
: mServer(Server)
{
}

Command::~Command()
{
}

void Command::Run(int fd)
{
	std::istringstream iss(mServer.GetMessage(fd));
	// std::map<int, class User *>& userList = mServer.GetUserList();
	// std::map<int, class User *>::iterator userIt = userList.find(fd);
	class User* user = mServer.FindUser(fd);
	std::vector<std::string> commandVec;
	std::string buffer;
	while (getline(iss, buffer, ' '))
	{
		std::size_t endPos = buffer.find_last_not_of("\r\n");
		commandVec.push_back(buffer.substr(0, endPos + 1));
	}
	if (user!= NULL && !user->GetIsRegist()) // new User Join Server
		RegistNewUser(fd, user, commandVec);
	else
	{
		if (commandVec[0] == "PING")
			Ping(fd, commandVec);
		else if (commandVec[0] == "USER")
			User(fd, commandVec);
		else if (commandVec[0] == "NICK")
			Nick(fd, commandVec);
		else if (commandVec[0] == "PASS")
			Pass(fd, commandVec);
		else if (commandVec[0] == "PRIVMSG")
			Privmsg(fd, commandVec);
		else if (commandVec[0] == "QUIT")
			Quit(fd, commandVec);
		else if (commandVec[0] == "PART")
			Part(fd, commandVec);
		else if (commandVec[0] == "JOIN")
			Join(fd, commandVec);
		else if (commandVec[0] == "KICK")
			Kick(fd, commandVec);
		else if (commandVec[0] == "MODE")
			Mode(fd, commandVec);
		else if (commandVec[0] == "TOPIC")
			Topic(fd, commandVec);
		else if (commandVec[0] == "INVITE")
			Invite(fd, commandVec);
	}
}

void Command::RegistNewUser(int fd, class User* user, std::vector<std::string>& commandVec)
{
	if (commandVec[0] == "PASS")
		Pass(fd, commandVec);
	else if (commandVec[0] == "NICK")
		Nick(fd, commandVec);
	else if (commandVec[0] == "USER")
	{
		User(fd, commandVec);
		if (user->GetIsRegist() == false)
		{
			// user->AppendUserSendBuf(commandVec[1] + ": " + ERR_NOTREGISTERED);
			mResponse.ErrorNotRegistered451(*user);
			send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
			// delete user->second;
			// userList.erase(fd);
			//close(fd);
			mServer.DeleteUserFromServer(fd);
			return;
		}
	}
	if (user->GetIsRegist())
	{
		mResponse.RPLWelcome001(*user, MakeFullName(fd));
		mResponse.RPLYourHost002(*user);
		mResponse.RPLCreated003(*user);
		mResponse.RPLMyInfo004(*user);
		mResponse.RPLISupport005(*user);
		mResponse.RPLNoMotd422(*user);
	}
}

/**
 * @brief
 * 이름이 channelName인 채널에 있는 유저 전부에게 매개변수 msg를 전송
 * @param target
 * @param channelName
 * @param command
 * @param msg
 */
void Command::MsgToAllChannel(int fd, std::string channelName, std::string command, std::string msg)
{
	// std::map<std::string, Channel *>& channelList = mServer.GetChannelList();
	// if (channelList.find(channelName) == channelList.end()) // not exist channel
	// 	return;
	// Channel* channel = channelList.find(channelName)->second;
	Channel* channel = mServer.FindChannel(channelName);
	if (channel == NULL) // does not exist
		return;
	std::vector<int> channelUserList = channel->GetUserFdList();
	std::vector<int>::iterator channelUserIt = channelUserList.begin();
	while (channelUserIt != channelUserList.end())
	{
		//class User*& targetUser = mServer.GetUserList().find(*channelUserIt)->second; // 채널에 있는 모든 유저를 순회하며 조회
		class User* targetUser = mServer.FindUser(fd); // 채널에 있는 모든 유저를 순회하며 조회
		if (command == "PRIVMSG" && fd == *channelUserIt) // 현재 channelUserIt이 자기 자신이면
		{
			channelUserIt++;
			continue;
		}
		targetUser->AppendUserSendBuf(MakeFullName(fd) + " " + command + " " + channelName + " " + msg + "\r\n");
		channelUserIt++;
	}
}

/**
 * @brief
 * 채널에 있는 유저가 닉네임을 변경했을 경우 채널 내부에 있는 모든 유저에게 알림
 * @param fd
 * @param channelName
 * @param oldNickName
 * @param NewNick
 */
void Command::NickMsgToAllChannel(int fd, std::string channelName, std::string oldNickName, std::string NewNick)
{
	// std::map<std::string, Channel *>& channelList = mServer.GetChannelList();
	// if (channelList.find(channelName) == channelList.end())
	// 	return;
	// Channel* channel = channelList.find(channelName)->second;
	Channel* channel = mServer.FindChannel(channelName);
	if (channel == NULL) // does not exist
		return;
	std::vector<int> channelUserList = channel->GetUserFdList();
	std::vector<int>::iterator channelUserIt = channelUserList.begin();
	while (channelUserIt != channelUserList.end())
	{
		//class User*& targetUser = mServer.GetUserList().find(*channelUserIt)->second;
		class User* targetUser = mServer.FindUser(fd);
		if (fd == *channelUserIt) //  현재 channelUserIt이 자기 자신이면
		{
			channelUserIt++;
			continue;
		}
		targetUser->AppendUserSendBuf(":" + oldNickName + " NICK " + NewNick + "\r\n");
		channelUserIt++;
	}
}

std::string Command::MakeFullName(int fd)
{
	// std::map<int, class User* >& userList = mServer.GetUserList();
	// std::map<int, class User* >::iterator userIt = userList.find(fd);
	// class User*& user = userIt->second;
	class User* user = mServer.FindUser(fd);
	std::string temp = (":" + user->GetNickName() + "!" + user->GetUserName() + "@" + user->GetServerName());
	return (temp);
}

void Command::NameListMsg(int fd, std::string channelName)
{
	// std::map<std::string, Channel* >& channelList = mServer.GetChannelList();
	// if (channelList.find(channelName) == channelList.end())
	// 	return;
	// Channel* channel = channelList.find(channelName)->second;
	Channel* channel = mServer.FindChannel(channelName);
	if (channel == NULL) // does not exist
		return;
	std::vector<int> userFdList = channel->GetUserFdList();
	std::vector<int>::iterator iter = userFdList.begin();
	std::string message;
	while (iter != userFdList.end())
	{
		//class User*& user = mServer.GetUserList().find(*iter)->second;
		class User* user = mServer.FindUser(*iter);
		if (channel->CheckOperator(user->GetUserFd()))
		{
			message += "@";
		}
		message += user->GetNickName();
		if (iter != userFdList.end() - 1)
			message += " ";
		iter++;
	}
	//class User*& user = mServer.GetUserList().find(fd)->second;
	class User* user = mServer.FindUser(fd);
	/* e
		O ("=", 0x3D)- 공개 채널.
		X ("@", 0x40)- 비밀 채널( 비밀 채널 모드 "+s" ).
		X ("*", 0x2A)- 개인 채널( "+p"이전에는 널리 사용되지 않았음).
		공개 채널만 지원
	 */
	mResponse.RPL_NamReply353(*user, channelName, message);
	mResponse.RPL_EndOfNames366(*user, channelName);
}

std::string Command::ChannelMessage(int index, std::vector<std::string> commandVec)
{
	std::string message = "";
	for (size_t i = index; i < commandVec.size(); i++)
	{
		message += commandVec[i];
		if (i != commandVec.size() - 1)
			message += " ";
	}
	if (message[0] == ':')
		message = message.substr(1, message.length() - 1);
	if (message.length() == 0)
		message = "NO REASON";
	return (message);
}

void Command::ChannelPrivmsg(std::string message, class User& user, Channel* chennal)
{
	std::vector<int> fdList = chennal->GetUserFdList();
	std::vector<int>::iterator fdIter = fdList.begin();
	for(; fdIter != fdList.end(); fdIter++)
	{
		if (*fdIter != user.GetUserFd())
		{
			//class User*& target = mServer.GetUserList().find(*fdIter)->second;
			class User* target = mServer.FindUser(*fdIter);
			target->AppendUserSendBuf(":" + user.GetNickName() + " PRIVMSG " + chennal->GetChannelName() + " :" + message + "\r\n");
		}
	}
}

// void Command::ChannelPART(int fd, std::string channelName, std::vector<std::string> commandVec)
// {
// 	// std::map<int, class User* >& userList = mServer.GetUserList();
// 	// std::map<int, class User* >::iterator userIt = userList.find(fd);
// 	class User* user = mServer.FindUser(fd);
// 	Channel* channel = mServer.FindChannel(channelName);
// 	std::vector<int> fdList = channel->GetUserFdList();
// 	std::vector<int>::iterator fdIter = fdList.begin();
// 	std::string message = ChannelMessage(1, commandVec);
// 	for(; fdIter != fdList.end(); fdIter++)
// 	{
// 		if (*fdIter != fd)
// 		{
// 			//class User*& target = mServer.GetUserList().find(*fdIter)->second;
// 			class User* target = mServer.FindUser(*fdIter);
// 			target->AppendUserSendBuf(":" + target->GetNickName() + "!" + target->GetUserName() + "@" + target->GetServerName() + user->GetNickName() + " PART " + channel->GetChannelName() + " " + message + "\r\n");
// 		}
// 	}
// }

