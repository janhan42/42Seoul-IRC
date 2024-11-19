#include "../Command.hpp"
#include "../User.hpp"
#include <iostream>

/**
 * @brief
 * 사용자가 채널 모드를 변경하거나 채널 모드를 조회하는 함수
 *
 * 1. 채널의 모드를 조회하거나, 사용자에 의해 채널 모드를 설정하는 역할을 합니다.
 * 2. 채널 모드는 `+` 또는 `-` 부호로 설정하거나 해제할 수 있으며, 특정 모드에는 추가 인자가 필요합니다.
 * 3. 사용자가 권한이 없거나 파라미터가 부족한 경우, 에러 메시지를 응답합니다.
 *
 * @param fd 사용자 파일 디스크립터
 * @param commandVec IRC 명령어와 파라미터가 담긴 벡터
 */
void Command::Mode(int fd, std::vector<std::string> commandVec)
{
	/* MODE <channel> <+/- i, t, k, l, o> (<mode-needed-value>) */

	// 사용자 객체를 파일 디스크립터로부터 찾음
	class User* user = mServer.FindUser(fd);

	// 파라미터가 부족한 경우
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);  // 부족한 파라미터에 대한 에러 응답
		return;
	}

	// 채널을 이름으로 찾음
	Channel* channel = mServer.FindChannel(commandVec[1]);

	// 사용자 닉네임과 채널이 다르거나 채널이 없으면 채널 없음 에러
	if (user->GetNickName() != commandVec[1] && channel == NULL)
	{
		mResponse.ErrorNosuchChannel403(*user, commandVec[1]);  // 채널 없음 에러
		return;
	}

	// 채널이 존재하고, MODE 명령어가 채널 모드 조회일 경우
	if (channel != NULL && commandVec.size() == 2)
	{
		user->AppendUserSendBuf("324 " + user->GetNickName() + " " + commandVec[1] + " +" + channel->GetMode() + "\r\n"); // 채널 모드 출력
		return;
	}

	// 사용자 닉네임과 채널명이 같고, "+i" 모드를 설정하려는 경우
	if (user->GetNickName() == commandVec[1] && commandVec[2] == "+i")
	{
		user->AppendUserSendBuf("324 " + user->GetNickName() +  " " + commandVec[1] + " +i" + "\r\n"); // "+i" 모드 출력
		return;
	}

	// "b" 모드는 처리하지 않음
	if (commandVec[2] == "b")
		return;

	// 채널에 대한 사용자 권한이 없는 경우
	if (channel != NULL && !channel->CheckOperator(fd))
	{
		mResponse.ErrorChanOprivsNeeded482(*user, commandVec[1]);  // 채널 권한 부족 에러
		return;
	}

	// 모드 파라미터를 처리하기 위한 변수들
	std::string mode = commandVec[2];
	char sign;
	int index = 0;

	// '+' 또는 '-' 부호를 찾아 모드를 설정
	while (mode[index] == '+' || mode[index] == '-')
	{
		sign = mode[index];
		index++;
	}

	// 모드 길이가 1이면 아무 작업도 하지 않음
	if (mode.length() == 1)
		return;

	// 모드 관련 메시지와 인자들을 저장할 리스트
	std::string msg = "";
	std::vector<std::string> modeArgList;
	unsigned int modeArgIndex = 3;

	// 모드 문자열을 순차적으로 처리
	for (size_t i = index; i < mode.length(); i++)
	{
		bool isSetMode = false;

		// 초대 모드 처리
		if (mode[i] == 'i')
		{
			if (sign == '+' && channel->CheckMode(INVITE))
				continue;
			if (sign == '-' && !channel->CheckMode(INVITE))
				continue;
			channel->SetMode(INVITE, sign);
			isSetMode = true;
		}
		// 토픽 모드 처리
		else if (mode[i] == 't')
		{
			if (sign == '+' && channel->CheckMode(TOPIC))
				continue;
			if (sign == '-' && !channel->CheckMode(TOPIC))
				continue;
			channel->SetMode(TOPIC, sign);
			isSetMode = true;
		}
		// 키 모드 처리
		else if (mode[i] == 'k')
		{
			if (sign == '-' && !channel->CheckMode(KEY))
				continue;
			if (commandVec.size() > modeArgIndex)
			{
				channel->SetMode(KEY, sign);
				if (sign == '+')
				{
					channel->SetKey(commandVec[3]);  // 키 설정
				}
				else if (sign == '-')
				{
					channel->SetKey("");  // 키 해제
				}
				isSetMode = true;
				modeArgList.push_back(commandVec[modeArgIndex]);  // 모드 인자 저장
				modeArgIndex++;
			}
		}
		// 제한 모드 처리
		else if (mode[i] == 'l')
		{
			if (sign == '-' && !channel->CheckMode(LIMIT))
				continue;
			if (commandVec.size() > modeArgIndex)
			{
				std::string limit_s = commandVec[modeArgIndex].c_str();
				bool isDigit = true;

				// 제한 인자가 숫자인지 확인
				for (size_t j = 0; j < limit_s.length(); ++j)
				{
					if (!isdigit(limit_s[j]))
					{
						isDigit = false;
						break;
					}
				}

				// 숫자가 아닌 경우 건너뜀
				if (!isDigit)
				{
					modeArgIndex++;
					continue;
				}

				int limit = atoi(limit_s.c_str());
				if (limit < 0)
				{
					modeArgIndex++;
					continue;
				}

				channel->SetMode(LIMIT, sign);
				if (sign == '+')
				{
					channel->SetLimit(limit);  // 제한 인자 설정
				}
				isSetMode = true;
				modeArgList.push_back(commandVec[modeArgIndex]);  // 제한 인자 저장
				modeArgIndex++;
			}
		}
		// 운영자 모드 처리
		else if (mode[i] == 'o')
		{
			if (commandVec.size() <= modeArgIndex)
			{
				continue;
			}

			// 타겟 사용자 찾기
			class User* target = mServer.FindUser(commandVec[modeArgIndex]);
			if (target == NULL)	// 사용자가 존재하지 않으면
			{
				mResponse.ErrorNosuchNick401(*user, commandVec[modeArgIndex]);  // 사용자 없음
				return;
			}
			else
			{
				if (user->GetNickName() == target->GetNickName())  // 자신이 아니면
				{
					return;
				}
				if (!channel->CheckUserInChannel(target->GetUserFd()))	// 사용자가 채널에 없으면
				{
					mResponse.ErrorUserNotInChannel441(*user, commandVec[modeArgIndex], commandVec[1]);  // 사용자 채널 없음
					return;
				}
				else if (sign == '+') // 운영자 권한 부여
				{
					channel->AddOperatorFd(target->GetUserFd());
					isSetMode = true;
					modeArgList.push_back(commandVec[modeArgIndex]);  // 운영자 부여 인자 저장
					modeArgIndex++;
				}
				else if (sign == '-') // 운영자 권한 제거
				{
					channel->RemoveOperatorFd(target->GetUserFd());
					isSetMode = true;
					modeArgList.push_back(commandVec[modeArgIndex]);  // 운영자 제거 인자 저장
					modeArgIndex++;
				}
			}
		}

		// 모드가 설정되었으면, 해당 모드 정보를 메시지에 추가
		if (isSetMode)
		{
			if (msg.empty())
				msg += sign;
			msg += mode[i];
		}
	}

	// 모드 인자들을 메시지에 추가
	for (size_t i = 0; i < modeArgList.size(); ++i)
	{
		msg += " " + modeArgList[i];
	}

	// 모드 메시지가 비어 있지 않으면, 모든 채널에 모드 변경 메시지 전송
	if (!msg.empty())
	{
		MsgToAllChannel(fd, commandVec[1], "MODE", msg);
	}
}
