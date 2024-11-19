#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"
#include "../Define.hpp"
#include "../Utils/IsSpecial.hpp"
#include <iostream>

/*
	- RESPONSE LIST -
	ERR_NONICKNAMEGIVEN (431) o
	ERR_ERRONEUSNICKNAME (432) o
	ERR_NICKNAMEINUSE (433) o
	ERR_NICKCOLLISION (436) (다른 서버에 대한 처리라서 안함)
 */

/**
 * @brief
 * /NICK 명령어를 처리하는 함수
 *
 * 1. 사용자가 자신의 닉네임을 설정하거나 변경할 때 호출됩니다.
 * 2. 패스워드 인증이 완료되지 않은 경우, 인증 에러를 처리하고 사용자를 서버에서 삭제합니다.
 * 3. 닉네임이 비어 있으면 에러 메시지를 보내고, 닉네임이 유효하지 않거나 중복되면 각각의 에러를 처리합니다.
 * 4. 채널에 참여한 경우, 변경된 닉네임을 해당 채널에 전파합니다.
 * 5. 성공적으로 닉네임을 변경한 후, 사용자에게 변경된 닉네임을 전송합니다.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /NICK 명령어에 대한 파라미터 리스트
 */
void Command::Nick(int fd, std::vector<std::string> commandVec)
{
	// 사용자 객체를 서버에서 찾음
	class User* user = mServer.FindUser(fd);

	// 패스워드 인증이 완료되지 않은 경우, 인증 에러 처리
	if (!user->GetPassRegist())
	{
		mResponse.ErrorNotRegistered451(*user);
		send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
		mServer.DeleteUserFromServer(fd);
		return ;
	}

	// 닉네임이 비어 있으면, 닉네임 제공 에러 처리
	if (commandVec[1].empty())
	{
		mResponse.ErrorNoNickNameGiven431(*user);
		return ;
	}

	// 닉네임 유효성 체크
	if (!CheckNickNameValid(commandVec[1]))
	{
		mResponse.ErrorErronusNickName432(*user, commandVec[1]);
		user->AppendUserSendBuf("/NICK <nickname> First Letter is not digit and length is under 10.\r\n");
		return ;
	}

	// 닉네임 중복 체크
	if (!CheckNickNameDuplicate(commandVec[1], mServer.GetUserList()))
	{
		mResponse.ErrorNickNameInuse433(*user, commandVec[1]);
		return ;
	}

	// 기존 닉네임 저장
	std::string oldNickName = user->GetNickName();
	if (oldNickName == "User") // 기본 값 "User"일 경우, 기존 닉네임을 다시 저장
		oldNickName = user->GetNickName();

	// 사용자가 속한 채널들에 대해 NICK 변경 메시지 전송
	std::vector<std::string> channelList = user->GetChannelList();
	std::vector<std::string>::iterator channelIt = channelList.begin();
	for (; channelIt != channelList.end(); channelIt++)
	{
		Channel* channel = mServer.FindChannel(*channelIt);
		if (channel)
			NickMsgToAllChannel(fd, channel->GetChannelName(),  oldNickName, commandVec[1]);
	}

	// 새로운 닉네임 설정
	user->SetNickName(commandVec[1]);
	// 닉네임 변경 정보를 사용자에게 전송
	user->AppendUserSendBuf(":" + oldNickName + " NICK " + user->GetNickName() + "\r\n");
	user->SetNickRegist(true);
}

/**
 * @brief
 * 닉네임이 유효한지 체크하는 함수
 *
 * 1. 닉네임의 길이가 1~9자 사이인지, 첫 문자가 숫자가 아닌지 확인합니다.
 * 2. 나머지 문자는 알파벳, 숫자 및 특수 문자인지 확인합니다.
 * 3. 유효한 닉네임이면 true를 반환하고, 그렇지 않으면 false를 반환합니다.
 *
 * @param nickName 확인할 닉네임
 * @return 유효하면 true, 아니면 false
 */
bool Command::CheckNickNameValid(std::string nickName)
{
	// 닉네임 길이가 0이거나 10 이상이면 유효하지 않음
	if (nickName.length() == 0 || nickName.length() > 9)
		return (false);

	// 첫 문자가 숫자이면 유효하지 않음
	if ('0' <= nickName[0] && nickName[0] <= '9')
		return (false);

	// 두 번째 문자부터는 알파벳, 숫자, 특수 문자가 아니면 유효하지 않음
	for (size_t i = 1; i < nickName.length(); i++)
	{
		if (!isalnum(nickName[i]) && !isSpecial(nickName[i]))
			return (false);
	}
	return (true);
}

/**
 * @brief
 * 닉네임이 이미 사용 중인지 체크하는 함수
 *
 * 1. 서버에 존재하는 사용자들의 닉네임과 비교하여 중복된 닉네임이 있으면 false를 반환합니다.
 * 2. 닉네임 길이가 다르면 중복되지 않음을 확인합니다.
 * 3. 대소문자 구분 없이 닉네임이 같은지 비교하여 중복 여부를 판단합니다.
 *
 * @param nickName 확인할 닉네임
 * @param userList 서버에 존재하는 사용자 목록
 * @return 중복되지 않으면 true, 중복되면 false
 */
bool Command::CheckNickNameDuplicate(std::string nickName, UserMap& userList)
{
	UserMap::iterator it = userList.begin();
	for (; it != userList.end(); it++)
	{
		std::string userNickName = it->second->GetNickName();
		bool isSame = true;

		// 닉네임 길이가 다르면 중복되지 않음
		if (userNickName.length() != nickName.length())
			continue;

		// 대소문자 구분 없이 닉네임이 같은지 체크
		for (size_t i = 0; i < nickName.length(); ++i)
		{
			if (std::toupper(nickName[i]) != std::toupper(userNickName[i]))
			{
				isSame = false;
				break;
			}
		}

		// 중복된 닉네임이 있다면 false 반환
		if (isSame)
			return (false);
	}
	// 중복되지 않으면 true 반환
	return (true);
}
