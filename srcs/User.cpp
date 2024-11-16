#include "User.hpp"

User::User(int fd)
{
	mUserFd = fd;
	mNickName = "User";
	mbIsRegistPass = false;
	mbIsRegistNick = false;
	mbIsRegistUser = false;
	mbIsCapEnd = false;
}

User::~User()
{
	mNickName = "";
	mUserName = "";
	mHostName = "";
	mServerName = "";
	mRealName = "";
	mUserFd = -2;
	ClearChannelList();
	ClearUserSendBuf();
	SetRegist(false);
	mbIsCapEnd = false;
}

/* Getter */

std::string	User::GetNickName()
{
	return (mNickName);
}

std::string	User::GetUserName()
{
	return (mUserName);
}

std::string	User::GetHostName()
{
	return (mHostName);
}

std::string	User::GetServerName()
{
	return (mServerName);
}

std::string	User::GetRealName()
{
	return (mRealName);
}

bool	User::GetIsRegist()
{
	return (mbIsRegistPass && mbIsRegistNick && mbIsRegistUser);
}

bool	User::GetPassRegist()
{
	return (mbIsRegistPass);
}

bool	User::GetNickRegist()
{
	return (mbIsRegistNick);
}

bool	User::GetUserRegist()
{
	return (mbIsRegistUser);
}

bool	User::GetCapEnd()
{
	return (mbIsCapEnd);
}

int		User::GetUserFd()
{
	return (mUserFd);
}

std::string	User::GetUserSendBuf()
{
	return (mUserSendBuf);
}

std::vector<std::string>&	User::GetChannelList()
{
	return (mChannelList);
}


/* Setter */
void	User::SetNickName(std::string name)
{
	mNickName = name;
}

void	User::SetUser(std::string userName, std::string hostName, std::string serverName, std::string realName)
{
	mUserName = userName;
	mHostName = hostName;
	mServerName = serverName;
	mRealName = realName;
}

void	User::SetPassRegist(bool state)
{
	mbIsRegistPass = state;
}

void	User::SetNickRegist(bool state)
{
	mbIsRegistNick = state;
}

void	User::SetUserRegist(bool state)
{
	mbIsRegistUser = state;
}

void	User::SetRegist(bool state)
{
	mbIsRegistPass = state;
	mbIsRegistNick = state;
	mbIsRegistUser = state;
}

void	User::SetCapEnd(bool state)
{
	mbIsCapEnd = state;
}


/* Append */
void	User::AppendUserSendBuf(std::string userRecvBuffer)
{
	mUserSendBuf.append(userRecvBuffer);
}

void	User::AppendChannelList(std::string channelName)
{
	mChannelList.push_back(channelName);
}


/* Clear */
void	User::ClearUserSendBuf()
{
	mUserSendBuf.clear();
}

void User::ClearUserSendBuf(int len)
{
	mUserSendBuf = mUserSendBuf.substr(len);
}

void	User::ClearChannelList()
{
	mChannelList.clear();
}

void	User::MakeUserToBot()
{
	mNickName = "Bot";
	mUserName = "Bot";
	mHostName = "Bot";
	mServerName = "Bot";
	mRealName = "Bot";
	SetRegist(true);
}

// 인자로 들어온 채널이름을 유저의 mChannelList에서 삭제
void User::RemoveChannel(std::string channelName)
{
	std::vector<std::string>::iterator It = FindChannel(channelName);
	if (It != mChannelList.end())
		mChannelList.erase(It);
}

// 유저가 접속중인 채널 중에 인자로 주어진 이름의 채널이 있는지 찾는 함수
// iterator 를 리턴하는데 있으면 찾은 자리, 없으면 end() 를 리턴
// 위에 RemoveChannel함수에서 사용할 수 있게 제작됨
std::vector<std::string>::iterator	User::FindChannel(std::string channelName)
{
	std::vector<std::string>::iterator It = mChannelList.begin();
	for (; It != mChannelList.end(); It++)
	{
		if (*It == channelName)
			return (It);
	}
	return (It);
}

// 유저의 channelName 채널 접속 여부 리턴
bool User::IsInChannel(const std::string channelName)
{
	if (this->FindChannel(channelName) == mChannelList.end())
		return (false);
	return (true);
}
