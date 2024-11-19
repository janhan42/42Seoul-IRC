#include "../Command.hpp"
#include "../Server.hpp"
#include "../Utils/Split.hpp"
#include "../User.hpp"
#include "../Bot.hpp"

/*
	- RESPONSE LIST-
	ERR_NEEDMOREPARAMS (461)
	ERR_NOSUCHCHANNEL (403)
	ERR_TOOMANYCHANNELS (405) -> 서버에서 최대 채널 수 제한을 안해뒀기 때문에 비사용
	ERR_BADCHANNELKEY (475)
	ERR_BANNEDFROMCHAN (474) -> b 모드 지원안하기 때문에 비사용
	ERR_CHANNELISFULL (471)
	ERR_INVITEONLYCHAN (473)
	ERR_BADCHANMASK (476) -> ERR_NOSUCHCHANNEL로 대체
	RPL_TOPIC (332)
	RPL_TOPICWHOTIME (333)
	RPL_NAMREPLY (353)
	RPL_ENDOF (366)
 */

/**
 * @brief
 * 사용자가 여러 채널에 참여하는 함수
 *
 * 1. 클라이언트가 /JOIN 명령어를 보내면, 채널 이름과 키를 처리하여 채널에 참여시킵니다.
 * 2. 채널에 이미 존재하는 경우, 초대 모드, 키 모드, 제한된 채널 조건 등을 확인하여 참여 가능 여부를 검사합니다.
 * 3. 채널이 존재하지 않으면 새 채널을 생성하고, 채널에 사용자를 추가합니다.
 *
 * @param fd 사용자 파일 디스크립터
 * @param commandVec IRC 명령어와 파라미터가 담긴 벡터
 */
void Command::Join(int fd, std::vector<std::string> commandVec)
{
	/* JOIN <channel>{,<channel>} [<key>{,<key>}] */
	/*
		commandVec[0] = JOIN
		commandVec[1] = <channel>{,<channel>}
		commnadVec[2] = [<key>{,key>}]
	*/

	// 파라미터가 부족한 경우
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*mServer.FindUser(fd), commandVec[1]);  // 파라미터 부족에 대한 에러 응답
		return;
	}

	// 채널 이름을 ','로 구분하여 파싱
	std::vector<std::string> joinChannel = split(commandVec[1], ',');
	std::vector<std::string>::iterator iter = joinChannel.begin();
	std::vector<std::string> joinKey;
	std::vector<std::string>::iterator keyIt;

	// 키가 있는 경우 파싱
	if (commandVec.size() > 2)
	{
		joinKey = split(commandVec[2], ',');
		keyIt = joinKey.begin();
	}

	// 사용자 객체를 파일 디스크립터로부터 찾음
	class User* user = mServer.FindUser(fd);

	// 각 채널에 대해 처리
	while(iter != joinChannel.end())
	{
		// 채널 이름이 '#' 또는 '&'로 시작하지 않으면 잘못된 채널 이름
		if ((*iter)[0] != '#' && (*iter)[0] != '&')
		{
			mResponse.ErrorNosuchChannel403(*user, *iter);  // 채널 없음 에러
			iter++;
			// 키가 있으면 다음 키로 이동
			if (commandVec.size() > 2 || keyIt != joinKey.end())
				keyIt++;
			continue;
		}

		// 채널이 존재하는지 확인
		Channel* targetChannel = mServer.FindChannel(*iter);
		if (targetChannel != NULL) // 채널이 이미 존재하는 경우
		{
			/* 조건: 사용자가 채널에 참여할 수 있는지 */
			Channel* channel = targetChannel;

			// 이미 채널에 존재하면 넘어감
			if (channel->CheckUserInChannel(fd))
			{
				iter++;
				if (commandVec.size() > 2 || keyIt != joinKey.end())
					keyIt++;
				continue;
			}

			// 초대 모드인 경우
			if (channel->CheckMode(INVITE) == true)
			{
				if (channel->CheckInvite(fd) == false) // 초대받지 않은 경우
				{
					mResponse.ErrorInviteOnlychan473(*user, *iter);  // 초대받지 않음 에러
					iter++;
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}

			// 키 모드인 경우
			if (channel->CheckMode(KEY) == true)
			{
				if (commandVec.size() <= 2 || keyIt == joinKey.end() || channel->CheckKey(*keyIt) == false) // 잘못된 키
				{
					mResponse.ErrorBadChannelKey475(*user, *iter);  // 잘못된 채널 키 에러
					iter++;
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}

			// 제한된 채널에 사용자가 많으면
			if (channel->CheckMode(LIMIT) == true)
			{
				if (channel->GetUserFdList().size() >= channel->GetLimit()) // 채널이 꽉 찼을 경우
				{
					mResponse.ErrorChannelIsFull471(*user, *iter);  // 채널이 꽉 찼다는 에러
					iter++;
					if (commandVec.size() > 2 || keyIt != joinKey.end())
						keyIt++;
					continue;
				}
			}

			// 채널에 사용자 추가
			std::string channelName = *iter;
			targetChannel->AppendUserFdList(fd);
			user->AppendChannelList(channelName);  // 사용자 채널 리스트에 추가

			// 다른 사용자들에게 채널 참여 메시지 전송
			MsgToAllChannel(fd, channelName, "JOIN", "");
			TopicMsg(fd, channelName);  // 채널의 주제 메시지 전송
		}
		else // 채널이 존재하지 않으면 (새로운 채널 생성)
		{
			mServer.AppendNewChannel(*iter, fd);  // 새 채널 생성
			Channel* newChannel = mServer.FindChannel(*iter);

			// 새 채널에 사용자 추가
			newChannel->AppendUserFdList(-1);  // 가상의 봇 사용자 추가
			newChannel->AppendUserFdList(fd);  // 실제 사용자 추가
			user->AppendChannelList(*iter);

			// 다른 사용자들에게 채널 참여 메시지 전송
			MsgToAllChannel(fd, *iter, "JOIN", "");

			// 새 채널의 오퍼레이터 설정
			newChannel->AddOperatorFd(fd);
		}

		// 채널의 사용자 목록 요청
		NameListMsg(fd, *iter);  // 353 RPL_NAMREPLAY, 366 RPL_ENDOFNAMES
		// 봇이 새 채널에 참여했을 때 봇 소개 메시지 전송
		MsgToAllChannel(-1, *iter, "PRIVMSG", mServer.FindChannel(*iter)->GetBot()->Introduce());

		iter++;  // 다음 채널로 이동
		if (commandVec.size() > 2 || keyIt != joinKey.end())
			keyIt++;
	}
}

/**
 * @brief
 * 사용자가 특정 채널에 대해 주제를 요청할 때 채널의 주제를 전송하는 함수
 *
 * 1. 클라이언트가 채널 주제를 요청하면 해당 채널의 주제를 확인하여 클라이언트에게 전달합니다.
 * 2. 채널에 주제가 없으면 주제 메시지를 보내지 않습니다.
 * 3. 채널에 주제가 있으면, 주제를 클라이언트에게 전송합니다.
 *
 * @param fd 사용자 파일 디스크립터
 * @param channelName 채널 이름
 */
void Command::TopicMsg(int fd, std::string channelName)
{
	// 채널 객체를 이름으로 찾음
	Channel* channel = mServer.FindChannel(channelName);
	std::string topic = channel->GetTopic();

	// 채널에 주제가 없으면 종료
	if (topic.length() == 0)
		return;

	// 주제에서 앞의 공백을 제거
	topic = topic.substr(1, topic.length() - 1);

	// 사용자 객체를 파일 디스크립터로부터 찾음
	class User* user = mServer.FindUser(fd);

	// 사용자에게 주제 메시지 전송
	user->AppendUserSendBuf("332 " + user->GetNickName() + " " + channelName + " :" + topic + "\r\n");
}
