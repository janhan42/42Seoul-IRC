#include "Bot.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Server.hpp"
#include "User.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

Bot::Bot()
: mbGameOn(false)
{}

Bot::~Bot()
{}

void Bot::HelpMsgtoChannel(Command* command, std::string channel)
{
	command->MsgToAllChannel(BOT, channel, "PRIVMSG", " @bot buckshot <target-user>: target-user에게 BuckShot게임을 신청합니다.\r\n");
	command->MsgToAllChannel(BOT, channel, "PRIVMSG", " @bot accept: 게임을 수락합니다.\r\n");
	command->MsgToAllChannel(BOT, channel, "PRIVMSG", " @bot reject: 게임을 거절합니다.\r\n");
	command->MsgToAllChannel(BOT, channel, "PRIVMSG", " @bot me(자신에게), other(상대방에게)쏩니다.\r\n");
}

void Bot::ValidTarget(int fd, Server* server, Channel* channel, Command* command, std::vector<std::string> commandVec)
{
	std::vector<int>::iterator channelInUser;
	class User* targetUser = server->FindUser(commandVec[4]); // <target> 을 서버에서 한번 찾음
	if (targetUser != NULL) // 서버에 있고 채널에 있는지 확인
		channelInUser = channel->FindMyUserIt(targetUser->GetUserFd());
	else
		channelInUser = channel->GetUserFdList().end();
	if (channelInUser == channel->GetUserFdList().end()) // User not in channel
	{
		std::string response = "사용자 [" + commandVec[4] + "] 를 찾을수 없습니다.";
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
		return;
	}
	SetFirstUser(server->GetUserList().find(fd)->second);
	SetSecondUser(targetUser);
	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", std::string("유저 [" + GetFirstUser()->GetNickName() + "]가 유저[" + GetSecondUser()->GetNickName() + "] 에게 게임을 신청했습니다."));
	SetReady(true);
}

void Bot::ResponseGameRequest(Channel* channel, Command* command, std::vector<std::string> commandVec)
{
	if (commandVec[3] == "accept") // 신청을 받았으면.
	{
		std::string response = "[" + GetSecondUser()->GetNickName() + "]이 게임을 수락했습니다! 다른 유저는 게임이 끝날때 까지 게임을 할수 없습니다.";
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
		SettingGame();
		std::string Chamber = SettingChamber();
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", Chamber);
		int ammo = 5;
		std::string Ammo = "Ammo |";
		while (ammo--)
		{
			Ammo = Ammo + "◼";
		}
		Ammo = Ammo + "|";
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", Ammo);
	}
	else if (commandVec[3] == "reject") // 거절을 받았으면
	{
		std::string response = "[" + GetSecondUser()->GetNickName() + "]이 게임을 거절했습니다!";
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
		ClearGame();
	}
	else
	{
		std::string response = "accept 로 수락 reject로 거절 할 수 있습니다.";
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
	}
}

void Bot::GameControl(int fd, Channel* channel, Command* command, std::vector<std::string> commandVec)
{
/* 빼내기 Game Control */
/* 빼내기 Handle Game Turn */
	HandleGameTurn(fd, command, channel, commandVec);
	GameInfo(command, channel);
	CheckWinner(command, channel);
	// if (GetWhoShot() == false) // false 면 첫번쨰 유저 턴
	// {
	// 	if (fd == GetSecondUser()->GetUserFd())
	// 	{
	// 		std::string response = "현재 [" + GetFirstUser()->GetNickName() + "]의 턴입니다.";
	// 		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
	// 		return;
	// 	}
	// 	std::string shotReuslt = GameShot(commandVec[3]);
	// 	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", shotReuslt);
	// }
	// else if (GetWhoShot() == true) // true 면 두번째 유저 턴
	// {
	// 	if (fd == GetFirstUser()->GetUserFd())
	// 	{
	// 		std::string response = "현재 [" + GetSecondUser()->GetNickName() + "]의 턴입니디.";
	// 		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
	// 		return;
	// 	}
	// 	std::string shotReuslt = GameShot(commandVec[3]);
	// 	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", shotReuslt);
	// }
	// if (GetFirstHp() == 0 || GetSecondHp() == 0)
	// {
	// 	std::string response;
	// 	if (GetFirstHp() == 0)
	// 	{
	// 		response = "[" + GetSecondUser()->GetNickName() + "]님이 [" + GetFirstUser()->GetNickName() + "]을 죽였습니다.";
	// 		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
	// 		response = "승자는 [" + GetSecondUser()->GetNickName() + "]님 입니다.";
	// 		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
	// 	}
	// 	else if (GetSecondHp() == 0)
	// 	{
	// 		response = "[" + GetFirstUser()->GetNickName() + "]님이 [" + GetSecondUser()->GetNickName() + "]을 죽였습니다.";
	// 		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
	// 		response = "승자는 [" + GetFirstUser()->GetNickName() + "]님 입니다.";
	// 		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
	// 	}
	// 	ClearGame();
	// }
	/* END */
/* END */
}

void Bot::HandleGameTurn(int fd, Command* command, Channel* channel,  std::vector<std::string> commandVec)
{
	if (GetWhoShot() == false) // false 면 첫번쨰 유저 턴
	{
		if (fd == GetSecondUser()->GetUserFd())
		{
			std::string response = "현재 [" + GetFirstUser()->GetNickName() + "]의 턴입니다.";
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		std::string shotReuslt = GameShot(commandVec[3]);
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", shotReuslt);
	}
	else if (GetWhoShot() == true) // true 면 두번째 유저 턴
	{
		if (fd == GetFirstUser()->GetUserFd())
		{
			std::string response = "현재 [" + GetSecondUser()->GetNickName() + "]의 턴입니디.";
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		std::string shotReuslt = GameShot(commandVec[3]);
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", shotReuslt);
	}
}

void Bot::GameInfo(Command* command, Channel* channel)
{
	/* 뺴내기 Game Info */
	int ammo = AmmoCount();
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
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", AmmoUnit);
	}
	else if (ammo == 0)
	{
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", "Ammo |◻︎◻︎◻︎◻︎◻︎|");
		std::string Chamber = SettingChamber();
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", Chamber);
		int ammo = 5;
		std::string Ammo = "Ammo |";
		while (ammo--)
		{
			Ammo = Ammo + "◼";
		}
		Ammo = Ammo + "|";
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", Ammo);
	}
	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", GetFirstHpInfo());
	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", GetSecondHpInfo());
	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", "---------------------------------------------\033[0m");
	/* END */
}

void Bot::CheckWinner(Command* command, Channel* channel)
{
	std::string response;
	if (GetFirstHp() == 0 || GetSecondHp() == 0)
	{
		if (GetFirstHp() == 0)
		{
			response = "[" + GetSecondUser()->GetNickName() + "]님이 [" + GetFirstUser()->GetNickName() + "]을 죽였습니다.";
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			response = "승자는 [" + GetSecondUser()->GetNickName() + "]님 입니다.";
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
		}
		else if (GetSecondHp() == 0)
		{
			response = "[" + GetFirstUser()->GetNickName() + "]님이 [" + GetSecondUser()->GetNickName() + "]을 죽였습니다.";
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			response = "승자는 [" + GetFirstUser()->GetNickName() + "]님 입니다.";
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
		}
		ClearGame();
	}
}

const std::string Bot::Introduce()
{
	return "@bot help 로 BOT에 대한 설명을 볼수 있습니다.\r\n";
}

class User*	Bot::GetFirstUser(void)
{
	return (mFirstUser);
}

class User*	Bot::GetSecondUser(void)
{
	return (mSecondUser);
}

const std::string Bot::GetFirstHpInfo()
{
	std::string result = "[" + mFirstUser->GetNickName() + " HP] ";
	int userHp = mFirstUserHp;
	int count = 5 - mFirstUserHp;
	while (userHp--)
	{
		result = result + "◼";
	}
	while (count--)
	{
		result = result + "◻︎";
	}
	return (result);
}

const std::string Bot::GetSecondHpInfo()
{
	std::string result = "[" + mSecondUser->GetNickName() + " HP] ";
	int userHp = mSecondUserHp;
	int count = 5 - mSecondUserHp;
	while (userHp--)
	{
		result = result + "◼";
	}
	while (count--)
	{
		result = result + "◻︎";
	}
	return (result);
}

int Bot::GetFirstHp()
{
	return (mFirstUserHp);
}

int Bot::GetSecondHp()
{
	return (mSecondUserHp);
}

void Bot::SetFirstUser(class User* firstUser)
{
	mFirstUser = firstUser;
}

void Bot::SetSecondUser(class User* secondUser)
{
	mSecondUser = secondUser;
}

void Bot::SettingGame()
{
	mFirstUserHp = 5;		// FirstUserHp
	mSecondUserHp = 5;		// SecondUserHp;
	mbGameOn = true;		// Game상태 true;
	mbWhoShot = false;		// false 면 FirstUser true면 SecondUser
}

const std::string Bot::SettingChamber()
{
	srand((unsigned int)time(NULL));
	std::stringstream ss;
	std::string result;
	int ammoCount = 5;
	int fakeround = 0;
	int realround = 0;
	bool valid = false;
	while (!valid)
	{
		std::stack<bool> tempStack;
		fakeround = 0;
		realround = 0;
		for (int i = 0; i < ammoCount; ++i)
		{
			int temp = rand() % 2;
			if (temp == 1)
				realround++;
			else
				fakeround++;
			std::cout << temp << std::endl;
			tempStack.push(temp);
		}
		// 둘 다 0이 아닌지 체크
		if (fakeround > 0 && realround > 2)
		{
			mbAmmoChamber = tempStack;
			valid = true; // 조건이 만족되면 valid를 true로 설정
		}
	}
	ss << "\033[3m\033[32m공포탄\033[0m은 " << fakeround << " 발" << " \033[3m\033[31m실탄\033[0m은 " << realround << " 발" << "입니다.";
	result = ss.str();
	return result;
}

void Bot::SetReady(bool state)
{
	mbGameReady = state;
}

bool Bot::GetReady()
{
	return (mbGameReady);
}

void Bot::ClearGame()
{
	mFirstUser = NULL;		// Bot의 FristUser 초기화
	mSecondUser = NULL;		// Bot의 SecondUser 초기화
	mbGameOn = false;		// Game상태 false;
	mbGameReady = false;	// 게임 수락 여부
}

bool Bot::GameOn()
{
	return (mbGameOn);
}

bool Bot::GetWhoShot()
{
	return (mbWhoShot);
}

void Bot::SetWhoShot(bool state)
{
	mbWhoShot = state;
}

const std::string Bot::GameShot(const std::string& state)
{
	std::string response;
	if (mbWhoShot == false) // 첫번째 플레이어
	{
		if (state == "me")
		{
			response = "[" + mFirstUser->GetNickName() + "]님이 자기 자신을 쐈습니다.";
			if (mbAmmoChamber.top() == false)
			{
				response = response + " \033[3m\033[32m공포탄이였습니다\033[0m 한번 더 턴을 가져갑니다.";
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " \033[3m\033[31m실탄이였습니다\033[0m HP가 1감소하고 턴이 넘어갑니다.";
				mFirstUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
		else if (state == "other")
		{
			response = "[" + mFirstUser->GetNickName() + "]님이 [" + mSecondUser->GetNickName() + "]님을 쐈습니다.";
			if (mbAmmoChamber.top() == false)
			{
				response = response + " \033[3m\033[32m공포탄이였습니다\033[0m 아무런 이득없이 턴이 넘어갑니다.";
				mbWhoShot = !mbWhoShot;
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " \033[3m\033[31m실탄이였습니다\033[0m [" + mSecondUser->GetNickName() + "]님의 HP가 1감소하며, 턴이 넘어갑니다.";
				mSecondUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
	}
	else if (mbWhoShot == true) // 두번째 플레이어
	{
		if (state == "me")
		{
			response = "[" + mSecondUser->GetNickName() + "]님이 자기 자신을 쐈습니다.";
			std::cout << response << std::endl;
			if (mbAmmoChamber.top() == false)
			{
				response = response + " \033[3m\033[32m공포탄이였습니다\033[0m 한번 더 턴을 가져갑니다.";
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " \033[3m\033[31m실탄이였습니다\033[0m HP가 1감소하고 턴이 넘어갑니다.";
				mSecondUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
		else if (state == "other")
		{
			response = "[" + mSecondUser->GetNickName() + "]님이 [" + mFirstUser->GetNickName() + "]님을 쐈습니다.";
			if (mbAmmoChamber.top() == false)
			{
				response = response + " \033[3m\033[32m공포탄이였습니다\033[0m 아무런 이득없이 턴이 넘어갑니다.";
				mbWhoShot = !mbWhoShot;
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " \033[3m\033[31m실탄이였습니다\033[0m [" + mFirstUser->GetNickName() + "]님의 HP가 1감소하며, 턴이 넘어갑니다.";
				mFirstUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
	}
	mbAmmoChamber.pop();
	return (response);
}

int	Bot::AmmoCount()
{
	return (mbAmmoChamber.size());
}
