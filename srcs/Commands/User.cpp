#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/**
 * @brief
 * 사용자가 /USER 명령어를 보냈을 때 처리하는 함수
 *
 * 1. 사용자가 이미 등록된 경우, 등록된 에러 메시지를 전송.
 * 2. 사용자가 비밀번호 등록을 하지 않은 경우, 등록되지 않은 사용자 에러 메시지 전송 후, 사용자 삭제.
 * 3. /USER 명령어에 필요한 파라미터가 부족하거나 realname이 잘못된 경우 에러 메시지 전송.
 * 4. 명령어가 유효하면 사용자 정보를 설정하고, 사용자 등록을 완료.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /USER 명령어에 대한 파라미터 리스트
 */
void Command::User(int fd, std::vector<std::string> commandVec)
{
	// 사용자 객체를 서버에서 찾음
	class User* user = mServer.FindUser(fd);

	// 이미 등록된 사용자라면 462 에러 전송
	if (user->GetUserRegist())
	{
		mResponse.ErrorAlreadyRegistRed462(*user);
		return ;
	}

	// 비밀번호가 등록되지 않은 사용자라면 451 에러 전송 후, 사용자 삭제
	if (!user->GetPassRegist())
	{
		mResponse.ErrorNotRegistered451(*user);
		send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
		mServer.DeleteUserFromServer(fd);
		return;
	}

	// /USER 명령어에 필요한 파라미터가 부족하거나 realname이 잘못되었을 경우 처리
	if (commandVec.size() < 5 || !CheckRealName(commandVec[4]))
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		user->AppendUserSendBuf("/USER <username> <hostname> <servername> <:realname>\r\n");
		return ;
	}

	// realname을 파라미터에서 추출하여 설정
	std::string realname;
	for (size_t i = 4; i < commandVec.size(); i++)
	{
		realname += commandVec[i];
		if (i != commandVec.size() - 1)
			realname += " ";
	}

	// 사용자 정보를 설정하고 등록 완료
	user->SetUser(commandVec[1], commandVec[2], commandVec[3], realname);
	user->SetUserRegist(true);
}

/**
 * @brief
 * realname이 유효한지 확인하는 함수
 *
 * realname은 반드시 ':'로 시작해야 유효한 값으로 판단함.
 *
 * @param realName 검증할 realname 문자열
 * @return bool realname이 유효하면 true, 그렇지 않으면 false
 */
bool Command::CheckRealName(std::string realName)
{
	// realname이 비어 있으면 유효하지 않음
	if (realName.length() == 0)
		return (false);

	// realname이 ':'로 시작해야 유효함
	if (realName[0] != ':')
		return (false);

	// 유효한 realname인 경우 true 반환
	return (true);
}
