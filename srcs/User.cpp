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

std::string	User::GetUserRecvBuf()
{
	return (mUserRecvBuf);
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
void	User::AppendUserRecvBuf(std::string userRecvBuffer)
{
	mUserRecvBuf.append(userRecvBuffer);
}

void	User::AppendChannelList(std::string channelName)
{
	mChannelList.push_back(channelName);
}


/* Clear */
void	User::ClearUserRecvBuf()
{
	mUserRecvBuf.clear();
}

void	User::ClearChannelList()
{
	mChannelList.clear();
}

void	User::ClearUser()
{
	mNickName = "";
	mUserName = "";
	mHostName = "";
	mServerName = "";
	mRealName = "";
	mUserFd = -2;
	ClearChannelList();
	ClearUserRecvBuf();
	SetRegist(false);
	mbIsCapEnd = false;
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

void User::RemoveChannel(std::string channelName)
{
	std::vector<std::string>::iterator It = FindChannel(channelName);
	if (It != mChannelList.end())
		mChannelList.erase(It);
}

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
