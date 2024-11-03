#include "Bot.hpp"
#include "Command.hpp"
#include "User.hpp"
#include <cstdlib>
#include <iostream>

Bot::Bot()
: mbGameOn(false)
{}

Bot::~Bot()
{}

std::string Bot::Introduce()
{
	return "@BOT help 로 BOT에 대한 설명을 볼수 있습니다.";
}

std::string	Bot::GetHelpBuckShot()
{
	std::string result = " @BOT BuckShot <target-user>: target-user에게 BuckShot게임을 신청합니다.\n";
	return (result);
}

std::string Bot::GetHelpShop()
{
	std::string result = " @BOT POINTSHOP: 구매가능한 아이템들을 보여줍니다.\n";
	return (result);
}

std::string Bot::GetHelpBuy()
{
	std::string result = " @BOT BUY <item-name>: item-name을 구매합니다.\r\n";
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
	int ammoCount = 5;
	mFirstUserHp = 5;		// FirstUserHp
	mSecondUserHp = 5;		// SecondUserHp;
	mbGameOn = true;		// Game상태 true;
	mbWhoShot = false;		// false 면 FirstUser true면 SecondUser
	srand((unsigned int)time(NULL));
	while (ammoCount)
	{
		mbAmmoChamber.push(rand() % 2);
		ammoCount--;
	}
	while (ammoCount < 5)
	{
		bool temp = mbAmmoChamber.top();
		mbAmmoChamber.pop();
		std::cout << temp << std::endl;
		ammoCount++;
	}
}

void Bot::ClearGame()
{
	mFirstUser = NULL;		// Bot의 FristUser 초기화
	mSecondUser = NULL;		// Bot의 SecondUser 초기화
	mbGameOn = false;		// Game상태 false;
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

void Bot::GameShot(const std::string& state)
{
	if (state == "me") // 자기 자신 쏘기
	{
		if (mbAmmoChamber.top() == true) // 실탄
		{
			if (mbWhoShot == false) // 현재 턴이 첫번째 플레이어면
			{
				mFirstUserHp--; // 첫번째 유저의 hp를 1 감소
				// 메세지 센드해야할거 같은데 채팅방에
				mbAmmoChamber.pop(); // 총알 빼기

				mbWhoShot = !mbWhoShot; // 턴 변경
			}
			else if (mbWhoShot == true) // 현재 턴이 두번째 플레이어면
			{
				mSecondUserHp--; // 두번쨰 유저의 Hp를 1 감소
				mbAmmoChamber.pop(); // 총알 뺴기

				mbWhoShot = !mbWhoShot; // 턴 변경
			}
		}
		else if (mbAmmoChamber.top() == false) // 공포탄
		{
			if (mbWhoShot == false) // 현재 턴이 첫번쨰 플레이어면
			{
				mbAmmoChamber.pop();
				// 메세지 출력
			}
			else if (mbWhoShot == true)
			{
				mbAmmoChamber.pop();
				mbWhoShot = !mbWhoShot;
			}
		}
	}
	else if (state == "other") // 다른사람 쏘기
	{
	}
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

