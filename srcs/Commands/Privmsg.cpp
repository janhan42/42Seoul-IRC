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
	else if (strcmp(command.c_str(), "BuckShot") == 0)
	{
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
		std::cout << "어디서 뻑나는거?" << std::endl;
		Channel* channel = mServer.FindChannel(commandVec[1]);
		std::cout << "FindChannel CommandVec[2] : " << commandVec[1] << std::endl;
		std::cout << "어디서 뻑나는거?" << std::endl;
		std::vector<int>::iterator channelInUser;
		/* TESTOUTPUT */
		std::cout << "채널 User FD 리스트" << std::endl;
		std::vector<int> channelUserList = channel->GetUserFdList();
		std::cout << " 설마 여기겠음?" << std::endl;
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
		std::cout << "어디서 뻑나는거?" << std::endl;
		if (channelInUser == channel->GetUserFdList().end())
		{
			std::string response = "사용자 [" + commandVec[4] + "] 를 찾을수 없습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		bot->SetFirstUser(mServer.GetUserList().find(fd)->second);
		bot->SetSecondUser(targetUser->second);
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", std::string("유저 [" + bot->GetFirstUser()->GetNickName() + "]가 유저[" + bot->GetSecondUser()->GetNickName() + "] 에게 게임을 신청했습니다."));
		std::cout << "FirstUser Name: " << bot->GetFirstUser()->GetNickName() << std::endl;
		std::cout << "SecondUser Name: " << bot->GetSecondUser()->GetNickName() << std::endl;
	}
	/*
	else if (commandVec[2] == bot->GetSecondUser() && bot->GaemOn() == false) // 도전을 받은 유저이고 게임 시작 전이면
	{
		if (commandVec[3] == "accpet") // 신청을 받았으면.
		{
			bot->SettingGame();
			bot->SetGameOn(true);
			std::string response = "[" + commandVec[3] + "] 가 게임을 수락했습니다! 다른 유저는 게임이 끝날때 까지 게임을 할수 없습니다.";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
		}
		else if (commandVec[3] == "reject") // 거절을 받았으면
		{
			bot->ClearGame();
			bot->SetGameOn(false);
			std::string response = "[" + commandVec[3] + "] 가 게임을 거절했습니다!";
			MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", response);
		}
	}
	else if (bot->GetGameOn() == ture && (commandVec[2] == bot->GetFirstUser() || commandVec[2] == bot->GetSecondUser()))
	{
		if (commandVec[2] == bot->GetFirstUser())
		{
			if (commandVec[3] == "ShotMe")
			{
				bot->GameShot(bot->GetFirstUser());
			}
			else if (commandVec[3] == "ShotOther")
			{
				bot->GameShot(bot->GetSecondUser());
			}
		}
		else if (commandVec[2] == bot->GetSecondUser())
		{
			if (commandVec[3] == "ShotMe")
			{
				bot->GameShot(bot->GetSecondUser());
			}
			else if (commandVec[3] == "ShotOther")
			{
				bot->GameShot(bot->GetFirstUser());
			}
		}
		if (!bot->GetFirstUser()->GetPoint() || !bot->GetSecondUser()->GetPoint())
		{
			if(!bot->GetFirstUser()->GetPoint())
			{

			}
			else if (!bot->GetSecondUser()->GetPoint())
			{

			}
			bot->GameClear();
		}
	}
	*/
	else
	{
		MsgToAllChannel(-1, channel->GetChannelName(), "PRIVMSG", "BOT COMMAND NOT FOUND");
	}
}
