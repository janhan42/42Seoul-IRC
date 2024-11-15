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
	std::map<int, class User*>& userList = mServer.GetUserList();
	std::map<int, class User*>::iterator userIt = userList.find(fd);
	std::string buffer;
	std::vector<std::string> vec;
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*userIt->second, commandVec[1]);
		return;
	}
	if (commandVec[2].empty()) // <message>가 empty일때
	{
		mResponse.ErrorNoTextToSend412(*userIt->second);
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
				mResponse.ErrorNosuchChannel403(*userIt->second, *vecIt);
			}
		}
		else
		{
			std::map<int, class User*>::iterator user = mServer.FindUser(*vecIt);
			if (*vecIt == SERVERNAME) // /PING SIRC처리
				return ;
			if (user != mServer.GetUserList().end())
			{
				std::string messages = ChannelMessage(2, commandVec);
				user->second->AppendUserSendBuf(":" + userIt->second->GetNickName() + " PRIVMSG " + user->second->GetNickName() + " :" + messages + "\r\n");
			}
			else
			{
				mResponse.ErrorNosuchNick401(*userIt->second, *vecIt);
				return ;
			}
		}
	}
}

bool Command::CheckBotCommand(std::string comamnd)
{
	if (!strcmp(comamnd.c_str(), ":@bot"))
		return (true);
	return (false);
}


// TODO: 11월 15일 리펙토링중
void Command::BotCommand(int fd, std::vector<std::string> commandVec)
{
	/* TESTOUTPUT */
	// int index = 0;
	// for (std::vector<std::string>::iterator it = commandVec.begin(); it != commandVec.end(); it++)
	// {
		// std::cout << "commandVec[" << index << "] : " << *it << std::endl;
		// index++;
	// }
	/* END */

	if (commandVec.size() < 3 && commandVec[3] != "help") // 에초에 이 조건문에 들오올수 있나? TODO: 체크 바람
	{
		mResponse.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second, commandVec[1]);
		return;
	}
	Channel* channel = mServer.FindChannel(commandVec[1]);
	if (commandVec.size() == 3) // PRIVMSG <channel Name> @bot
	{
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", mServer.FindChannel(channel->GetChannelName())->GetBot()->Introduce());
		return;
	}

	std::string command = commandVec[3];
	std::cout << command << std::endl;
	Bot* bot = channel->GetBot();
	std::vector<int>::iterator channelInUser;

	if (strcmp(command.c_str(), "help") == 0) // 들어온 명령어 주어가 help면
		bot->HelpMsgtoChannel(this, channel->GetChannelName());
	else if (strcmp(command.c_str(), "buckshot") == 0) // 들어온 명령어 주어가 buckshot이면
	{
		// PRIVMSG <channel Name> @bot buckshot <target>
		if (bot->GameOn() == true || bot->GetReady())
		{
			std::string response = "현재 게임중 입니다. 다른 사람들은 게임을 할 수 없습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		if (commandVec.size() < 5) // PRIVMSG <channel Name> @bot buckshot arg개수 안맞음
		{
			mResponse.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second, commandVec[1]);
			return;
		}
		std::map<int, class User*>::iterator targetUser = mServer.FindUser(commandVec[4]); // <target> 을 서버에서 한번 찾음
		if (targetUser != mServer.GetUserList().end()) // 위에서 서버에서 찾았을때 있다면
			channelInUser = channel->FindMyUserIt(targetUser->second->GetUserFd());
		else
			channelInUser = channel->GetUserFdList().end();
		if (channelInUser == channel->GetUserFdList().end()) // User not in channel
		{
			std::string response = "사용자 [" + commandVec[4] + "] 를 찾을수 없습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		bot->SetFirstUser(mServer.GetUserList().find(fd)->second);
		bot->SetSecondUser(targetUser->second);
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", std::string("유저 [" + bot->GetFirstUser()->GetNickName() + "]가 유저[" + bot->GetSecondUser()->GetNickName() + "] 에게 게임을 신청했습니다."));
		bot->SetReady(true);
	}
	else if (bot->GetReady() == true && *channel->FindMyUserIt(fd) == bot->GetSecondUser()->GetUserFd() && bot->GameOn() == false) // 도전을 받은 유저이고 게임 시작 전이면
	{
		if (commandVec[3] == "accept") // 신청을 받았으면.
		{
			std::string response = "[" + bot->GetSecondUser()->GetNickName() + "]이 게임을 수락했습니다! 다른 유저는 게임이 끝날때 까지 게임을 할수 없습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			bot->SettingGame();
			std::string Chamber = bot->SettingChamber();
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", Chamber);
			int ammo = 5;
			std::string Ammo = "Ammo |";
			while (ammo--)
			{
				Ammo = Ammo + "◼";
			}
			Ammo = Ammo + "|";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", Ammo);
		}
		else if (commandVec[3] == "reject") // 거절을 받았으면
		{
			std::string response = "[" + bot->GetSecondUser()->GetNickName() + "]이 게임을 거절했습니다!";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			bot->ClearGame();
		}
		else
		{
			std::string response = "accept 로 수락 reject로 거절 할 수 있습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
		}
	}
	// 게임 중이고 두명의 유저중에 입력이 들어왔을 경우
	else if (bot->GameOn() == true && (fd == bot->GetFirstUser()->GetUserFd() || fd == bot->GetSecondUser()->GetUserFd()))
	{
		if (commandVec[3] != "me" && commandVec[3] != "other")
		{
			std::string response = "게임중 가능 명령어는 me 또는 other 입니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		if (bot->GetWhoShot() == false) // false 면 첫번쨰 유저 턴
		{
			if (fd == bot->GetSecondUser()->GetUserFd())
			{
				std::string response = "현재 [" + bot->GetFirstUser()->GetNickName() + "]의 턴입니다.";
				MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
				return;
			}
			// logic
			std::string shotReuslt = bot->GameShot(commandVec[3]);
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", shotReuslt);
		}
		else if (bot->GetWhoShot() == true) // true 면 두번째 유저 턴
		{
			if (fd == bot->GetFirstUser()->GetUserFd())
			{
				std::string response = "현재 [" + bot->GetSecondUser()->GetNickName() + "]의 턴입니디.";
				MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
				return;
			}
			// logic
			std::string shotReuslt = bot->GameShot(commandVec[3]);
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", shotReuslt);
			std::cout << "SecondPlayer Shot After Who Shot : " << bot->GetWhoShot() << std::endl;
		}
		int ammo = bot->AmmoCount();
		if (ammo != 0)
		{
			int oringAmmo = 5 - ammo;
			std::string AmmoUnit = "Ammo |";
			std::string Ammo;
			while (ammo--)
			{
				Ammo = Ammo + "◼";
			}
			while (oringAmmo--)
			{
				Ammo = Ammo + "◻︎";
			}
			AmmoUnit = AmmoUnit + Ammo + "|";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", AmmoUnit);
		}
		else if (ammo == 0)
		{
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", "Ammo |◻︎◻︎◻︎◻︎◻︎|");
			std::string Chamber = bot->SettingChamber();
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", Chamber);
			int ammo = 5;
			std::string Ammo = "Ammo |";
			while (ammo--)
			{
				Ammo = Ammo + "◼";
			}
			Ammo = Ammo + "|";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", Ammo);
		}
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", bot->GetFirstHpInfo());
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", bot->GetSecondHpInfo());
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", "---------------------------------------------\033[0m");

		if (bot->GetFirstHp() == 0 || bot->GetSecondHp() == 0)
		{
			std::string response;
			if (bot->GetFirstHp() == 0)
			{
				response = "[" + bot->GetSecondUser()->GetNickName() + "]님이 [" + bot->GetFirstUser()->GetNickName() + "]을 죽였습니다.";
				MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
				response = "승자는 [" + bot->GetSecondUser()->GetNickName() + "]님 입니다.";
				MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			}
			else if (bot->GetSecondHp() == 0)
			{
				response = "[" + bot->GetFirstUser()->GetNickName() + "]님이 [" + bot->GetSecondUser()->GetNickName() + "]을 죽였습니다.";
				MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
				response = "승자는 [" + bot->GetFirstUser()->GetNickName() + "]님 입니다.";
				MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			}
			bot->ClearGame();
		}
	}
	else
	{
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", "BOT COMMAND NOT FOUND");
	}
}
