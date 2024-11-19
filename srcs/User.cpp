#include "User.hpp"

User::User(int fd)
{
	mUserFd = fd;
	mNickName = "User";
	mbIsRegistPass = false;
	mbIsRegistNick = false;
	mbIsRegistUser = false;
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
}

/* Getter */
/**
 * @brief
 * User의 NickName을 리턴하는 함수
 * @return std::string
 */
std::string	User::GetNickName()
{
	return (mNickName);
}

/**
 * @brief
 * User의 UserName을 리턴하는 함수
 * @return std::string
 */
std::string	User::GetUserName()
{
	return (mUserName);
}

/**
 * @brief
 * User의 HostName을 리턴하는 함수
 * @return std::string
 */
std::string	User::GetHostName()
{
	return (mHostName);
}

/**
 * @brief
 * User의 ServerName을 리턴하는 함수
 * @return std::string
 */
std::string	User::GetServerName()
{
	return (mServerName);
}

/**
 * @brief
 * User의 RealName을 리턴하는 함수
 * @return std::string
 */
std::string	User::GetRealName()
{
	return (mRealName);
}

/**
 * @brief
 * User가 모든 등록 과정을 완료했는지 여부를 리턴하는 함수
 * @return bool
 */
bool	User::GetIsRegist()
{
	return (mbIsRegistPass && mbIsRegistNick && mbIsRegistUser);
}

/**
 * @brief
 * User의 비밀번호 등록 상태를 리턴하는 함수
 * @return bool
 */
bool	User::GetPassRegist()
{
	return (mbIsRegistPass);
}

/**
 * @brief
 * User의 NickName 등록 상태를 리턴하는 함수
 * @return bool
 */
bool	User::GetNickRegist()
{
	return (mbIsRegistNick);
}

/**
 * @brief
 * User의 UserName 등록 상태를 리턴하는 함수
 * @return bool
 */
bool	User::GetUserRegist()
{
	return (mbIsRegistUser);
}

/**
 * @brief
 * User의 파일 디스크립터(File Descriptor)를 리턴하는 함수
 * @return int
 */
int		User::GetUserFd()
{
	return (mUserFd);
}

/**
 * @brief
 * User의 송신 버퍼 내용을 리턴하는 함수
 * @return std::string
 */
std::string	User::GetUserSendBuf()
{
	return (mUserSendBuf);
}

/**
 * @brief
 * User가 참여 중인 채널 목록을 리턴하는 함수
 * @return std::vector<std::string>&
 */
std::vector<std::string>&	User::GetChannelList()
{
	return (mChannelList);
}


/* Setter */
/**
 * @brief
 * User의 NickName을 설정하는 함수
 * @param name 설정할 NickName
 */
void	User::SetNickName(std::string name)
{
	mNickName = name;
}

/**
 * @brief
 * User의 정보를 설정하는 함수
 * @param userName 설정할 UserName
 * @param hostName 설정할 HostName
 * @param serverName 설정할 ServerName
 * @param realName 설정할 RealName
 */
void	User::SetUser(std::string userName, std::string hostName, std::string serverName, std::string realName)
{
	mUserName = userName;
	mHostName = hostName;
	mServerName = serverName;
	mRealName = realName;
}

/**
 * @brief
 * User의 비밀번호 등록 상태를 설정하는 함수
 * @param state 설정할 등록 상태 (true/false)
 */
void	User::SetPassRegist(bool state)
{
	mbIsRegistPass = state;
}

/**
 * @brief
 * User의 NickName 등록 상태를 설정하는 함수
 * @param state 설정할 등록 상태 (true/false)
 */
void	User::SetNickRegist(bool state)
{
	mbIsRegistNick = state;
}

/**
 * @brief
 * User의 UserName 등록 상태를 설정하는 함수
 * @param state 설정할 등록 상태 (true/false)
 */
void	User::SetUserRegist(bool state)
{
	mbIsRegistUser = state;
}

/**
 * @brief
 * User의 모든 등록 상태를 설정하는 함수
 * @param state 설정할 등록 상태 (true/false)
 */
void	User::SetRegist(bool state)
{
	mbIsRegistPass = state;
	mbIsRegistNick = state;
	mbIsRegistUser = state;
}

/* Append */
/**
 * @brief
 * User의 송신 버퍼에 데이터를 추가하는 함수
 * @param userRecvBuffer 추가할 데이터
 */
void	User::AppendUserSendBuf(std::string userRecvBuffer)
{
	mUserSendBuf.append(userRecvBuffer);
}

/**
 * @brief
 * User의 채널 목록에 새로운 채널을 추가하는 함수
 * @param channelName 추가할 채널 이름
 */
void	User::AppendChannelList(std::string channelName)
{
	mChannelList.push_back(channelName);
}

/* Clear */
/**
 * @brief
 * User의 송신 버퍼를 비우는 함수
 */
void	User::ClearUserSendBuf()
{
	mUserSendBuf.clear();
}

/**
 * @brief
 * User의 송신 버퍼를 특정 길이만큼 잘라내는 함수
 * @param len 잘라낼 길이
 */
void	User::ClearUserSendBuf(int len)
{
	mUserSendBuf = mUserSendBuf.substr(len);
}

/**
 * @brief
 * User의 채널 목록을 비우는 함수
 */
void	User::ClearChannelList()
{
	mChannelList.clear();
}

/* Utility */
/**
 * @brief
 * User의 정보를 Bot으로 설정하는 함수
 */
void	User::MakeUserToBot()
{
	mNickName = "Bot";
	mUserName = "Bot";
	mHostName = "Bot";
	mServerName = "Bot";
	mRealName = "Bot";
	SetRegist(true);
}

/**
 * @brief
 * User의 채널 목록에서 특정 채널을 제거하는 함수
 * @param channelName 제거할 채널 이름
 */
void User::RemoveChannel(std::string channelName)
{
	std::vector<std::string>::iterator It = FindChannel(channelName);
	if (It != mChannelList.end())
		mChannelList.erase(It);
}

/**
 * @brief
 * User의 채널 목록에서 특정 채널을 찾는 함수
 * @param channelName 찾을 채널 이름
 * @return std::vector<std::string>::iterator 채널이 존재하면 해당 위치, 없으면 끝 위치를 반환
 */
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

/**
 * @brief
 * User가 특정 채널에 속해 있는지 확인하는 함수
 * @param channelName 확인할 채널 이름
 * @return bool 채널에 속해 있으면 true, 아니면 false
 */
bool User::IsInChannel(const std::string channelName)
{
	if (this->FindChannel(channelName) == mChannelList.end())
		return (false);
	return (true);
}

