#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include "../Bot.hpp"
#include <sstream>
#include <iostream>

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
				std::cout << "PRIVMSG USER NICK : " << userIt->second->GetNickName() << std::endl;
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
				std::cout << "PRIVMSG USER NICK : " << userIt->second->GetNickName() << std::endl;
				user->second->AppendUserSendBuf(":" + userIt->second->GetNickName() + " PRIVMSG " + user->second->GetNickName() + " :" + messages + "\r\n");
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
	if (!strcmp(comamnd.c_str(), ":@bot"))
		return (true);
	return (false);
}

void Command::BotCommand(int fd, std::vector<std::string> commandVec)
{
	if (commandVec.size() < 3 && commandVec[3] != "help")
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
	std::cout << command << std::endl;
	Bot* bot = channel->GetBot();
	if (strcmp(command.c_str(), "help") == 0)
	{
		std::cout << commandVec.size() << std::endl;
		if (commandVec.size() != 4)
		{
			mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second);
			return;
		}
		std::string response = bot->GetHelpBuckShot();
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
		response = bot->GetHelpShop();
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
		response = bot->GetHelpBuy();
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
	}
	else if (strcmp(command.c_str(), "buckshot") == 0)
	{
		if (bot->GameOn() == true)
		{
			std::string response = "현재 게임중 입니다. 다른 사람들은 게임을 할 수 없습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		// TEST OUTPUT
		std::cout << commandVec.size() << std::endl;

		if (commandVec.size() < 5) // arg error
		{
			mErrManager.ErrorNeedMoreParams461(*mServer.GetUserList().find(fd)->second);
			return;
		}
		std::map<int, class User*>::iterator targetUser = mServer.FindUser(commandVec[4]);

		/* TESTOUTPUT */
		std::cout << "commandVec[4] : " << commandVec[4] << std::endl;
		std::cout << "서버 유저 리스트" << std::endl;
		for (std::map<int, class User*>::iterator it = mServer.GetUserList().begin(); it != mServer.GetUserList().end(); it++)
		{
			std::cout << it->second->GetNickName() << std::endl;
		}
		if (targetUser != mServer.GetUserList().end())
			std::cout << "targetUser Nick :" << targetUser->second->GetNickName() << std::endl;
		/* END */

		Channel* channel = mServer.FindChannel(commandVec[1]);

		// TEST OUTPUT
		std::cout << "FindChannel CommandVec[2] : " << commandVec[1] << std::endl;
		std::vector<int>::iterator channelInUser;

		/* TESTOUTPUT */
		std::cout << "채널 User FD 리스트" << std::endl;
		std::vector<int> channelUserList = channel->GetUserFdList();
		for (std::vector<int>::iterator it = channelUserList.begin(); it != channelUserList.end(); it++)
		{
			std::cout << *it << std::endl;
		}
		std::cout << "채널 User FD 리스트 끝 " << std::endl << std::endl;
		/* END */

		if (targetUser != mServer.GetUserList().end())
			channelInUser = channel->FindMyUserIt(targetUser->second->GetUserFd());
		else
			channelInUser = channel->GetUserFdList().end();
		if (channelInUser == channel->GetUserFdList().end())
		{
			std::string response = "사용자 [" + commandVec[4] + "] 를 찾을수 없습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		bot->SetFirstUser(mServer.GetUserList().find(fd)->second);
		bot->SetSecondUser(targetUser->second);
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", std::string("유저 [" + bot->GetFirstUser()->GetNickName() + "]가 유저[" + bot->GetSecondUser()->GetNickName() + "] 에게 게임을 신청했습니다."));

		/* TESTOUTPUT */
		std::cout << "FirstUser Name: " << bot->GetFirstUser()->GetNickName() << std::endl;
		std::cout << "SecondUser Name: " << bot->GetSecondUser()->GetNickName() << std::endl;
		/* END */
	}
	else if (*channel->FindMyUserIt(fd) == bot->GetSecondUser()->GetUserFd() && bot->GameOn() == false) // 도전을 받은 유저이고 게임 시작 전이면
	{
		/* TESTOUTPUT */
		std::cout << "Accept : 들어오긴하나" << std::endl;
		std::cout << "commandVec[3]: " << commandVec[3] << std::endl;
		/* END */

		if (commandVec[3] == "accept") // 신청을 받았으면.
		{
			bot->SettingGame();
			std::string response = "[" + bot->GetSecondUser()->GetNickName() + "]이 게임을 수락했습니다! 다른 유저는 게임이 끝날때 까지 게임을 할수 없습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
		}
		else if (commandVec[3] == "reject") // 거절을 받았으면
		{
			std::string response = "[" + bot->GetSecondUser()->GetNickName() + "]이 게임을 거절했습니다!";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			bot->ClearGame();
		}
	}
	// 게임 중이고 두명의 유저중에 입력이 들어왔을 경우
	else if (bot->GameOn() == true && (fd == bot->GetFirstUser()->GetUserFd() || fd == bot->GetSecondUser()->GetUserFd()))
	{
		if (bot->GetWhoShot() == false) // false 면 첫번쨰 유저 턴
		{
			if (fd == bot->GetSecondUser()->GetUserFd())
			{
				std::string response = "현재 [" + bot->GetFirstUser()->GetNickName() + "]의 턴입니다.";
				MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
				return;
			}
			// logic
			// changeWhoSHot
			bot->SetWhoShot(true);
			std::cout << "FirstPlayer Shot After Who Shot : " << bot->GetWhoShot() << std::endl;
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

			// changeWhoShot
			bot->SetWhoShot(false);
			std::cout << "SecondPlayer Shot After Who Shot : " << bot->GetWhoShot() << std::endl;
		}
	}
	else
	{
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", "BOT COMMAND NOT FOUND");
	}
}
