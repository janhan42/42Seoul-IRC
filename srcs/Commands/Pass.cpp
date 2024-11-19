
#include <unistd.h>
#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"

/*
	- RESPONSE LIST -
	ERR_NEEDMOREPARAMS (461)
	ERR_ALREADYREGISTERED (462)
	ERR_PASSWDMISMATCH (464)
 */

/**
 * @brief
 * /PASS 명령어를 처리하는 함수
 *
 * 1. 클라이언트가 /PASS 명령어를 보내면, 해당 패스워드가 서버의 패스워드와 일치하는지 확인합니다.
 * 2. 패스워드가 일치하면, 사용자에 대해 패스워드 등록을 완료하고, 이후 명령을 받을 수 있도록 합니다.
 * 3. 패스워드가 일치하지 않으면, 에러 메시지를 전송하고, 해당 사용자를 서버에서 제거합니다.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /PASS 명령어에 대한 파라미터 리스트
 */
void Command::Pass(int fd, std::vector<std::string> commandVec)
{
	// 사용자 객체를 서버에서 찾음
	class User* user = mServer.FindUser(fd);
	// 서버에서 설정된 패스워드 가져오기
	std::string password = mServer.GetPassWord();

	// 패스워드가 이미 등록된 경우 에러 메시지 전송
	if (user->GetPassRegist())
	{
		mResponse.ErrorAlreadyRegistRed462(*user);
		return ;
	}

	// 패스워드가 제공되지 않으면 에러 메시지 전송
	if (commandVec.size() < 2)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]);
		return ;
	}

	// 제공된 패스워드가 서버 패스워드와 일치하지 않으면 에러 처리
	if (commandVec[1] != password)
	{
		mResponse.ErrorPasswdMisMatch464(*user);
		// 사용자에게 오류 메시지 전송
		send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
		// 사용자 세션 종료
		mServer.DeleteUserFromServer(fd);
		return ;
	}

	// 패스워드가 일치하면 패스워드 등록을 완료하고 이후 명령을 받을 수 있도록 설정
	user->SetPassRegist(true);
}
