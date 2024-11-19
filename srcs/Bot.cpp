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

/**
 * @brief
 * 채널에 BuckShot 게임 관련 도움말 메시지를 전송하는 함수
 * @param command Command 객체로, 채널에 메시지를 전송하는 기능을 제공
 * @param channel 도움말 메시지를 전송할 대상 채널
 */
void Bot::HelpMsgtoChannel(Command* command, std::string channel)
{
	command->MsgToAllChannel(BOT, channel, "PRIVMSG", " @bot buckshot <target-user>: target-user에게 BuckShot게임을 신청합니다.\r\n");
	command->MsgToAllChannel(BOT, channel, "PRIVMSG", " @bot accept: 게임을 수락합니다.\r\n");
	command->MsgToAllChannel(BOT, channel, "PRIVMSG", " @bot reject: 게임을 거절합니다.\r\n");
	command->MsgToAllChannel(BOT, channel, "PRIVMSG", " @bot me(자신에게), other(상대방에게)쏩니다.\r\n");
}


/**
 * @brief
 * 주어진 파일 디스크립터를 통해 타겟 사용자가 채널에 있는지 확인하고,
 * 게임 신청을 하는 함수
 * @param fd 파일 디스크립터로, 게임을 신청하는 유저의 ID
 * @param server 서버 객체로, 서버에서 유저를 찾는 데 사용
 * @param channel 채널 객체로, 해당 채널에서 타겟 유저의 존재 여부를 확인
 * @param command Command 객체로, 채널에 메시지를 전송하는 기능을 제공
 * @param commandVec 입력된 명령어를 담고 있는 벡터
 */
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
	SetFirstUser(server->FindUser(fd));
	SetSecondUser(targetUser);
	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", std::string("유저 [" + GetFirstUser()->GetNickName() + "]가 유저[" + GetSecondUser()->GetNickName() + "] 에게 게임을 신청했습니다."));
	SetReady(true);
}


/**
 * @brief
 * 게임 신청에 대한 응답을 처리하는 함수
 * @param channel 채널 객체로, 해당 채널에서 메시지를 전송하는 데 사용
 * @param command Command 객체로, 채널에 메시지를 전송하는 기능을 제공
 * @param commandVec 입력된 명령어를 담고 있는 벡터
 * 이 함수는 유저가 게임을 수락하거나 거절했을 때 응답 메시지를 채널에 보내고,
 * 게임을 시작하거나 종료하는 로직을 포함한다.
 */
void Bot::ResponseGameRequest(Channel* channel, Command* command, std::vector<std::string> commandVec)
{
	if (commandVec[3] == "accept") // 신청을 받았으면.
	{
		std::string response = "[" + GetSecondUser()->GetNickName() + "]이 게임을 수락했습니다! 다른 유저는 게임이 끝날때 까지 게임을 할수 없습니다.";
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
		SettingGame(); // 게임 설정을 초기화
		std::string Chamber = SettingChamber(); // 전투 환경 설정
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", Chamber);
		int ammo = 5; // 초기 총알 수
		std::string Ammo = "Ammo |";
		while (ammo--) // 총알 상태 표시
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
		ClearGame(); // 게임을 종료
	}
	else
	{
		std::string response = "accept 로 수락 reject로 거절 할 수 있습니다."; // 잘못된 입력에 대한 안내
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
	}
}

/**
 * @brief
 * 게임의 전체 진행을 제어하는 함수
 * @param fd 게임을 진행하는 유저의 파일 디스크립터
 * @param channel 게임이 진행되고 있는 채널 객체
 * @param command Command 객체로, 채널에 메시지를 전송하는 기능을 제공
 * @param commandVec 입력된 명령어를 담고 있는 벡터
 * 이 함수는 게임의 각 턴을 처리하고, 게임 정보를 업데이트하며, 승자를 확인하는 역할을 한다.
 */
void Bot::GameControl(int fd, Channel* channel, Command* command, std::vector<std::string> commandVec)
{
	HandleGameTurn(fd, command, channel, commandVec); // 게임 턴 처리
	GameInfo(command, channel); // 게임 정보 업데이트
	CheckWinner(command, channel); // 승자 확인
}

/**
 * @brief
 * 게임 턴을 처리하는 함수
 * @param fd 게임을 진행하는 유저의 파일 디스크립터
 * @param command Command 객체로, 채널에 메시지를 전송하는 기능을 제공
 * @param channel 게임이 진행되고 있는 채널 객체
 * @param commandVec 입력된 명령어를 담고 있는 벡터
 * 이 함수는 게임의 턴을 처리하고 각 턴에 해당하는 유저가 샷을 쐈을 때 그 결과를 전송한다.
 */
void Bot::HandleGameTurn(int fd, Command* command, Channel* channel,  std::vector<std::string> commandVec)
{
	if (GetWhoShot() == false) // false 면 첫번쨰 유저 턴
	{
		if (fd == GetSecondUser()->GetUserFd()) // 두 번째 유저가 턴을 시작하면, 첫 번째 유저의 턴이라고 알림
		{
			std::string response = "현재 [" + GetFirstUser()->GetNickName() + "]의 턴입니다.";
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		// 첫 번째 유저가 샷을 쐈다면 그 결과를 출력
		std::string shotReuslt = GameShot(commandVec[3]);
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", shotReuslt);
	}
	else if (GetWhoShot() == true) // true 면 두번째 유저 턴
	{
		if (fd == GetFirstUser()->GetUserFd()) // 첫 번째 유저가 턴을 시작하면, 두 번째 유저의 턴이라고 알림
		{
			std::string response = "현재 [" + GetSecondUser()->GetNickName() + "]의 턴입니디.";
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		// 두 번째 유저가 샷을 쐈다면 그 결과를 출력
		std::string shotReuslt = GameShot(commandVec[3]);
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", shotReuslt);
	}
}


/**
 * @brief
 * 게임 상태와 정보를 채널에 전달하는 함수
 * @param command Command 객체로, 채널에 메시지를 전송하는 기능을 제공
 * @param channel 게임이 진행되고 있는 채널 객체
 * 이 함수는 게임의 현재 상태, 탄약의 수, 체력 정보 등을 채널에 전송한다.
 */
void Bot::GameInfo(Command* command, Channel* channel)
{
	int ammo = AmmoCount(); // 현재 남은 탄약의 수를 가져옴
	if (ammo != 0) // 남은 탄약이 있을 경우
	{
		int oringAmmo = 5 - ammo; // 원래 총 탄약 수에서 남은 탄약을 빼서, 남은 탄약의 개수를 구함
		std::string AmmoUnit = "Ammo |"; // 탄약 표시 문자열 초기화
		std::string Ammo;
		while (ammo--) // 남은 탄약은 '◼'로 표시
		{
			Ammo = Ammo + "◼";
		}
		while (oringAmmo--) // 사용한 탄약은 '◻︎'로 표시
		{
			Ammo = Ammo + "◻︎";
		}
		AmmoUnit = AmmoUnit + Ammo + "|"; // 전체 표시 문자열에 합침
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", AmmoUnit); // 채널에 탄약 상태 전송
	}
	else if (ammo == 0) // 탄약이 다 떨어졌을 경우
	{
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", "Ammo |◻︎◻︎◻︎◻︎◻︎|"); // 기본적으로 빈 탄약 표시
		std::string Chamber = SettingChamber(); // 새 탄창 설정
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", Chamber); // 새로운 탄창 상태 전송
		int ammo = 5; // 탄약을 다시 채운 상태
		std::string Ammo = "Ammo |";
		while (ammo--) // 새 탄약은 '◼'로 표시
		{
			Ammo = Ammo + "◼";
		}
		Ammo = Ammo + "|";
		command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", Ammo); // 채널에 새 탄약 상태 전송
	}
	// 첫 번째 유저와 두 번째 유저의 체력 정보 전송
	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", GetFirstHpInfo());
	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", GetSecondHpInfo());
	// 구분선 출력
	command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", "---------------------------------------------\033[0m");
}

/**
 * @brief
 * 게임의 승자를 확인하고 결과를 채널에 전송하는 함수
 * @param command Command 객체로, 채널에 메시지를 전송하는 기능을 제공
 * @param channel 게임이 진행되고 있는 채널 객체
 * 이 함수는 양측의 체력을 확인하여, 체력이 0인 유저가 있을 경우 승자를 발표하고 게임을 종료한다.
 */
void Bot::CheckWinner(Command* command, Channel* channel)
{
	std::string response;
	if (GetFirstHp() == 0 || GetSecondHp() == 0) // 첫 번째 또는 두 번째 유저의 체력이 0인지 확인
	{
		if (GetFirstHp() == 0) // 첫 번째 유저가 체력이 0일 경우
		{
			response = "[" + GetSecondUser()->GetNickName() + "]님이 [" + GetFirstUser()->GetNickName() + "]을 죽였습니다."; // 승리 메시지
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response); // 채널에 전송
			response = "승자는 [" + GetSecondUser()->GetNickName() + "]님 입니다."; // 승자 발표
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response); // 채널에 전송
		}
		else if (GetSecondHp() == 0) // 두 번째 유저가 체력이 0일 경우
		{
			response = "[" + GetFirstUser()->GetNickName() + "]님이 [" + GetSecondUser()->GetNickName() + "]을 죽였습니다."; // 승리 메시지
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response); // 채널에 전송
			response = "승자는 [" + GetFirstUser()->GetNickName() + "]님 입니다."; // 승자 발표
			command->MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response); // 채널에 전송
		}
		ClearGame(); // 게임 상태를 초기화
	}
}

/**
 * @brief
 * BOT에 대한 소개 메시지를 반환하는 함수
 * @return 소개 메시지
 */
const std::string Bot::Introduce()
{
	return "@bot help 로 BOT에 대한 설명을 볼수 있습니다.\r\n";
}

/**
 * @brief
 * 첫 번째 사용자 객체를 반환하는 함수
 * @return 첫 번째 사용자 객체
 */
class User* Bot::GetFirstUser(void)
{
	return (mFirstUser);
}

/**
 * @brief
 * 두 번째 사용자 객체를 반환하는 함수
 * @return 두 번째 사용자 객체
 */
class User* Bot::GetSecondUser(void)
{
	return (mSecondUser);
}

/**
 * @brief
 * 첫 번째 사용자의 체력 정보를 문자열로 반환하는 함수
 * @return 첫 번째 사용자의 체력 정보
 */
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

/**
 * @brief
 * 두 번째 사용자의 체력 정보를 문자열로 반환하는 함수
 * @return 두 번째 사용자의 체력 정보
 */
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

/**
 * @brief
 * 첫 번째 사용자의 체력을 반환하는 함수
 * @return 첫 번째 사용자의 체력
 */
int Bot::GetFirstHp()
{
	return (mFirstUserHp);
}

/**
 * @brief
 * 두 번째 사용자의 체력을 반환하는 함수
 * @return 두 번째 사용자의 체력
 */
int Bot::GetSecondHp()
{
	return (mSecondUserHp);
}

/**
 * @brief
 * 첫 번째 사용자를 설정하는 함수
 * @param firstUser 설정할 첫 번째 사용자 객체
 */
void Bot::SetFirstUser(class User* firstUser)
{
	mFirstUser = firstUser;
}

/**
 * @brief
 * 두 번째 사용자를 설정하는 함수
 * @param secondUser 설정할 두 번째 사용자 객체
 */
void Bot::SetSecondUser(class User* secondUser)
{
	mSecondUser = secondUser;
}

/**
 * @brief
 * 게임 초기화 함수. 사용자의 체력을 설정하고 게임 상태를 활성화
 */
void Bot::SettingGame()
{
	mFirstUserHp = 5;		// FirstUserHp
	mSecondUserHp = 5;		// SecondUserHp;
	mbGameOn = true;		// 게임 상태를 true로 설정
	mbWhoShot = false;		// false면 첫 번째 유저, true면 두 번째 유저 차례
}

/**
 * @brief
 * 실탄과 공포탄을 설정하는 함수. 랜덤으로 실탄과 공포탄의 개수를 설정하고, 결과를 문자열로 반환
 * @return 실탄과 공포탄의 개수 정보
 */
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
		if (fakeround > 0 && realround > 2) // 공포탄은 적어도 1발, 실탄은 적어도 3발로 세팅
		{
			mbAmmoChamber = tempStack;
			valid = true; // 조건 만족 시 유효
		}
	}
	ss << "\033[3m\033[32m공포탄\033[0m은 " << fakeround << " 발" << " \033[3m\033[31m실탄\033[0m은 " << realround << " 발" << "입니다.";
	result = ss.str();
	return result;
}

/**
 * @brief
 * 게임 준비 상태를 설정하는 함수
 * @param state 게임 준비 상태 (true 또는 false)
 */
void Bot::SetReady(bool state)
{
	mbGameReady = state;
}

/**
 * @brief
 * 게임 준비 상태를 반환하는 함수
 * @return 게임 준비 상태 (true 또는 false)
 */
bool Bot::GetReady()
{
	return (mbGameReady);
}

/**
 * @brief
 * 게임 정보를 초기화하는 함수. 모든 사용자와 게임 상태를 초기화
 */
void Bot::ClearGame()
{
	mFirstUser = NULL;		// 첫 번째 사용자 초기화
	mSecondUser = NULL;		// 두 번째 사용자 초기화
	mbGameOn = false;		// 게임 상태 false
	mbGameReady = false;	// 게임 준비 상태 false
}

/**
 * @brief
 * 게임이 진행 중인지를 확인하는 함수
 * @return 게임 진행 여부 (true 또는 false)
 */
bool Bot::GameOn()
{
	return (mbGameOn);
}

/**
 * @brief
 * 현재 턴을 쏜 유저를 반환하는 함수
 * @return 현재 턴을 쏜 유저 (true 또는 false)
 */
bool Bot::GetWhoShot()
{
	return (mbWhoShot);
}

/**
 * @brief
 * 다음 턴을 쏜 유저를 설정하는 함수
 * @param state 설정할 턴 상태 (true 또는 false)
 */
void Bot::SetWhoShot(bool state)
{
	mbWhoShot = state;
}

/**
 * @brief
 * 게임에서 총을 쏜 결과를 처리하는 함수
 * @param state "me" 또는 "other"로 자신에게 또는 상대방에게 쏜 결과를 처리
 * @return 총을 쏜 결과 메시지
 */
const std::string Bot::GameShot(const std::string& state)
{
	std::string response;
	if (mbWhoShot == false) // 첫 번째 플레이어 턴
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
				response = response + " \033[3m\033[31m실탄이였습니다\033[0m HP가 1 감소하고 턴이 넘어갑니다.";
				mFirstUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
		else if (state == "other")
		{
			response = "[" + mFirstUser->GetNickName() + "]님이 [" + mSecondUser->GetNickName() + "]님을 쐈습니다.";
			if (mbAmmoChamber.top() == false)
			{
				response = response + " \033[3m\033[32m공포탄이였습니다\033[0m 아무런 이득 없이 턴이 넘어갑니다.";
				mbWhoShot = !mbWhoShot;
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " \033[3m\033[31m실탄이였습니다\033[0m HP가 1 감소하고 턴이 넘어갑니다.";
				mSecondUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
	}
	else if (mbWhoShot == true) // 두 번째 플레이어 턴
	{
		if (state == "me")
		{
			response = "[" + mSecondUser->GetNickName() + "]님이 자기 자신을 쐈습니다.";
			if (mbAmmoChamber.top() == false)
			{
				response = response + " \033[3m\033[32m공포탄이였습니다\033[0m 한번 더 턴을 가져갑니다.";
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " \033[3m\033[31m실탄이였습니다\033[0m HP가 1 감소하고 턴이 넘어갑니다.";
				mSecondUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
		else if (state == "other")
		{
			response = "[" + mSecondUser->GetNickName() + "]님이 [" + mFirstUser->GetNickName() + "]님을 쐈습니다.";
			if (mbAmmoChamber.top() == false)
			{
				response = response + " \033[3m\033[32m공포탄이였습니다\033[0m 아무런 이득 없이 턴이 넘어갑니다.";
				mbWhoShot = !mbWhoShot;
			}
			else if (mbAmmoChamber.top() == true)
			{
				response = response + " \033[3m\033[31m실탄이였습니다\033[0m HP가 1 감소하고 턴이 넘어갑니다.";
				mFirstUserHp--;
				mbWhoShot = !mbWhoShot;
			}
		}
	}
	mbAmmoChamber.pop(); // 발사된 총알을 제거
	return (response);
}

/**
 * @brief
 * 남은 총알 개수를 반환하는 함수
 * @return 남은 총알 개수
 */
int Bot::AmmoCount()
{
	return (mbAmmoChamber.size());
}
