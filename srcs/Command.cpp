#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include "Channel.hpp"
#include "Define.hpp"
#include "Server.hpp"
#include "Command.hpp"
#include "User.hpp"
#include <iostream>

Command::Command(Server& Server)
: mServer(Server)
{
}

Command::~Command()
{
}

// Server 클래스에서 메세지가 잘 끝났는지 확인 후 여기로 보냄
// 메세지를 잘라서 해당 함수로 전달함
// 아직 등록이 안된 유저인 경우는 정해진 순서에 따라 등록할 수 있도록
// RegistNewUser 함수로 보냄
void Command::Run(int fd)
{
	std::istringstream iss(mServer.GetMessage(fd));
	class User* user = mServer.FindUser(fd);
	std::vector<std::string> commandVec;
	std::string buffer;

	while (getline(iss, buffer, ' '))
	{
		std::size_t endPos = buffer.find_last_not_of("\r\n");
		commandVec.push_back(buffer.substr(0, endPos + 1));
	}
	if (user!= NULL && !user->GetIsRegist()) // new User Join Server
		RegistNewUser(fd, user, commandVec);
	else
	{
		if (commandVec[0] == "PING")
			Ping(fd, commandVec);
		else if (commandVec[0] == "USER")
			User(fd, commandVec);
		else if (commandVec[0] == "NICK")
			Nick(fd, commandVec);
		else if (commandVec[0] == "PASS")
			Pass(fd, commandVec);
		else if (commandVec[0] == "PRIVMSG")
			Privmsg(fd, commandVec);
		else if (commandVec[0] == "QUIT")
			Quit(fd, commandVec);
		else if (commandVec[0] == "PART")
			Part(fd, commandVec);
		else if (commandVec[0] == "JOIN")
			Join(fd, commandVec);
		else if (commandVec[0] == "KICK")
			Kick(fd, commandVec);
		else if (commandVec[0] == "MODE")
			Mode(fd, commandVec);
		else if (commandVec[0] == "TOPIC")
			Topic(fd, commandVec);
		else if (commandVec[0] == "INVITE")
			Invite(fd, commandVec);
	}
}

/**
 * @brief
 * 새로운 유저의 PASS NICK USER 프로토콜 응답 및 가입 상태를 관리하는 함수
 * @param fd
 * @param user
 * @param commandVec
 */
void Command::RegistNewUser(int fd, class User* user, std::vector<std::string>& commandVec)
{
	if (commandVec[0] == "PASS")
		Pass(fd, commandVec);
	else if (commandVec[0] == "NICK")
		Nick(fd, commandVec);
	else if (commandVec[0] == "USER")
	{
		User(fd, commandVec);
		if (user->GetIsRegist() == false)
		{
			mResponse.ErrorNotRegistered451(*user);
			send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
			mServer.DeleteUserFromServer(fd);
			return;
		}
	}
	if (user->GetIsRegist())
	{
		mResponse.RPLWelcome001(*user, MakeFullName(fd));
		mResponse.RPLYourHost002(*user);
		mResponse.RPLCreated003(*user);
		mResponse.RPLMyInfo004(*user);
		mResponse.RPLISupport005(*user);
		mResponse.RPLNoMotd422(*user);
	}
}

/**
 * @brief
 * 이름이 channelName인 채널에 있는 유저 전부에게 매개변수 msg를 전송
 * @param target
 * @param channelName
 * @param command
 * @param msg
 */
void Command::MsgToAllChannel(int fd, std::string channelName, std::string command, std::string msg)
{
	Channel* channel = mServer.FindChannel(channelName);
	if (channel == NULL) // does not exist
		return;
	std::vector<int> channelUserList = channel->GetUserFdList();
	std::vector<int>::iterator channelUserIt = channelUserList.begin();
	while (channelUserIt != channelUserList.end())
	{
		class User* targetUser = mServer.FindUser(*channelUserIt); // 채널에 있는 모든 유저를 순회하며 조회
		if (command == "PRIVMSG" && fd == *channelUserIt) // 현재 channelUserIt이 자기 자신이면 스킵
		{
			channelUserIt++;
			continue;
		}
		targetUser->AppendUserSendBuf(MakeFullName(fd) + " " + command + " " + channelName + " " + msg + "\r\n");
		channelUserIt++;
	}
}

/**
 * @brief
 * 채널에 있는 유저가 닉네임을 변경했을 경우 채널 내부에 있는 모든 유저에게 알림
 * @param fd
 * @param channelName
 * @param oldNickName
 * @param NewNick
 */
void Command::NickMsgToAllChannel(int fd, std::string channelName, std::string oldNickName, std::string NewNick)
{
	Channel* channel = mServer.FindChannel(channelName);
	if (channel == NULL) // does not exist
		return;
	std::vector<int> channelUserList = channel->GetUserFdList();
	std::vector<int>::iterator channelUserIt = channelUserList.begin();
	while (channelUserIt != channelUserList.end())
	{
		class User* targetUser = mServer.FindUser(*channelUserIt);
		if (fd == *channelUserIt) //  현재 channelUserIt이 자기 자신이면 스킵
		{
			channelUserIt++;
			continue;
		}
		targetUser->AppendUserSendBuf(":" + oldNickName + " NICK " + NewNick + "\r\n");
		channelUserIt++;
	}
}

/**
 * @brief
 * 현재 유저에 대한 별명변경을 알리기 위한 전체 문장을 생성
 * @param fd
 * @return std::string
 */
std::string Command::MakeFullName(int fd)
{
	class User* user = mServer.FindUser(fd);
	std::string temp = (":" + user->GetNickName() + "!" + user->GetUserName() + "@" + user->GetServerName());
	return (temp);
}

/**
 * @brief
 * /names 및 채널 접속시 현재 유저들에 대한 이름 전송
 * @param fd
 * @param channelName
 */
void Command::NameListMsg(int fd, std::string channelName)
{
	Channel* channel = mServer.FindChannel(channelName);

	if (channel == NULL) // does not exist
		return;

	std::vector<int> userFdList = channel->GetUserFdList();
	std::vector<int>::iterator iter = userFdList.begin();
	std::string message;
	while (iter != userFdList.end())
	{
		class User* user = mServer.FindUser(*iter);
		if (channel->CheckOperator(user->GetUserFd()))
		{
			message += "@";
		}
		message += user->GetNickName();
		if (iter != userFdList.end() - 1)
			message += " ";
		iter++;
	}
	class User* user = mServer.FindUser(fd);
	/* e
		O ("=", 0x3D)- 공개 채널.
		X ("@", 0x40)- 비밀 채널( 비밀 채널 모드 "+s" ).
		X ("*", 0x2A)- 개인 채널( "+p"이전에는 널리 사용되지 않았음).
		공개 채널만 지원
	 */
	mResponse.RPL_NamReply353(*user, channelName, message);
	mResponse.RPL_EndOfNames366(*user, channelName);
}

/**
 * @brief
 * 분리 해뒀던 commandVec을 다시 하나의 메시지로 만드는 함수
 * @param index
 * @param commandVec
 * @return std::string
 */
std::string Command::ChannelMessage(int index, std::vector<std::string> commandVec)
{
	std::string message = "";
	for (size_t i = index; i < commandVec.size(); i++)
	{
		message += commandVec[i];
		if (i != commandVec.size() - 1)
			message += " ";
	}
	if (message[0] == ':')
		message = message.substr(1, message.length() - 1);
	if (message.length() == 0) // PART No message
		message = "NO REASON";
	return (message);
}

/**
 * @brief
 * 채널 내에 있는 모든 유저에게 동일한 메시지를 전송하는 함수
 * @param message
 * @param user
 * @param chennal
 */
void Command::ChannelPrivmsg(std::string message, class User& user, Channel* chennal)
{
	std::vector<int> fdList = chennal->GetUserFdList();
	std::vector<int>::iterator fdIter = fdList.begin();
	for(; fdIter != fdList.end(); fdIter++)
	{
		if (*fdIter != user.GetUserFd())
		{
			//class User*& target = mServer.GetUserList().find(*fdIter)->second;
			class User* target = mServer.FindUser(*fdIter);
			target->AppendUserSendBuf(":" + user.GetNickName() + " PRIVMSG " + chennal->GetChannelName() + " :" + message + "\r\n");
		}
	}
}
