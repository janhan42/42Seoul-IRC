#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include "../Bot.hpp"
#include <sstream>
#include <iostream>
#include <vector>

/*
	- RESPONSE LIST -
	ERR_NOSUCHNICK (401)
	ERR_NOSUCHSERVER (402) -> 다중서버 처리가 아니기 때문에 없음
	ERR_CANNOTSENDTOCHAN (404) -> 외부에서 메세지를 보내지 못하는 모드는 구현 안되있기 때문에 없음
	ERR_TOOMANYTARGETS (407) -> 타겟 리밋 변수가 없기 때문에 없음
	ERR_NORECIPIENT (411) -> irssi쪽에서 막아버림
	ERR_NOTEXTTOSEND (412)
	ERR_NOTOPLEVEL (413) -> 구현사항 아님
	ERR_WILDTOPLEVEL (414) -> 구현사항 아님
	RPL_AWAY (301) -> /away <message> 를 한 상대에게 /msg를 했을때 응답인데 해야하나 모르겠음
 */

/**
 * @brief
 * /PRIVMSG (또는 /msg) 명령어를 처리하는 함수
 *
 * 1. 사용자가 보낸 메시지가 채널로 향하는지, 또는 다른 사용자에게 향하는지 확인.
 * 2. 채널로 메시지를 보낼 경우, 해당 채널에 사용자가 존재하는지 확인하고, 존재하면 메시지를 전송.
 * 3. 채널이 존재하지 않으면 에러 메시지 전송.
 * 4. 사용자가 보내는 메시지가 특정 봇 명령어와 관련된 경우, 봇 명령어를 처리.
 * 5. 봇 명령어에 대한 처리는 BotCommand 함수에서 담당.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /PRIVMSG 명령어에 대한 파라미터 리스트
 */
void Command::Privmsg(int fd, std::vector<std::string> commandVec)
{
	// 사용자 객체를 서버에서 찾음
	class User* user = mServer.FindUser(fd);

	std::string buffer;
	std::vector<std::string> vec;

	// 명령어 파라미터가 부족하면 에러 발생
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		return;
	}

	// 메시지가 비어있으면 에러 발생
	if (commandVec[2].empty())
	{
		mResponse.ErrorNoTextToSend412(*user);
		return;
	}

	// 메시지의 대상이 여러 명일 수 있으므로, 쉼표(,)를 기준으로 나눠서 벡터에 저장
	std::istringstream iss(commandVec[1]);
	while (getline(iss, buffer, ','))
		vec.push_back(buffer);

	// 각 대상에 대해 메시지를 전송
	std::vector<std::string>::iterator vecIt = vec.begin();
	for(; vecIt != vec.end(); vecIt++)
	{
		if ((*vecIt)[0] == '#' || (*vecIt)[0] == '&') // 대상이 채널일 경우
		{
			Channel* channel = mServer.FindChannel(*vecIt); // 채널 찾기
			if (channel) // 채널이 존재하면
			{
				// 봇 명령어가 포함된 메시지인지 확인
				if (commandVec.size() > 2 && CheckBotCommand(commandVec[2]))
				{
					BotCommand(fd, commandVec); // 봇 명령어 처리
					return ;
				}

				// 채널에 메시지 전송
				std::string messages = ChannelMessage(2, commandVec);
				ChannelPrivmsg(messages, *user, channel);
			}
			else // 채널이 존재하지 않으면
			{
				mResponse.ErrorNosuchChannel403(*user, *vecIt);
			}
		}
		else // 대상이 채널이 아닌 사용자일 경우
		{
			if (*vecIt == SERVERNAME) // 서버 이름이 대상이면 /PING 처리
			{
				return ;
			}
			if (commandVec[2] == ":\x01PING") // 유저가 임의로 ping 보낸 경우 무시
			{
				return;
			}

			// 대상 사용자 찾기
			class User* target_user = mServer.FindUser(*vecIt);
			if (target_user != NULL)
			{
				// 사용자에게 메시지 전송
				std::string messages = ChannelMessage(2, commandVec);
				target_user->AppendUserSendBuf(":" + user->GetNickName() + " PRIVMSG " + target_user->GetNickName() + " :" + messages + "\r\n");
			}
			else // 대상 사용자가 없으면 에러 발생
			{
				mResponse.ErrorNosuchNick401(*user, *vecIt);
				return ;
			}
		}
	}
}

/**
 * @brief
 * 봇 명령어가 포함된 메시지인지 확인하는 함수
 *
 * @param command 명령어 문자열
 * @return true 봇 명령어일 경우
 * @return false 일반 명령어일 경우
 */
bool Command::CheckBotCommand(std::string command)
{
	// 봇 명령어는 ":@bot"만 처리
	if (command == ":@bot")
		return (true);
	return (false);
}

/**
 * @brief
 * 봇 명령어를 처리하는 함수
 *
 * 1. 봇에 대한 기본적인 소개 및 도움말을 반환하거나, 게임을 시작하는 등의 명령어를 처리.
 * 2. 'buckshot' 명령어가 주어지면 봇과 사용자 간의 게임을 진행.
 * 3. 봇의 상태와 관련된 명령어를 처리.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /PRIVMSG 봇 명령어에 대한 파라미터 리스트
 */
void Command::BotCommand(int fd, std::vector<std::string> commandVec)
{
	// 채널 찾기
	Channel* channel = mServer.FindChannel(commandVec[1]);
	// 채널에서 봇 객체 가져오기
	Bot* bot = channel->GetBot();

	if (commandVec.size() == 3) // PRIVMSG <channel Name> @bot
	{
		// 봇 소개 메시지를 채널에 전송
		MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", bot->Introduce());
		return;
	}

	std::string command = commandVec[3];
	if (command == "help") // 도움말 명령어 처리
	{
		bot->HelpMsgtoChannel(this, channel->GetChannelName());
	}
	else if (command == "buckshot") // buckshot 명령어 처리
	{
		// 게임 중이거나 대기 중이면 게임을 시작할 수 없다고 응답
		if (bot->GameOn() == true || bot->GetReady() == true)
		{
			std::string response = "현재 게임중 입니다. 다른 사람들은 게임을 할 수 없습니다.";
			MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}

		// 상대방 닉네임이 없는 경우 에러 메시지 전송
		if (commandVec.size() < 5)
		{
			std::string response = "상대방의 닉네임을 입력해주세요.";
			MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			mResponse.ErrorNeedMoreParams461(*mServer.FindUser(fd), commandVec[1]);
			return;
		}

		// 상대방 닉네임을 유효성 체크
		bot->ValidTarget(fd, &mServer, channel, this, commandVec);
	}
	else if (bot->GetReady() == true && *channel->FindMyUserIt(fd) == bot->GetSecondUser()->GetUserFd() && bot->GameOn() == false) // 도전자가 게임 시작 전이라면
	{
		// 게임 요청 처리
		bot->ResponseGameRequest(channel, this, commandVec);
	}
	else if (bot->GameOn() == true && (fd == bot->GetFirstUser()->GetUserFd() || fd == bot->GetSecondUser()->GetUserFd())) // 게임 중이면
	{
		// 게임 진행 명령어 처리
		if (commandVec[3] != "me" && commandVec[3] != "other")
		{
			std::string response = "게임중 가능 명령어는 me 또는 other 입니다.";
			MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", response);
			return;
		}
		// 게임 진행
		bot->GameControl(fd, channel, this, commandVec);
	}
	else
	{
		// 봇 명령어를 찾을 수 없으면 에러 메시지 전송
		MsgToAllChannel(BOT, channel->GetChannelName(), "PRIVMSG", "BOT COMMAND NOT FOUND");
	}
}
