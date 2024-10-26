#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include "../Bot.hpp"
#include <sstream>

void Command::Privmsg(int fd, std::vector<std::string> commandVec)
{
	/* PRIVMSG(or /msg) <channel/nickname> <messages...> */
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);
	std::string buffer;
	std::vector<std::string> vec;
	if (commandVec.size() < 2)
	{
		mErrManager.ErrorNeedMoreParams461(*userIt->second);
		return;
	}

	std::istringstream iss(commandVec[1]);
	while (getline(iss, buffer, ','))
		vec.push_back(buffer);

	std::vector<std::string>::iterator vecIt = vec.begin();
	for(; vecIt != vec.end(); vecIt++)
	{
		if ((*vecIt)[0] == '#' || (*vecIt)[0] == '&') // Send To Channel
		{
			Channel* channel = mServer.FindChannel(*vecIt); // Find Channel
			if (channel) // if Channel exists
			{
				if (commandVec.size() > 2 && CheckBotCommand(commandVec[2]))
				{
					BotCommand(fd, commandVec);
					return ;
				}
				std::string messages = ChannelMessage(2, commandVec);
				ChannelPrivmsg(messages, *userIt->second, channel);
			}
			else // if Channel not exists
			{
				mErrManager.ErrorNosuchChannel403(*userIt->second, *vecIt);
			}
		}
		else
		{
			std::map<int, class User*>::iterator user = mServer.FindUser(*vecIt);
			if (user != mServer.GetUserList().end())
			{
				std::string messages = ChannelMessage(2, commandVec);
				user->second->AppendUserRecvBuf(":" + userIt->second->GetNickName() + " PRIVMSG " + user->second->GetNickName() + " :" + messages + "\r\n");
			}
			else
			{
				mErrManager.ErrorNosuchNick401(*userIt->second, *vecIt);
			}
		}
	}
}

bool Command::CheckBotCommand(std::string comamnd)
{
	if (!strcmp(comamnd.c_str(), ":@BOT"))
		return (true);
	return (false);
}

void Command::BotCommand(int fd, std::vector<std::string> commandVec)
{
	if (commandVec.size() < 3)
	{
		mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second);
		return;
	}
	Channel* channel = mServer.FindChannel(commandVec[1]);
	if (commandVec.size() == 3)
	{
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", mServer.FindChannel(channel->GetChannelName())->GetBot()->Introduce());
		return;
	}
	std::string command = commandVec[3];
	Bot* bot = channel->GetBot();
	if (strcmp(command.c_str(), "list") == 0)
	{
		std::vector<std::string> botCommandList = bot->GetCommandList();
		std::vector<std::string>::iterator it = botCommandList.begin();
		std::string message = "BOT COMMAND LIST : ";
		while (it != botCommandList.end())
		{
			message += *it;
			if (it != botCommandList.end() -1)
				message += ", ";
			it++;
		}
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", message);
	}
	else if (strcmp(command.c_str(), "add") == 0)
	{
		if (commandVec.size() < 6)
		{
			mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second);
			return;
		}
		std::string commandName = commandVec[4];
		std::string commandContent = commandVec[5];
		bot->AddComamnd(commandName, commandContent);
	}
	else if (strcmp(command.c_str(), "del") == 0)
	{
		if (commandVec.size() < 5)
		{
			mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second);
			return;
		}
		std::string commandName = commandVec[4];
		bot->DelCommand(commandName);
	}
	else if (strcmp(command.c_str(), "do") == 0)
	{
		if (commandVec.size() < 5)
		{
			mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second);
			return;
		}
		std::string commandName = commandVec[4];
		std::string response = bot->Docommand(commandName);
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
	}
	else
	{
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", "BOT COMMAND NOT FOUND");
	}
}
