#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461) o
	ERR_NOSUCHCHANNEL (403) o
	ERR_NOTONCHANNEL (442) o
	ERR_CHANOPRIVSNEEDED (482) o
	RPL_NOTOPIC (331) o
	RPL_TOPIC (332) o
	RPL_TOPICWHOTIME (333) irssi 쪽에서 관리함
 */

/**
 * @brief
 * /TOPIC 명령어를 처리하는 함수
 *
 * 1. 사용자가 입력한 /TOPIC 명령어의 매개변수를 확인하고, 부족한 경우 에러 메시지 전송.
 * 2. 사용자가 지정한 채널이 존재하지 않으면 403 에러 메시지 전송.
 * 3. 사용자가 해당 채널에 속하지 않으면 442 에러 메시지 전송.
 * 4. 채널이 `+t` 모드인 경우, 사용자가 채널 운영자가 아니라면 482 에러 메시지 전송.
 * 5. 두 번째 매개변수가 없으면 현재 채널의 토픽을 반환하고, 토픽이 없다면 331 에러 메시지 전송.
 * 6. 토픽을 설정하려면, 주어진 메시지를 채널의 토픽으로 설정하고, 변경 사항을 모든 채널 사용자에게 전달.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /TOPIC 명령어에 대한 파라미터 리스트
 */
void Command::Topic(int fd, std::vector<std::string> commandVec)
{
	// 사용자 객체를 서버에서 찾음
	class User& user = *mServer.FindUser(fd);

	// /TOPIC 명령어에 필요한 채널 파라미터가 부족하면 461 에러 전송
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(user, commandVec[1]);
		return;
	}

	// 채널을 서버에서 찾음
	Channel* channel = mServer.FindChannel(commandVec[1]);
	// 채널이 없으면 403 에러 전송
	if (channel == NULL)
	{
		mResponse.ErrorNosuchChannel403(user, commandVec[1]);
		return;
	}

	// 사용자가 해당 채널에 존재하는지 확인
	if (user.IsInChannel(commandVec[1]) == false)
	{
		mResponse.ErrorNotOnChannel442(user, commandVec[1]);
		return;
	}

	// 채널 모드가 +t인 경우(토픽 변경 제한 모드), 사용자가 채널 운영자가 아닌 경우 482 에러 전송
	if (channel->CheckMode(TOPIC)) // Channel Mode +t
	{
		if (channel->CheckOperator(fd) == false)
		{
			mResponse.ErrorChanOprivsNeeded482(user, commandVec[1]);
			return;
		}
	}

	// /TOPIC <channel> 명령어가 토픽 조회라면
	if (commandVec.size() == 2)
	{
		// 채널에 토픽이 없다면 331 에러 메시지 전송
		if (channel->GetTopic().length() == 0)
		{
			mResponse.ErrorNoTopic331(user, commandVec[1]);
			return;
		}
		// 토픽이 있다면 사용자에게 채널의 토픽을 전송
		user.AppendUserSendBuf("332 " + user.GetNickName() + " " + commandVec[1] + " :" + channel->GetTopic() + "\r\n");
	}
	// 토픽을 설정하는 경우
	else // SetTopic
	{
		// 토픽이 ":"만 있다면 빈 토픽으로 설정
		if (commandVec[2] == ":")
			channel->SetTopic("");
		else
		{
			// 토픽 메시지가 ":" 이후에 있는 경우, 전체 토픽을 설정
			std::string topic = commandVec[2];
			for (size_t i = 3; i < commandVec.size(); i++)
			{
				topic += " " + commandVec[i];
			}
			channel->SetTopic(topic);
		}
		// 토픽이 변경되었음을 모든 채널 사용자에게 전송
		MsgToAllChannel(fd, commandVec[1], "TOPIC", channel->GetTopic());
	}
}
