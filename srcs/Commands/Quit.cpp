#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	-RESPONSE LIST-
	없음
 */

/**
 * @brief
 * /QUIT 명령어를 처리하는 함수
 *
 * 1. 사용자가 속한 모든 채널에서 사용자를 제거하고, 채널에서 해당 사용자의 정보 및 권한을 삭제.
 * 2. 사용자가 마지막으로 남은 채널인 경우, 해당 채널을 서버에서 제거하고 삭제.
 * 3. 사용자가 여전히 다른 채널에 속해 있으면, 모든 채널에 퇴장 메시지를 전송.
 * 4. 서버에서 사용자 정보를 삭제하고, 연결을 종료.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /QUIT 명령어에 대한 파라미터 리스트
 */
void Command::Quit(int fd, std::vector<std::string> commandVec)
{
	// 사용자 객체를 서버에서 찾음
	class User* user = mServer.FindUser(fd);

	// 사용자가 속한 모든 채널 목록을 가져옴
	std::vector<std::string> channelList = user->GetChannelList();
	std::vector<std::string>::iterator channelIt = channelList.begin();

	// 사용자가 속한 모든 채널에서 퇴장 처리
	for (; channelIt != channelList.end(); channelIt++)
	{
		// 해당 채널을 서버에서 찾음
		Channel* channel = mServer.FindChannel(*channelIt);

		// 채널이 없으면 계속 진행
		if (!channel)
			continue;

		// 채널에서 사용자를 제거
		channel->RemoveUserFdList(fd);
		channel->RemoveOperatorFd(fd);

		// 채널에 남은 사용자가 1명이라면 해당 채널을 서버에서 제거하고 삭제
		if (channel->GetUserFdList().size() == 1)
		{
			mServer.RemoveChannel(channel->GetChannelName());
			delete channel;
		}
		else // 사용자가 남아있는 경우, 퇴장 메시지를 모든 채널 사용자에게 전송
			MsgToAllChannel(fd, channel->GetChannelName(), "QUIT", ChannelMessage(1, commandVec));
	}

	// 서버에서 사용자 정보를 삭제하고 연결 종료
	mServer.DeleteUserFromServer(fd);
}
