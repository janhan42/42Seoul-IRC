#include "User.hpp"

User::User(int Userfd)
: mUserFd(Userfd)
, mUserName("User")
, mbRegistUser(false)
, mbRegistNick(false)
, mbRegistPass(false)
{}

User::~User()
{}

/* Getter & Setter */
/**
 * @brief
 * User 클래스의 mNickName을 반환하는 함수
 * @return std::string
 */
std::string User::GetNickName(void)
{
	return (mNickName);
}

/**
 * @brief
 * User 클래스의 mUserName을 반환하는 함수
 * @return std::string
 */
std::string	User::GetUserName(void)
{
	return (mUserName);
}

/**
 * @brief
 * User 클래스의 mHostName을 반환하는 함수
 * @return std::string
 */
std::string User::GetHostName(void)
{
	return (mHostName);
}

/**
 * @brief
 * User 클래스의 mServerName을 반환하는 함수
 * @return std::string
 */
std::string	User::GetServerName(void)
{
	return (mServerName);
}

/**
 * @brief
 * User 클래스의 mRealName을 반환하는 함수
 * @return std::string
 */
std::string	User::GetRealName(void)
{
	return (mRealName);
}

/**
 * @brief
 * PASS USER NICK 세가지를 확인후 서버에 등록됬는지를 반환하는 함수
 * @return true
 * @return false
 */
bool User::GetIsRegist(void)
{
	return (mbRegistPass && mbRegistUser && mbRegistNick);
}

/**
 * @brief
 * User 클래스의 mbRegistPass를 반환하는 함수
 * @return true
 * @return false
 */
bool User::GetRegistPass(void)
{
	return (mbRegistPass);
}

/**
 * @brief
 * User 클래스의 mbRegistNick을 반환하는 함수
 * @return true
 * @return false
 */
bool User::GetRegistNick(void)
{
	return (mbRegistNick);
}

/**
 * @brief
 * User 클래스의 mbRegistUser를 반환하는 함수
 * @return true
 * @return false
 */
bool User::GetRegistUser(void)
{
	return (mbRegistUser);
}

/**
 * @brief
 * User 클래스의 mUserFd를 반환하는 함수
 * @return int
 */
int User::GetUserFd(void)
{
	return (mUserFd);
}

/**
 * @brief
 * User 클래스의 mUserRecvBuf를 반환하는 함수
 * @return std::string
 */
std::string User::GetUserRecvBuf(void)
{
	return (mUserRecvBuf);
}

/**
 * @brief
 * User 클래스의 mChannels를 반환하는 함수
 * @return std::vector<std::string>&
 */
std::vector<std::string>& User::GetChannels(void)
{
	return (mChannels);
}

/* Setter */
/**
 * @brief
 * User클래스의 mUserName, mHostName, mServerName, mRealName을 한번에 매개변수 값으로 초기화 해주는 함수
 * @param username
 * @param hostname
 * @param servername
 * @param realname
 */
void User::SetUser(std::string username, std::string hostname, std::string servername, std::string realname)
{
	mUserName = username;
	mHostName = hostname;
	mServerName = servername;
	mRealName = realname;
}

/**
 * @brief
 * User 클래스의 mUserName을 매개변수 name으로 초기화 해주는 함수
 * @param name
 */
void User::SetUserName(const std::string& name)
{
	mUserName = name;
}

/**
 * @brief
 * User 클래스의 mNickName을 매개변수 name으로 초기화 해주는 함수
 * @param name
 */
void User::SetNickName(const std::string& name)
{
	mNickName = name;
}

/**
 * @brief
 * User 클래스의 mHostName을 매개변수 name으로 초기화 해주는 함수
 * @param name
 */
void User::SetHostName(const std::string& name)
{
	mHostName = name;
}

/**
 * @brief
 * User 클래스의 mServerName을 매개변수 name으로 초기화 해주는 함수
 * @param name
 */
void User::SetServerName(const std::string& name)
{
	mServerName = name;
}

/**
 * @brief
 * User 클래스의 mRealName을 매개변수 name으로 초기화 해주는 함수
 * @param name
 */
void User::SetRealNmae(const std::string& name)
{
	mRealName = name;
}

/**
 * @brief
 * User 클래스의 mbRegistPass를 매개변수 state으로 초기화 해주는 함수
 * @param stste
 */
void User::SetRegistPass(bool stste)
{
	mbRegistPass = stste;
}

/**
 * @brief
 * User 클래스의 mbRegistNick을 매개변수 state으로 초기화 해주는 함수
 * @param state
 */
void User::SetRegistNick(bool state)
{
	mbRegistNick = state;
}

/**
 * @brief
 * User 클래스의 mbRegistUser을 매개변수 state으로 초기화 해주는 함수
 * @param state
 */
void User::SetRegistUser(bool state)
{
	mbRegistUser = state;
}

/**
 * @brief
 * User 클래스의 mbRegistUser, mbRegistNick, mbRegistPass를
 * 매개변수 state값으로 초기화 해주는 함수
 * @param state
 */
void User::SetRegistAll(bool state)
{
	mbRegistUser = state;
	mbRegistNick = state;
	mbRegistPass = state;
}

/* Append */
/**
 * @brief
 * User 클래스의 mUserRecvBuf(내부 버퍼)에 매개변수 userRecvBuf를 추가해주는 함수
 * @param userRecvBuf
 */
void User::AppendUserRecvBuf(std::string userRecvBuf)
{
	mUserRecvBuf.append(userRecvBuf);
}

/**
 * @brief
 * User 클래스의 mChannels에 매개변수 channelName을 추가해주는 함수(pust_back)
 * @param channelName
 */
void User::AppendChannels(std::string channelName)
{
	mChannels.push_back(channelName);
}

/* Clear */
/**
 * @brief
 * User 클래스의 mUserRecvBuf를 클리어하는 함수
 */
void User::ClearUserRecvBuf(void)
{
	mUserRecvBuf.clear();
}

/**
 * @brief
 * User 클랫의 mChannels를 클리어하는 함수
 */
void User::ClearChannels(void)
{
	mChannels.clear();
}

/**
 * @brief
 * User 클래스의 mNickName, mUserName, mHostName, mServerName, mRealName, mUserFd(-2), mChannels, mUserRecvBuf, mbRegist~를 빈것으로 초기화 하는 함수
 */
void User::ClearUser(void)
{
	mNickName = "";
	mUserName = "";
	mHostName = "";
	mServerName = "";
	mRealName = "";
	mUserFd = -2;
	ClearChannels();
	ClearUserRecvBuf();
	SetRegistAll(false);
}

/* Other */
/**
 * @brief
 * User 클래스를 Bot 설정값으로 설정하는 함수
 */
void User::MakeUserToBot(void)
{
	mNickName = "Bot";
	mUserName = "Bot";
	mHostName = "Bot";
	mServerName = "Bot";
	mRealName = "Bot";
	SetRegistAll(true);
}

/**
 * @brief
 * User 클래스의 mChannels에서 매개변서 channelNmae을 찾아서 지우는 함수
 * @param channelName
 */
void User::RemoveChannel(std::string channelName)
{
	std::vector<std::string>::iterator it = FindChannel(channelName);
	if (it != mChannels.end())
		mChannels.erase(it);
}

/**
 * @brief
 * User 클래스의 mChannels에서 매개변수 channelName을 찾아서 iterator를 반환하는 함수
 * @param channelName
 * @return std::vector<std::string>::iterator
 */
std::vector<std::string>::iterator User::FindChannel(std::string channelName)
{
	std::vector<std::string>::iterator it = mChannels.begin();
	for (; it != mChannels.end(); ++it)
	{
		if (*it == channelName)
			return (it);
	}
	return (it);
}

