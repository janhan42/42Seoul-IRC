#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461)
	ERR_NOORIGIN (409)
 */

/**
 * @brief
 * /PING 명령어를 처리하는 함수
 *
 * 1. PING <token> 명령어가 전달되면, 해당 token을 사용하여 PONG 응답을 보내는 기능을 수행합니다.
 * 2. 명령어에 필요한 파라미터가 부족한 경우, 에러 메시지를 전송합니다.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /PING 명령어에 대한 파라미터 리스트
 */
void Command::Ping(int fd, std::vector<std::string> commandVec)
{
	// 사용자 객체를 서버에서 찾음
	class User* user = mServer.FindUser(fd);

	// 명령어에 필요한 파라미터가 부족하면 에러 메시지 전송
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		// PING 명령어에 대한 올바른 형식 안내
		user->AppendUserSendBuf("/PING <token>\r\n");
		return ;
	}

	// PONG 응답 메시지를 전송
	user->AppendUserSendBuf("PONG " + commandVec[1] + " :" + commandVec[1] + "\r\n");
}
