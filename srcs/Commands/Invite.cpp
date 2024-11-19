#include "../Command.hpp"
#include "../Server.hpp"
#include "../User.hpp"


/*
	- RESPONSE LIST -
	RPL_INVITING (341)
	ERR_NEEDMOREPARAMS (461)
	ERR_NOSUCHCHANNEL (403)
	ERR_NOTONCHANNEL (442)
	ERR_CHANOPRIVSNEEDED (482)
	ERR_USERONCHANNEL (443)
 */

/**
 * @brief
 * /INVITE 명령어를 처리하는 함수
 *
 * 1. 클라이언트가 /INVITE 명령어를 보내면, 초대할 사용자와 채널 정보를 확인합니다.
 * 2. 초대할 대상 사용자가 존재하는지, 초대할 채널이 존재하는지 확인합니다.
 * 3. 요청한 사용자가 채널에 참여하고 있는지, 오퍼레이터 권한이 있는지 체크합니다.
 * 4. 초대 대상 사용자가 이미 채널에 있을 경우 에러 메시지를 전송합니다.
 * 5. 초대가 성공하면, 초대된 사용자에게 채널 초대 메시지를 전송하고, 초대된 사용자를 채널의 초대 리스트에 추가합니다.
 *
 * @param fd 사용자의 파일 디스크립터
 * @param commandVec /INVITE 명령어에 대한 파라미터 리스트
 */
void Command::Invite(int fd, std::vector<std::string> commandVec)
{
	// 클라이언트가 보낸 /INVITE 명령어에서 사용자와 채널 정보를 추출
	class User* user = mServer.FindUser(fd);

	// /INVITE 명령어에 필요한 최소 인자가 부족한 경우, 에러 메시지 전송
	if (commandVec.size() < 3)
	{
		mResponse.ErrorNeedMoreParams461(*user, commandVec[1]); // 파라미터 부족 에러
		return;
	}

	// 초대할 대상 사용자가 존재하는지 확인
	class User* target = mServer.FindUser(commandVec[1]);
	if (target == NULL)
	{
		mResponse.ErrorNosuchNick401(*user, commandVec[1]); // 대상 사용자 없음 에러
		return;
	}

	// 초대할 채널이 존재하는지 확인
	Channel* channel = mServer.FindChannel(commandVec[2]);
	if (channel == NULL)
	{
		mResponse.ErrorNosuchChannel403(*user, commandVec[2]); // 채널 없음 에러
		return;
	}

	// 요청한 사용자가 채널에 참여하고 있는지 확인
	if (channel->CheckUserInChannel(user->GetUserFd()) == false)
	{
		mResponse.ErrorNotOnChannel442(*user, commandVec[2]); // 채널에 없음 에러
		return;
	}

	// 채널의 오퍼레이터 권한이 있는지 확인
	if (channel->CheckOperator(user->GetUserFd()) == false)
	{
		mResponse.ErrorChanOprivsNeeded482(*user, commandVec[2]); // 오퍼레이터 권한 부족 에러
		return;
	}

	// 초대 대상 사용자가 이미 채널에 있을 경우 에러 메시지 전송
	if (channel->CheckUserInChannel(target->GetUserFd()) == true)
	{
		mResponse.ErrorUserOnChannel443(*user, commandVec[1], commandVec[2]); // 이미 채널에 있는 사용자 에러
		return;
	}

	// 초대 성공 시, 초대 메시지를 전송
	user->AppendUserSendBuf("341 " + user->GetNickName() + " " + commandVec[1] + " " + commandVec[2] + "\r\n");

	// 초대된 사용자에게 채널 초대 메시지를 전송
	target->AppendUserSendBuf(MakeFullName(fd) + " INVITE " + target->GetNickName() + " " + commandVec[2] + "\r\n");

	// 초대된 사용자를 채널의 초대 리스트에 추가
	channel->AppendInviteFdList(target->GetUserFd());
}
