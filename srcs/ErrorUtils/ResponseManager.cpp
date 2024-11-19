#include "ResponseManager.hpp"
#include "../User.hpp"
#include "../Command.hpp"

/**
 * @brief
 * ResponseManager 생성자
 */
ResponseManager::ResponseManager()
{}

/**
 * @brief
 * ResponseManager 소멸자
 */
ResponseManager::~ResponseManager()
{}

/**
 * @brief
 * 사용자에게 환영 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param FullName 서버의 전체 이름
 */
void ResponseManager::RPLWelcome001(User& user, std::string FullName)
{
	user.AppendUserSendBuf(":SIRC 001 " + user.GetNickName() + " :Welcome to the Smoking Relay Chat " + FullName + " \r\n");
}

/**
 * @brief
 * 사용자에게 호스트 정보를 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::RPLYourHost002(User& user)
{
	user.AppendUserSendBuf(":SIRC 002 " + user.GetNickName() + " :Your host is SIRC, running version v1.0.0 \r\n");
}

/**
 * @brief
 * 서버 생성 일자를 사용자에게 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::RPLCreated003(User& user)
{
	user.AppendUserSendBuf(":SIRC 003 " + user.GetNickName() + " :This server was created 2024 \r\n");
}

/**
 * @brief
 * 사용자에게 서버 정보와 버전을 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::RPLMyInfo004(User& user)
{
	user.AppendUserSendBuf(":SIRC 004  " + user.GetNickName() + " :SIRC v1.0.0 \r\n");
}

/**
 * @brief
 * 서버에서 지원하는 기능을 사용자에게 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::RPLISupport005(User& user)
{
	user.AppendUserSendBuf(":SIRC 005 " + user.GetNickName() + " :MAXNICKLEN=9 MAXCHANNELLEN=200 :are supported by this server\r\n");
}

/**
 * @brief
 * 채널에 대한 주제(Topic)가 없음을 사용자에게 전송하는 함수
 * @param user 사용자 객체
 * @param channel 채널 이름
 */
void ResponseManager::ErrorNoTopic331(User& user, std::string channel)
{
	user.AppendUserSendBuf("331 " + user.GetNickName() + " " + channel + " :" + RPL_NOTOPIC);
}

/**
 * @brief
 * 채널의 사용자 목록을 전송하는 함수
 * @param user 사용자 객체
 * @param channel 채널 이름
 * @param message 사용자 목록 메시지
 */
void ResponseManager::RPL_NamReply353(User& user, std::string channel, std::string message)
{
	user.AppendUserSendBuf("353 " + user.GetNickName() + " = " + channel + " :" + message + "\r\n");
}

/**
 * @brief
 * 채널에 대한 사용자 목록 전송을 마쳤다는 메시지를 사용자에게 전송하는 함수
 * @param user 사용자 객체
 * @param channel 채널 이름
 */
void ResponseManager::RPL_EndOfNames366(User& user, std::string channel)
{
	user.AppendUserSendBuf("366 " + user.GetNickName() + " " + channel + " " + RPL_ENDOFNAMES);
}

/**
 * @brief
 * 존재하지 않는 닉네임에 대해 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param nickName 잘못된 닉네임
 */
void ResponseManager::ErrorNosuchNick401(User& user, std::string nickName)
{
	user.AppendUserSendBuf("401 " + user.GetNickName() + " " + nickName + " :" + ERR_NOSUCHNICK);
}

/**
 * @brief
 * 존재하지 않는 채널에 대해 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param channel 잘못된 채널
 */
void ResponseManager::ErrorNosuchChannel403(User& user, std::string channel)
{
	user.AppendUserSendBuf("403 " + user.GetNickName() + " " + channel + " :[" + channel + "] " + ERR_NOSUCHCHANNEL);
}

/**
 * @brief
 * 전송할 텍스트가 없을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::ErrorNoTextToSend412(User& user)
{
	user.AppendUserSendBuf("412 " + user.GetNickName() + " :" + ERR_NOTEXTTOSEND);
}

/**
 * @brief
 * MOTD 파일이 없다는 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::RPLNoMotd422(User& user)
{
	user.AppendUserSendBuf(":SIRC 422  " + user.GetNickName() + " :MOTD File is missing\r\n");
}

/**
 * @brief
 * 닉네임이 주어지지 않았을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::ErrorNoNickNameGiven431(User& user)
{
	user.AppendUserSendBuf("431 " + user.GetNickName() + " :" + ERR_NONICKNAMEGIVEN);
}

/**
 * @brief
 * 사용자가 채널에 없을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param nickName 잘못된 닉네임
 * @param channel 채널 이름
 */
void ResponseManager::ErrorUserNotInChannel441(User& user, std::string nickName, std::string channel)
{
	user.AppendUserSendBuf("441 " + user.GetNickName() + " " + nickName + " " + channel + " :" + ERR_USERNOTINCHANNEL);
}

/**
 * @brief
 * 사용자가 해당 채널에 없을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param channel 채널 이름
 */
void ResponseManager::ErrorNotOnChannel442(User& user, std::string channel)
{
	user.AppendUserSendBuf("442 " + user.GetNickName() + " " + channel + " :" + ERR_NOTONCHANNEL);
}

/**
 * @brief
 * 사용자가 채널에 이미 있을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param nickName 이미 채널에 있는 닉네임
 * @param channel 채널 이름
 */
void ResponseManager::ErrorUserOnChannel443(User& user, std::string nickName, std::string channel)
{
	user.AppendUserSendBuf("443 " + user.GetNickName() + " " + nickName + " " + channel + " :" + ERR_USERONCHANNEL);
}

/**
 * @brief
 * 명령어에 필요한 매개변수가 부족할 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param command 잘못된 명령어
 */
void ResponseManager::ErrorNeedMoreParams461(User& user, std::string command)
{
	user.AppendUserSendBuf("461 " + user.GetNickName() + " " + command + " :" + ERR_NEEDMOREPARAMS);
}

/**
 * @brief
 * 이미 등록된 사용자가 다시 등록하려 할 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::ErrorAlreadyRegistRed462(User& user)
{
	user.AppendUserSendBuf("462 " + user.GetNickName() + " :" + ERR_ALREADYREGISTERED);
}

/**
 * @brief
 * 사용자가 등록되지 않았을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::ErrorNotRegistered451(User& user)
{
	user.AppendUserSendBuf("451: " + user.GetNickName() + " " + ERR_NOTREGISTERED);
}

/**
 * @brief
 * 비밀번호가 잘못되었을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::ErrorPasswdMisMatch464(User& user)
{
	user.AppendUserSendBuf("464 " + user.GetNickName() + " :" + ERR_PASSWDMISMATCH);
}

/**
 * @brief
 * 채널이 가득 차서 가입할 수 없을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param channel 채널 이름
 */
void ResponseManager::ErrorChannelIsFull471(User& user, std::string channel)
{
	user.AppendUserSendBuf("471 " + user.GetNickName() + " " + channel + " :" + ERR_CHANNELISFULL);
}

/**
 * @brief
 * 잘못된 모드 문자를 사용했을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param mode 잘못된 모드 문자
 */
void ResponseManager::ErrorUnknownMode472(User& user, char mode)
{
	user.AppendUserSendBuf("472 " + user.GetNickName() + " " + mode + " :" + ERR_UNKNOWNMODE);
}

/**
 * @brief
 * 초대 전용 채널에 가입하려 할 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param channel 채널 이름
 */
void ResponseManager::ErrorInviteOnlychan473(User& user, std::string channel)
{
	user.AppendUserSendBuf("473 " + user.GetNickName() + " " + channel + " :" + ERR_INVITEONLYCHAN);
}

/**
 * @brief
 * 비밀번호가 필요한 채널에 잘못된 비밀번호를 입력했을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param channel 채널 이름
 */
void ResponseManager::ErrorBadChannelKey475(User& user, std::string channel)
{
	user.AppendUserSendBuf("475 " + user.GetNickName() + " " + channel + " :" + ERR_BADCHANNELKEY);
}

/**
 * @brief
 * 채널의 운영자가 아닐 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param channel 채널 이름
 */
void ResponseManager::ErrorChanOprivsNeeded482(User& user, std::string channel)
{
	user.AppendUserSendBuf("482 " + user.GetNickName() + " " + channel + " :" + ERR_CHANOPRIVSNEEDED);
}

/**
 * @brief
 * 잘못된 닉네임을 사용했을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param name 잘못된 닉네임
 */
void ResponseManager::ErrorErronusNickName432(User& user, std::string name)
{
	user.AppendUserSendBuf("432: " + name + " " + name + " " + ERR_ERRONEUSNICKNAME);
}

/**
 * @brief
 * 이미 사용 중인 닉네임을 사용했을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 * @param name 이미 사용 중인 닉네임
 */
void ResponseManager::ErrorNickNameInuse433(User& user, std::string name)
{
	user.AppendUserSendBuf("433: " + name + " " + name + " " + ERR_NICKNAMEINUSE);
}

/**
 * @brief
 * 사용자가 다른 사용자와 일치하지 않을 때 에러 메시지를 전송하는 함수
 * @param user 사용자 객체
 */
void ResponseManager::ErrorUsersDontMatch502(User& user)
{
	user.AppendUserSendBuf("502 " + user.GetNickName() + " :" + ERR_USERSDONTMATCH);
}
