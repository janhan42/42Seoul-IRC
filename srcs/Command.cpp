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
	// std::map<int, class User *>& userList = mServer.GetUserList();
	// std::map<int, class User *>::iterator userIt = userList.find(fd);
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

// 새로 연결된 유저는 프로토콜에 맞춰 pass nick user 명령령어를 주고 받아야함
//
// user 명령어가 끝난 시점에 GetIsRegist 함수가 거짓을 반환하는 경우
// 서버에 등록이 되면 안되기 때문에 451을 보내고 유저를 서버에서 삭제함
//
// 정상적으로 등록이 되면 프로토콜에 맞춰서 001~005, 442 응답을 전송 후 종료
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
			// user->AppendUserSendBuf(commandVec[1] + ": " + ERR_NOTREGISTERED);
			mResponse.ErrorNotRegistered451(*user);
			send(fd, user->GetUserSendBuf().c_str(), user->GetUserSendBuf().length(), 0);
			// delete user->second;
			// userList.erase(fd);
			//close(fd);
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
	// std::map<std::string, Channel *>& channelList = mServer.GetChannelList();
	// if (channelList.find(channelName) == channelList.end()) // not exist channel
	// 	return;
	// Channel* channel = channelList.find(channelName)->second;
	Channel* channel = mServer.FindChannel(channelName);
	if (channel == NULL) // does not exist
		return;
	std::vector<int> channelUserList = channel->GetUserFdList();
	std::vector<int>::iterator channelUserIt = channelUserList.begin();
	while (channelUserIt != channelUserList.end())
	{
		//class User*& targetUser = mServer.GetUserList().find(*channelUserIt)->second; // 채널에 있는 모든 유저를 순회하며 조회
		class User* targetUser = mServer.FindUser(*channelUserIt); // 채널에 있는 모든 유저를 순회하며 조회
		if (command == "PRIVMSG" && fd == *channelUserIt) // 현재 channelUserIt이 자기 자신이면
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
	// std::map<std::string, Channel *>& channelList = mServer.GetChannelList();
	// if (channelList.find(channelName) == channelList.end())
	// 	return;
	// Channel* channel = channelList.find(channelName)->second;
	Channel* channel = mServer.FindChannel(channelName);
	if (channel == NULL) // does not exist
		return;
	std::vector<int> channelUserList = channel->GetUserFdList();
	std::vector<int>::iterator channelUserIt = channelUserList.begin();
	while (channelUserIt != channelUserList.end())
	{
		//class User*& targetUser = mServer.GetUserList().find(*channelUserIt)->second;
		class User* targetUser = mServer.FindUser(*channelUserIt);
		if (fd == *channelUserIt) //  현재 channelUserIt이 자기 자신이면
		{
			channelUserIt++;
			continue;
		}
		targetUser->AppendUserSendBuf(":" + oldNickName + " NICK " + NewNick + "\r\n");
		channelUserIt++;
	}
}

// 유저 닉네임과 풀네임, 서버이름을 정해진 규칙에 맞춰서 만든 후 리턴
std::string Command::MakeFullName(int fd)
{
	// std::map<int, class User* >& userList = mServer.GetUserList();
	// std::map<int, class User* >::iterator userIt = userList.find(fd);
	// class User*& user = userIt->second;
	class User* user = mServer.FindUser(fd);
	std::string temp = (":" + user->GetNickName() + "!" + user->GetUserName() + "@" + user->GetServerName());
	return (temp);
}

// 채널 내부에 있는 유저들의 이름을 공백으로 구분된 문자열로 만들어서
// fd 유저의 버퍼에 추가해주는 함수
// 체널 오퍼레이터는 앞에 @가 붙음
void Command::NameListMsg(int fd, std::string channelName)
{
	// std::map<std::string, Channel* >& channelList = mServer.GetChannelList();
	// if (channelList.find(channelName) == channelList.end())
	// 	return;
	// Channel* channel = channelList.find(channelName)->second;
	Channel* channel = mServer.FindChannel(channelName);
	if (channel == NULL) // does not exist
		return;
	std::vector<int> userFdList = channel->GetUserFdList();
	std::vector<int>::iterator iter = userFdList.begin();
	std::string message;
	while (iter != userFdList.end())
	{
		//class User*& user = mServer.GetUserList().find(*iter)->second;
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
	//class User*& user = mServer.GetUserList().find(fd)->second;
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

// 스페이스바로 전부 쪼개놨던 string vector를 다시 붙여주는 함수
// PRIVMSG(or /msg) <channel/nickname> <messages...>
// 이런 식으로 들어왔을 때 messages 도 전부 스페이스바로 쪼개져서 벡터에 들어있는데
// 이건 이제 합쳐서 보내야 하기 때문에 해당 함수로 합쳐버림
// 콜론은 이 함수를 호출하는 부분에 보통 들어가있기 때문에 제거해주고
// message length가 0인 경우는 part 명령어 <채널이름> 뒤에 아무것도 안오면
// NO REASON을 추가해주기 위함
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
	if (message.length() == 0)
		message = "NO REASON";
	return (message);
}

// PRIVMSG 명령어가 들어오면 채널에 있는 다른유저들 전부에게 해당 메세지 내용을 전달해주는 함수
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

// 안쓰는 함수인듯 주석처리해도 컴파일잘됨
// void Command::ChannelPART(int fd, std::string channelName, std::vector<std::string> commandVec)
// {
// 	// std::map<int, class User* >& userList = mServer.GetUserList();
// 	// std::map<int, class User* >::iterator userIt = userList.find(fd);
// 	class User* user = mServer.FindUser(fd);
// 	Channel* channel = mServer.FindChannel(channelName);
// 	std::vector<int> fdList = channel->GetUserFdList();
// 	std::vector<int>::iterator fdIter = fdList.begin();
// 	std::string message = ChannelMessage(1, commandVec);
// 	for(; fdIter != fdList.end(); fdIter++)
// 	{
// 		if (*fdIter != fd)
// 		{
// 			//class User*& target = mServer.GetUserList().find(*fdIter)->second;
// 			class User* target = mServer.FindUser(*fdIter);
// 			target->AppendUserSendBuf(":" + target->GetNickName() + "!" + target->GetUserName() + "@" + target->GetServerName() + user->GetNickName() + " PART " + channel->GetChannelName() + " " + message + "\r\n");
// 		}
// 	}
// }

