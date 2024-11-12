#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include "ErrDefine.hpp"
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
	std::string buffer;
	std::map<int, class User *> &userList = mServer.GetUserList();
	std::map<int, class User *>::iterator iter = userList.find(fd);
	// user = userList[fd]
	std::vector<std::string> commandVec;
	while (getline(iss, buffer, ' '))
	{
		std::size_t endPos = buffer.find_last_not_of("\r\n");
		commandVec.push_back(buffer.substr(0, endPos + 1));
	}
	if (iter != userList.end() && !iter->second->GetIsRegist()) // 유저 첫 등록시
		RegistNewUser(fd, userList, iter, commandVec);
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
		else if (commandVec[0] == "NOTICE")
			Notice(fd, commandVec);
	}
}

void Command::RegistNewUser(int &fd, std::map<int, class User *> &userList, std::map<int, class User *>::iterator &iter, std::vector<std::string> &commandVec)
{
	if (commandVec[0] == "PASS")
		Pass(fd, commandVec);
	else if (commandVec[0] == "NICK")
		Nick(fd, commandVec);
	else if (commandVec[0] == "USER")
	{
		User(fd, commandVec);
		if (!iter->second->GetIsRegist())
		{
			iter->second->AppendUserSendBuf(commandVec[1] + ": ");
			iter->second->AppendUserSendBuf(ERR_NOTREGISTERED);
			send(fd, iter->second->GetUserSendBuf().c_str(), iter->second->GetUserSendBuf().length(), 0);
			// delete iter->second;
			// userList.erase(fd);
			//close(fd);
			mServer.DeleteUserFromServer(fd);
		}
	}
	iter = userList.find(fd);
	if (iter != userList.end())
	{
		if (iter->second->GetIsRegist())
		{
			iter->second->AppendUserSendBuf(":SIRC 001 " + iter->second->GetNickName() + " :Welcome to the Smoking Relay Chat " + iter->second->GetNickName() + "!" + iter->second->GetUserName() + "@" + iter->second->GetHostName() + " \r\n");
			iter->second->AppendUserSendBuf(":SIRC 002 " + iter->second->GetNickName() + " :Your host is SIRC, running version v1.0.0 \r\n");
			iter->second->AppendUserSendBuf(":SIRC 003 " + iter->second->GetNickName() + " :This server was created 2024 \r\n");
			iter->second->AppendUserSendBuf(":SIRC 004  " + iter->second->GetNickName() + " SRIC v1.0.0 \r\n");
			iter->second->AppendUserSendBuf( ":SIRC 005 " + iter->second->GetNickName() + " MAXNICKLEN=9 MAXCHANNELLEN=200 :are supported by this server\r\n");
			iter->second->AppendUserSendBuf(":SIRC 442  " + iter->second->GetNickName() + " :MOTD File is missing\r\n");
		}
	}
}

void Command::MsgToAllChannel(int target, std::string channelName, std::string command, std::string msg)
{
	std::map<std::string, Channel* >& channelList = mServer.GetChannelList();
	if (channelList.find(channelName) == channelList.end()) // not exist channel
		return;
	Channel* channel = channelList.find(channelName)->second;
	std::vector<int> userFdList = channel->GetUserFdList();
	std::vector<int>::iterator iter = userFdList.begin();
	while (iter != userFdList.end())
	{
		class User*& user = mServer.GetUserList().find(*iter)->second;
		if (command == "PRIVMSG" && target == *iter)
		{
			iter++;
			continue;
		}
		user->AppendUserSendBuf(MakeFullName(target) + " " + command + " " + channelName + " " + msg + "\r\n");
		iter++;
	}
}

void Command::NickMsgToAllChannel(int target, std::string channelName, std::string oldNickName, std::string NewNick)
{
	std::map<std::string, Channel*>& channelList = mServer.GetChannelList();
	if (channelList.find(channelName) == channelList.end())
		return;
	Channel* channel = channelList.find(channelName)->second;
	std::vector<int> userFdList = channel->GetUserFdList();
	std::vector<int>::iterator iter = userFdList.begin();
	while (iter != userFdList.end())
	{
		class User*& user = mServer.GetUserList().find(*iter)->second;
		if (target == *iter)
		{
			iter++;
			continue;
		}
		user->AppendUserSendBuf(":" + oldNickName + " NICK " + NewNick + "\r\n");
		iter++;
	}
}

std::string Command::MakeFullName(int fd)
{
	std::map<int, class User* >& userList = mServer.GetUserList();
	std::map<int, class User* >::iterator userIt = userList.find(fd);
	class User*& user = userIt->second;
	std::string temp = (":" + user->GetNickName() + "!" + user->GetUserName() + "@" + user->GetServerName());
	return (temp);
}

void Command::NameListMsg(int fd, std::string channelName)
{
	std::map<std::string, Channel* >& channelList = mServer.GetChannelList();
	if (channelList.find(channelName) == channelList.end())
		return;
	Channel* channel = channelList.find(channelName)->second;
	std::vector<int> userFdList = channel->GetUserFdList();
	std::vector<int>::iterator iter = userFdList.begin();
	std::string message;
	while (iter != userFdList.end())
	{
		class User*& user = mServer.GetUserList().find(*iter)->second;
		if (channel->CheckOperator(user->GetUserFd()))
		{
			message += "@";
		}
		message += user->GetNickName();
		if (iter != userFdList.end() - 1)
			message += " ";
		iter++;
	}
	class User*& user = mServer.GetUserList().find(fd)->second;
	/*
		O ("=", 0x3D)- 공개 채널.
		X ("@", 0x40)- 비밀 채널( 비밀 채널 모드 "+s" ).
		X ("*", 0x2A)- 개인 채널( "+p"이전에는 널리 사용되지 않았음).
		공개 채널만 지원
	 */
	user->AppendUserSendBuf("353 " + user->GetNickName() + " = " + channelName + " :" + message + "\r\n");
	user->AppendUserSendBuf("366 " + user->GetNickName() + " " + channelName + " :End of NAMES list.\r\n");
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
			class User*& target = mServer.GetUserList().find(*fdIter)->second;
			target->AppendUserSendBuf(":" + user.GetNickName() + " PRIVMSG " + chennal->GetChannelName() + " :" + message + "\r\n");
		}
	}
}

void Command::ChannelPART(int fd, std::string channelName, std::vector<std::string> commandVec)
{
	std::map<int, class User* >& userList = mServer.GetUserList();
	std::map<int, class User* >::iterator userIt = userList.find(fd);
	Channel* channel = mServer.FindChannel(channelName);
	std::vector<int> fdList = channel->GetUserFdList();
	std::vector<int>::iterator fdIter = fdList.begin();
	std::string message = ChannelMessage(1, commandVec);
	for(; fdIter != fdList.end(); fdIter++)
	{
		if (*fdIter != fd)
		{
			class User*& target = mServer.GetUserList().find(*fdIter)->second;
			target->AppendUserSendBuf(":" + target->GetNickName() + "!" + target->GetUserName() + "@" + target->GetServerName() + userIt->second->GetNickName() + " PART " + channel->GetChannelName() + " " + message + "\r\n");
		}
	}
}
