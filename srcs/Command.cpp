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
	std::istringstream	iss(mServer.GetMessage(fd));
	std::string			buffer;
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator iter = userList.find(fd);
	std::vector<std::string> commandVec;
	while (getline(iss, buffer, ' '))
	{
		std::size_t endPos = buffer.find_last_not_of("\r\n");
		commandVec.push_back(buffer.substr(0, endPos + 1));
	}
	if (iter != userList.end() && !iter->second->GetIsRegist())
	{
		// std::cout << "들어오나?: " << commandVec[0] << std::endl;
		// std::vector<std::string>::iterator it = commandVec.begin();
		// for (; it != commandVec.end(); it++)
		// 	std::cout << "commandVector:" << *it << std::endl;
		if (commandVec[0] == "PASS")
		{
			Pass(fd, commandVec);
		}
		else if (commandVec[0] == "NICK")
		{
			Nick(fd, commandVec);
		}
		else if (commandVec[0] == "USER")
		{
			User(fd, commandVec);
		}
		else if (commandVec[0] == "CAP")
		{
			std::vector<std::string>::iterator it = commandVec.begin();
			for (; it != commandVec.end(); it++)
				std::cout << "commandVector:" << *it << std::endl;
			// iter->second->AppendUserRecvBuf(":IRC CAP * LS:");
			iter->second->AppendUserRecvBuf(":IRC 001 " + iter->second->GetNickName() + " : Welcome to the Smoking Relay Chat " + iter->second->GetNickName() + "!" + iter->second->GetUserName() + "@" + iter->second->GetHostName() + "\r\n");
			iter->second->SetCapEnd(true);
			iter->second->SetRegist(true);
		}
		else
		{
			std::cout << "여기서 나가나 " << std::endl;
			std::cout << iter->second->GetPassRegist() << iter->second->GetNickRegist() << iter->second->GetUserRegist() << std::endl;
			iter->second->AppendUserRecvBuf(iter->second->GetNickName() + ":");
			iter->second->AppendUserRecvBuf(ERR_NOTREGISTERED);
			send(fd, iter->second->GetUserRecvBuf().c_str(), iter->second->GetUserRecvBuf().length(), 0);
			iter->second->ClearUser();
			userList.erase(fd);
			close(fd);
		}
		iter = userList.find(fd);
		if (iter != userList.end())
		{
			if (iter->second->GetIsRegist())
				iter->second->AppendUserRecvBuf(":IRC 001 " + iter->second->GetNickName() + " : Welcome to the Smoking Relay Chat " + iter->second->GetNickName() + "!" + iter->second->GetUserName() + "@" + iter->second->GetHostName() + "\r\n");
		}
	}
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
		user->AppendUserRecvBuf(MakeFullName(target) + " " + command + " " + channelName + " " + msg + "\r\n");
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
			message += "@";
		message += user->GetNickName();
		if (iter != userFdList.end() - 1)
			message += " ";
		iter++;
	}
	class User*& user = mServer.GetUserList().find(fd)->second;
	user->AppendUserRecvBuf("353 " + user->GetNickName() + " = " + channelName + " :" + message + "\r\n");
	user->AppendUserRecvBuf("366 " + user->GetNickName() + " " + channelName + " :End of NAMES list.\r\n");
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
			target->AppendUserRecvBuf(":" + user.GetNickName() + " PRIVMSG " + chennal->GetChannelName() + " :" + message + "\r\n");
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
			target->AppendUserRecvBuf(":" + target->GetNickName() + "!" + target->GetUserName() + "@" + target->GetServerName() + userIt->second->GetNickName() + " PART " + channel->GetChannelName() + " " + message + "\r\n");
		}
	}
}