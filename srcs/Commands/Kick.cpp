#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include <sstream>
#include <iostream>
#include <string>

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461)
	ERR_NOSUCHCHANNEL (403)
	ERR_CHANOPRIVSNEEDED (482)
	ERR_USERNOTINCHANNEL (441)
	ERR_NOTONCHANNEL (442)
 */
/**
 * @brief
 * 사용자를 채널에서 강퇴하는 함수
 *
 * 1. 클라이언트가 /KICK 명령어를 보내면, 해당 채널에서 사용자를 강퇴합니다.
 * 2. 사용자가 해당 채널에 없거나, 권한이 부족한 경우 에러 메시지를 전송합니다.
 * 3. 사용자가 채널에 있으면, 강퇴 메시지를 보내고 사용자를 채널에서 제거합니다.
 *
 * @param fd 사용자 파일 디스크립터
 * @param commandVec IRC 명령어와 파라미터가 담긴 벡터
 */
void Command::Kick(int fd, std::vector<std::string> commandVec)
{
	/* KICK <channel> <nickname> */

	// 사용자 객체를 파일 디스크립터로부터 찾음
	class User* user = mServer.FindUser(fd);

	// 파라미터가 부족한 경우
	if (commandVec.size() < 3)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);  // 부족한 파라미터에 대한 에러 응답
		return;
	}

	// 사용자가 채널에 없으면
	if (user->IsInChannel(commandVec[1]) == false)
	{
		mResponse.ErrorNotOnChannel442(*user, commandVec[1]);  // 채널에 없음 에러
		return;
	}

	// 대상 사용자 목록을 ','로 구분하여 파싱
	std::istringstream iss(commandVec[1]);
	std::string buffer;
	std::vector<std::string> vec;
	while(getline(iss, buffer, ',')) // TargetUser
		vec.push_back(buffer);
	std::vector<std::string>::iterator vecIt = vec.begin();

	// 채널을 찾고, 채널의 오퍼레이터가 아닌 경우
	Channel* channel = mServer.FindChannel(*vecIt);
	if (channel && channel->CheckOperator(fd) == false)
	{
		mResponse.ErrorChanOprivsNeeded482(*user, *vecIt);  // 채널 오퍼레이터 권한 부족
		return;
	}

	// 대상 사용자 리스트에 대해 반복하여 처리
	for (; vecIt != vec.end(); vecIt++)
	{
		Channel* channel = mServer.FindChannel(*vecIt);
		std::cout << "Find Channel : " << *vecIt << std::endl; // TEST OUTPUT

		// 채널이 존재하지 않으면
		if (channel == NULL)
		{
			mResponse.ErrorNosuchChannel403(*user, *vecIt);  // 채널 없음 에러
		}
		else
		{
			// 강퇴할 사용자 찾기
			class User* target = mServer.FindUser(commandVec[2]);
			if (target == NULL) // 사용자가 존재하지 않으면
			{
				mResponse.ErrorUserNotInChannel441(*user, commandVec[2], *vecIt);  // 사용자가 채널에 없음
				return;
			}

			// 봇인 경우 강퇴하지 않음
			if (target->GetUserFd() == -1)
				return;
			else
			{
				// 사용자가 채널에 없다면
				if (!channel->CheckUserInChannel(target->GetUserFd()))
				{
					mResponse.ErrorUserNotInChannel441(*user, commandVec[2], *vecIt);  // 사용자가 채널에 없음
				}
				else // 채널에서 사용자 강퇴
				{
					// 강퇴 메시지 설정
					std::string message = commandVec[2];
					if (commandVec.size() > 3)
					{
						for (size_t i = 3; i < commandVec.size(); i++)
						{
							message = message + " " + commandVec[i];
						}
					}

					// 모든 채널에 강퇴 메시지 전송
					MsgToAllChannel(fd, *vecIt, "KICK", message);

					// 사용자를 채널에서 제거
					channel->RemoveUserFdList(target->GetUserFd());
					channel->RemoveOperatorFd(target->GetUserFd()); // 오퍼레이터 권한도 제거
					target->RemoveChannel(*vecIt);

					// 채널에 더 이상 사용자가 없으면 채널 삭제
					if (channel->GetUserFdList().size() <= 1)
					{
						mServer.RemoveChannel(channel->GetChannelName());  // 채널 삭제
						delete channel;  // 채널 객체 삭제
					}
				}
			}
		}
	}
}
