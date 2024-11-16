#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include "../Bot.hpp"
#include <sstream>
#include <iostream>
#include <vector>

/*
	- RESPONSE LIST -
	ERR_NOSUCHNICK (401)
	ERR_NOSUCHSERVER (402) -> 다중서버 처리가 아니기 때문에 없음
	ERR_CANNOTSENDTOCHAN (404) -> 외부에서 메세지를 보내지 못하는 모드는 구현 안되있기 때문에 없음
	ERR_TOOMANYTARGETS (407) -> 타겟 리밋 변수가 없기 때문에 없음
	ERR_NORECIPIENT (411) -> irssi쪽에서 막아버림
	ERR_NOTEXTTOSEND (412)
	ERR_NOTOPLEVEL (413) -> 구현사항 아님
	ERR_WILDTOPLEVEL (414) -> 구현사항 아님
	RPL_AWAY (301) -> /away <message> 를 한 상대에게 /msg를 했을때 응답인데 해야하나 모르겠음
 */
void Command::Privmsg(int fd, std::vector<std::string> commandVec)
{
	/* PRIVMSG(or /msg) <channel/nickname> <messages...> */
	// std::map<int, class User*>& userList = mServer.GetUserList();
	// std::map<int, class User*>::iterator userIt = userList.find(fd);
	class User* user = mServer.FindUser(fd);

	std::string buffer;
	std::vector<std::string> vec;
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		return;
	}
	if (commandVec[2].empty()) // <message>가 empty일때
	{
		mResponse.ErrorNoTextToSend412(*user);
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
				ChannelPrivmsg(messages, *user, channel);
			}
			else // if Channel not exists
			{
				mResponse.ErrorNosuchChannel403(*user, *vecIt);
			}
		}
		else
		{
			class User* user = mServer.FindUser(*vecIt);
			if (*vecIt == SERVERNAME) // /PING SIRC처리
				return ;
			if (user != NULL)
			{
				std::string messages = ChannelMessage(2, commandVec);

				user->AppendUserSendBuf(":" + user->GetNickName() + " PRIVMSG " + user->GetNickName() + " :" + messages + "\r\n");
			}
			else
			{
				mResponse.ErrorNosuchNick401(*user, *vecIt);
				return ;
			}
		}
	}
}

bool Command::CheckBotCommand(std::string command)
{
	//if (!strcmp(comamnd.c_str(), ":@bot"))
	if (command == ":@bot")
		return (true);
	return (false);
}

void Command::BotCommand(int fd, std::vector<std::string> commandVec)
{
	Channel* channel = mServer.FindChannel(commandVec[1]);
	Bot* bot = channel->GetBot();
	if (commandVec.size() == 3) // PRIVMSG <channel Name> @bot
	{
		MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", bot->Introduce());
		return;
	}
	std::string command = commandVec[3];
	//if (strcmp(command.c_str(), "help") == 0) // 들어온 명령어 가 help면
	if (command == "help")
	{
		bot->HelpMsgtoChannel(this, channel->GetChannelName());
	}
	//else if (strcmp(command.c_str(), "buckshot") == 0) // 들어온 명령어가 buckshot이면
	else if (command == "buckshot")
	{
		// PRIVMSG <channel Name> @bot buckshot <target>
		if (bot->GameOn() == true || bot->GetReady() == true) // 게임 중이거나 수락 대기중이라면
		{
			std::string response = "현재 게임중 입니다. 다른 사람들은 게임을 할 수 없습니다.";
			MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		if (commandVec.size() < 5) // PRIVMSG <channel Name> @bot buckshot arg개수 안맞음
		{
			std::string response = "상대방의 닉네임을 입력해주세요.";
			MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			mResponse.ErrorNeedMoreParams461(*mServer.FindUser(fd), commandVec[1]);
			return;
		}
		bot->ValidTarget(fd, &mServer, channel, this, commandVec);
	}
	else if (bot->GetReady() == true && *channel->FindMyUserIt(fd) == bot->GetSecondUser()->GetUserFd() && bot->GameOn() == false) // 도전을 받은 유저이고 게임 시작 전이면
	{
		bot->ResponseGameRequest(channel, this, commandVec);
	}
	else if (bot->GameOn() == true && (fd == bot->GetFirstUser()->GetUserFd() || fd == bot->GetSecondUser()->GetUserFd())) // 게임 중이고 두명의 유저중에 입력이 들어왔을 경우
	{
		if (commandVec[3] != "me" && commandVec[3] != "other")
		{
			std::string response = "게임중 가능 명령어는 me 또는 other 입니다.";
			MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		bot->GameControl(fd, channel, this, commandVec);
	}
	else
	{
		MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", "BOT COMMAND NOT FOUND");
	}
}
