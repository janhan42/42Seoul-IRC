#include "Bot.hpp"
#include "Command.hpp"
#include "User.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

Bot::Bot()
: mbGameOn(false)
{}

Bot::~Bot()
{}

const std::string Bot::Introduce()
{
	return "@bot help 로 BOT에 대한 설명을 볼수 있습니다.\r\n";
}

const std::string	Bot::GetHelpBuckShot()
{
	std::string result = " @bot buckshot <target-user>: target-user에게 BuckShot게임을 신청합니다.\r\n";
	return (result);
}

const std::string Bot::GetHelpAccept()
{
	std::string result = " @bot accept: 게임을 수락합니다.\r\n";
	return (result);
}

const std::string Bot::GetHelpReject()
{
	std::string result = " @bot reject: 게임을 거절합니다.\r\n";
	return (result);
}

const std::string Bot::GetHelpGame()
{
	std::string result = " @bot me(자신에게), other(상대방에게)쏩니다, gg(항복을 할수 있습니다).\r\n";
	return (result);
}


// Game
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
	srand((unsigned int)time(NULL));
}

const std::string Bot::SettingChamber()
{
	std::stringstream ss;
	std::string result;
	int ammoCount = 5;
	int fakeround = 0;
	int realround = 0;

	// 두 개의 변수를 조건에 따라 초기화합니다.
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
		if (fakeround > 0 && realround > 0)
		{
			mbAmmoChamber = tempStack;
			valid = true; // 조건이 만족되면 valid를 true로 설정
		}
	}
	ss << "공포탄은 " << fakeround << " 발" << " 실탄은 " << realround << " 발" << "입니다.";
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

// false -> 첫번쨰 플레이어, true -> 두번째 플레이어
// Ammo false -> 공포탄, true -> 실탄
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
				response = response + " 공포탄이였습니다 한번 더 턴을 가져갑니다.";
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " 실탄이였습니다 HP가 1감소하고 턴이 넘어갑니다.";
				mFirstUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
		else if (state == "other")
		{
			response = "[" + mFirstUser->GetNickName() + "]님이 [" + mSecondUser->GetNickName() + "]님을 쐈습니다.";
			if (mbAmmoChamber.top() == false)
			{
				response = response + " 공포탄이였습니다 아무런 이득없이 턴이 넘어갑니다.";
				mbWhoShot = !mbWhoShot;
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " 실탄이였습니다 [" + mSecondUser->GetNickName() + "]님의 HP가 1감소하며, 턴이 넘어갑니다.";
				mSecondUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
	}
	else if (mbWhoShot == true) // 두번째 플레이어
	{
		std::cout << "어드민 유저 테스트" << std::endl;
		if (state == "me")
		{
			response = "[" + mSecondUser->GetNickName() + "]님이 자기 자신을 쐈습니다.";
			std::cout << response << std::endl;
			if (mbAmmoChamber.top() == false)
			{
				response = response + " 공포탄이였습니다 한번 더 턴을 가져갑니다.";
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " 실탄이였습니다 HP가 1감소하고 턴이 넘어갑니다.";
				mSecondUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
		else if (state == "other")
		{
			response = "[" + mSecondUser->GetNickName() + "]님이 [" + mFirstUser->GetNickName() + "]님을 쐈습니다.";
			if (mbAmmoChamber.top() == false)
			{
				response = response + " 공포탄이였습니다 아무런 이득없이 턴이 넘어갑니다.";
				mbWhoShot = !mbWhoShot;
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " 실탄이였습니다 [" + mFirstUser->GetNickName() + "]님의 HP가 1감소하며, 턴이 넘어갑니다.";
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
// 도박봇을 만들꺼임
// @BOT <game> <targetUser> // 상대 유저에게 게임 신청
// @BOT <accept> // 신청 수락
// @BOT <reject> // 신청 거절
// 게임 rand로 5개의 총알중 실탄과 공포탄 개수 지정
// Rand로 랜덤으로 장전 <std::vector or queue>
// @BOT shotme or shotother
	// std::vector or queue::iterator it->bullet
	// 현재 장전되어있는 총알이 실탄인지 공포탄인지에 따라 HP 조정
// 둘중에 피가 0이 되는 순간 게임 끝 피가 남아있는 플레이어에게 betting Point * 1.5 배당

