#include <sstream>
#include <iostream>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST-
	ERR_NEEDMOREPARAMS (461)
	ERR_NOSUCHCHANNEL (403)
	ERR_NOTONCHANNEL (442)
 */

/**
 * @brief
 * /PART 명령어를 처리하는 함수
 *
 * 1. 사용자가 지정한 채널에서 퇴장할 때 호출됩니다.
 * 2. 채널 이름이 제공되지 않으면 에러 메시지를 전송합니다.
 * 3. 채널 이름이 여러 개인 경우 `,`로 구분하여 처리합니다.
 * 4. 사용자가 해당 채널에 있을 경우, 해당 채널에서 퇴장 메시지를 전송하고, 사용자를 채널에서 제거합니다.
 * 5. 채널에 남은 사용자가 1명 이하일 경우, 채널을 삭제합니다.
 * 6. 사용자가 해당 채널에 없다면, 채널에 없음 또는 채널이 존재하지 않음을 알리는 에러 메시지를 전송합니다.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /PART 명령어에 대한 파라미터 리스트
 */
void Command::Part(int fd, std::vector<std::string> commandVec)
{
	// 사용자 객체를 서버에서 찾음
	class User* user = mServer.FindUser(fd);

	// 채널 이름이 제공되지 않으면 에러 메시지 전송
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		return;
	}

	// 채널 이름이 여러 개일 수 있으므로 ','로 구분된 채널 이름을 벡터에 저장
	std::istringstream iss(commandVec[1]);
	std::string buffer;
	std::vector<std::string> vec;
	while (getline(iss, buffer, ',')) // 채널 이름을 ','로 분리
		vec.push_back(buffer);

	// 각 채널에 대해 처리
	std::vector<std::string>::iterator vecIt = vec.begin();
	for (; vecIt != vec.end(); vecIt++)
	{
		// 사용자가 해당 채널에 있을 경우
		if (user->IsInChannel(*vecIt) == true)
		{
			Channel* channel = mServer.FindChannel(*vecIt); // 채널 찾기
			// 채널에 사용자 퇴장 메시지 전송
			MsgToAllChannel(fd, *vecIt, "PART", ChannelMessage(2, commandVec));

			// 사용자 퇴장 처리
			channel->RemoveUserFdList(fd);
			channel->RemoveOperatorFd(fd);

			// 채널에 남은 사용자가 1명 이하라면 채널 삭제
			if (channel->GetUserFdList().size() <= 1)
			{
				std::cout << "채널에 fd 갯수 : " << channel->GetUserFdList().size() << std::endl;
				mServer.RemoveChannel(channel->GetChannelName()); // 채널 삭제
				delete channel; // 채널 객체 메모리 해제
			}
		}
		else // 사용자가 해당 채널에 없다면 에러 처리
		{
			if (mServer.FindChannel(*vecIt))
			{
				mResponse.ErrorNotOnChannel442(*user, *vecIt); // 채널에 없음
			}
			else
			{
				mResponse.ErrorNosuchChannel403(*user, *vecIt); // 채널이 존재하지 않음
			}
		}
	}
}
